package cvac.corpus;

import com.ice.tar.TarEntry;
import com.ice.tar.TarInputStream;
import cvac.CorpusCallback;
import cvac.Labelable;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.util.Iterator;
import java.util.Properties;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.zip.GZIPInputStream;
import util.Data_IO_Utils;
import util.Data_IO_Utils.FS_ObjectType;
import util.DownloadUtils;

/**
 * All common data sets use functionality in this class.  Samples and
 * categories can be added, removed etc.
 * LabelMe, Vatic etc are in a separate class.
 *
 * @author matz
 */
public class CommonDataSet extends CorpusI {
    private Properties properties;

    public enum DataSetLocType {LOCAL, URL, STREAMING, REMOTE, LABELME}
    public enum CompressionType {GZIP, BZ2, Z, UNCOMPRESSED}
    public enum ArchiveType {A, AR, CPIO, SHAR, LBR, ISO, TAR, UNARCHIVED}
    
    private File m_metaDataFolder, // '.meta' sub-folder of local corpus folder
                 m_metaStatusFile;   // File: 'status.txt'
    
    //private CommonDataSet m_parent; //?

    public CommonDataSet(String name, String description, String homepageURL, boolean isImmutableMirror)
    {
        super(name, description, homepageURL, isImmutableMirror);
                
        // note that at first these dirs/files don't exist yet
        m_metaDataFolder = new File( m_dataSetFolder + File.separator + ".meta" );
        m_metaStatusFile = new File( m_metaDataFolder + File.separator + "status.txt" );
    }
    
//    /*
//     * Create samples for each file in the dataset on the remote mirror.
//     */
//    public void loadSamplesFromRemoteMirror( DataSetMirror mirror )
//    {
//        String fileName = getMirrorListFile( mirror );
//        File listFile = new File(fileName);
//
//         // Read back resulting detected rectangles
//        BufferedReader rlist = null;
//        try {
//            rlist = new BufferedReader(new FileReader(listFile));
//        }
//        catch(FileNotFoundException fnf) {
//            String msg = "Could not open mirror list file \n" + 
//                         fnf.toString();
//            logger.warning(msg);
//            return;
//        }     
//        String folder = mirror.getInitLocation();
//        int len = folder.length();
//        if ('/' == folder.charAt(len - 1)){
//            folder = folder.substring(0, len - 1);  
//        }
//        loadSampleFromRemoteMirrorFile(name, folder, rlist, null);
//        
//       
//    }
    
    
    @Override
    void createLocalMirror( CorpusCallback cb )
    {
        // does a local mirror exist already?
        if ( localMirrorExists() )
        {
            logger.log( Level.INFO, "Local mirror for Corpus {0} exists already.", this.name );
            return;
        }
        
        // download and uncompress the Corpus archive
        String main_location = properties.getProperty("main_location");
        File ml = new File( main_location );
        String fname = ml.getName();
        File localArchiveFile = new File( m_metaDataFolder + File.separator + fname );
        String decompressed = "decompressed.tar";
        expandDataset_toLocal( main_location, localArchiveFile, decompressed, 
                m_dataSetFolder, CommonDataSet.CompressionType.GZIP );

        // if no errors, write a little metadata file
        writeStatusFile();
        
        // read in the Label data
        loadImages();
    }


    private boolean localMirrorExists() {
        // where shall this mirror live?
        // for now, it's a fixed location based on the corpus name
        File tentfile = new File( m_dataSetFolder );
        if (tentfile.exists())
        {
            // check if the metadata is present and complete, if so, this mirror exists already
            if (m_metaStatusFile.exists() )
            {
                return true;
            }
            
            // if not: something else is in this folder, and we can't handle that right now
            throw new RuntimeException("data dir exists already but no metadata; aborting because no plan B available");
        }
        return false;
    }
   
    
      // Orchestrate entire unpacking sequence
    protected void expandDataset_toLocal( String webURL, File web_SavedFile, String decompressedOutputName,
                                          String expandBelow_Path, CompressionType uncompressType) {
        // Grab file to local disk
        try {
            DownloadUtils.URL_readToFile(webURL, web_SavedFile);
        }
        catch(IOException e) {
            String msg = "Exception in URL_readToFile: " + e.toString();
            logger.warning(msg);
        }

        File tarOutput;
        if(CompressionType.GZIP == uncompressType) {  // Unzipping GZip Produces (.tar) in /expansion/ folder
            tarOutput = new File( m_metaDataFolder.getAbsolutePath() +
                                  File.separatorChar + decompressedOutputName ); 

            try {
                uncompressDataset(web_SavedFile, tarOutput, expandBelow_Path, uncompressType);

                String msg = "Uncompressed Dataset with 'GZIP', saved: " + tarOutput.getName();
                logger.log(Level.INFO, msg);
            }
            catch(IOException e)
            {
                throw new RuntimeException("Error uncompressing DataSet with 'GZIP' protocol.  \n" + e.toString());
            }
        }
        else {
            tarOutput = new File(web_SavedFile.getPath());  // Web URL provided uncompressed (.tar)
        }
        File targetDatasetFolder = new File(expandBelow_Path);

        // TODO: why was there a new dataset created here?
        //DataSet dataSetInstance = new DataSet();
        //dataSetInstance.unpackArchiveBelow(tarOutput, targetDatasetFolder, ArchiveType.TAR);
        unpackArchiveBelow(tarOutput, targetDatasetFolder, ArchiveType.TAR);
    }
    
    // Read source-File, uncompress and save into destination-File in the 'source' directory
    protected void uncompressDataset(File compressedFileSrc, File destFile, String destFileDir, CompressionType in_type) throws IOException {

          // Create new file if target !exists, or Uncompress over an existing target
        if ((null == destFile) || (!destFile.exists())) {
            Data_IO_Utils.createDir_orFile(destFile, FS_ObjectType.FILE);
        }
        
          // Todo, operate on generic src and dest Stream Objects after switch code
        switch(in_type) {
            
            case GZIP:  // GNU Gunzip
                final int GUNZIP_BUFFER = 2048;
                byte buff[];
                int amtRead;
                FileOutputStream outToFile = null;
                GZIPInputStream GZIP_InStrm = null;

                try {
                    GZIP_InStrm = new GZIPInputStream(new FileInputStream(compressedFileSrc));
                    outToFile = new FileOutputStream(destFile);
                    buff = new byte[GUNZIP_BUFFER];

                    while ((amtRead = GZIP_InStrm.read(buff)) > 0) {
                        outToFile.write(buff, 0, amtRead);  // Transfer blocks of data 'src' to 'dest'
                    }
                    GZIP_InStrm.close();
                    outToFile.close();
                }
                catch(FileNotFoundException e) {
                    String msg = ("Error using specified files for extracting with Gzip.\n" + 
                                  "Compressed Source: " + compressedFileSrc.getPath() + "\n" +
                                  "Destination File: "  + destFile.getPath()    + "\n" + e.toString());
                    logger.warning(msg);
                    removeMetaFlg();
                }
                    catch(Exception e) {
                    String msg = ("Error while extracting from Gzip-Input-Stream." + e.toString());
                    logger.warning(msg);
                }
                break;

            default:
                logger.warning("De-compression type not yet supported.  Usable types include: 'GZIP' (Gnu-Gunzip).  ");
                removeMetaFlg();
        }
    }

    // Creates a new folder using the Dataset's name String to create folder containing all dataset content
    protected void unpackArchiveBelow(File archiveFile, File dsTargetFolder, ArchiveType archiveType) {
        
        Data_IO_Utils.dieRuntime_IfNull(dsTargetFolder, 
                                        "'contentFolder' File-object is not allowed to be null in 'unpackArchiveBelow(..)'");
        switch(archiveType) {
            
            case TAR:

                TarInputStream tis = null;
                try {                                                                               // Input (.tar)
                    tis = new TarInputStream(new BufferedInputStream(new FileInputStream(archiveFile.getAbsoluteFile())));
                }
                catch(IOException fileIOEx)
                {
                    String msg = "Exception creating Input Stream with 'archiveFile' \n" + fileIOEx.toString();
                    logger.warning(msg);
                    removeMetaFlg();
                }
                
                TarEntry entry;
                try {
                    while ((entry = tis.getNextEntry()) != null) {

                        int byteCount;
                        byte data[] = new byte[2048];
                        String newFS_Item_Name = (dsTargetFolder.getPath() + File.separatorChar + entry.getName());
                        
                        if(entry.isDirectory()) {

                              // DataSet folders are branches under main 'dsTargetFolder'
                            File newDir = new File(newFS_Item_Name);
                            if(!newDir.exists()) {
                                try {
                                    Data_IO_Utils.createDir_orFile(newDir, FS_ObjectType.DIRECTORY);
                                }
                                catch(IOException e) {
                                    String msg = "Error creating Directory with method 'createDir_orFile(..)'" + 
                                                 "Path for 'newDir': " + newDir.getPath();
                                    logger.warning(msg);
                                    removeMetaFlg();
                                }
                            }
                        }
                        else {  // 'Entry' is File from (.tar) Archive                        
                            File newUnarchived = new File(newFS_Item_Name);
                            try {
                                Data_IO_Utils.createDir_orFile(newUnarchived, FS_ObjectType.FILE);
                            }
                            catch(IOException e) {
                                    String msg = "Error creating new File.'  \n" + 
                                                 "Path for 'newUnarchived': " + newUnarchived.getPath() + " \n" +
                                                 "Filename: " + newUnarchived.getName();
                                    logger.warning(msg);
                                    removeMetaFlg();
                            }

                            BufferedOutputStream dest = null;
                            try {
                                dest = new BufferedOutputStream(new FileOutputStream(newUnarchived));
                            }
                            catch(Exception e) {
                                String msg = "Error creating Output Stream(s).'  \n" + 
                                             "Path for wrapped 'newUnarchived' File: " + newUnarchived.getPath() + " \n" +
                                             "Wrapped filename: " + newUnarchived.getName();
                                logger.warning(msg);
                                removeMetaFlg();
                            }

                            try {
                                while ((byteCount = tis.read(data)) != -1) {
                                    dest.write(data, 0, byteCount);
                                }

                                dest.flush();
                                dest.close();
                            }
                            catch(Exception e) {

                                String msg = "Error transferring Bytes of data into Output from Tar-Input Stream.";
                                logger.warning(msg);                                
                                removeMetaFlg();
                            }
                        }
                    }
                }
                catch(Exception e) {
                    String msg = "Error attempting to get next Entry from Tar Stream.'  \n" + e.toString();
                    logger.warning(msg);
                    removeMetaFlg();
                }
                
                try {
                    tis.close();
                }
                catch(Exception e) {
                    String msg = "Error closing Tar Input Stream.'  \n" + e.toString();
                    logger.warning(msg);
                    removeMetaFlg();
                }
                break;
                
            default:
                logger.warning("Un-Archive type not yet supported.  Usable types include: 'TAR' (unix tarball)");
                removeMetaFlg();
        }        
    }

    @Override
    public void addCategory(LabelableListI samples)
    {
        m_images.put(samples.getName(), samples);
    }

    @Override
    public void addSample(String category, Labelable sam){
        
        Iterator vit = m_images.values().iterator();
        Iterator kit = m_images.keySet().iterator();
        boolean found = false;
        
        while (vit.hasNext()){
            
            LabelableListI sl = (LabelableListI)vit.next();
            String key = (String)kit.next();
            if (key.equals(category)){
                
                 // We found the list with that name.
                 sl.add(sam);
                 found = true;
            }
        }
        
        if (false == found){
            // Add this category to the list
            LabelableListI sl = new LabelableListI(category, this, null);
            sl.add(sam);
            m_images.put(category,sl);
        }
    }
    
    private void writeStatusFile()
    {
        try {
            Data_IO_Utils.createDir_orFile( m_metaStatusFile, FS_ObjectType.FILE );
        } catch (IOException ex) {
            logger.log(Level.WARNING, "can't write metadata file. this will cause Corpus to be downloaded again", ex);
        }
    }
    
    public void removeMetaFlg() {
        
        logger.warning("Removing 'Meta Flag: status.txt' due to error expanding-DataSet.");
        
        boolean success = m_metaStatusFile.delete();
        if(!success) {
            logger.severe("Unable to delete MetaFootprint after expansion failure.");
            logger.severe("Future expansions may be subject to incorrect 'DataSet / disk' state in next expansion.");
        }
    }
    
    @Override
    public void removeSample(String category){
        m_images.remove(category);
    }

    @Override
    public Properties getProperties() {
        Properties props = new Properties();
        props.setProperty("name", name);
        if (null != description)
            props.setProperty("description", description);
        else
            props.setProperty("description", "");
//        if (m_imageType == DataSet.DataSetImageType.VIDEO)
//            props.setProperty("imageType", "video");
//        else
//            props.setProperty("imageType", "image");
        
        return props;
    }
    
    @Override
    public void configureFromProperties(Properties config) 
            throws CorpusConfigurationException
    {
        properties = config;
    }
}