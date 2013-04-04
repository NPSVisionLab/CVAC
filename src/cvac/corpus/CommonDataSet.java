package cvac.corpus;

//import com.ice.tar.TarEntry;
//import com.ice.tar.TarInputStream;
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
import java.util.zip.GZIPInputStream;

/**
 * All common data sets use functionality in this class.  Samples and
 * categories can be added, removed etc.
 * LabelMe, Vatic etc are in a separate class.
 *
 * @author matz
 */
public class CommonDataSet extends CorpusI {

//    protected DataSet_Metadata        m_metadata;
    
    public CommonDataSet(String name, String description, String homepageURL, boolean isImmutableMirror)
    {
        super( name, description, homepageURL, isImmutableMirror );
//        m_metadata = null; // needs m_dataSetFolder to be set
    }
//    
//    public void setName(String n) {
//
//        m_name = n;
//        
//        m_dataSetFolder = (Main.getEngine().getDataDir() + File.separator + m_name);
//        m_metadata = new DataSet_Metadata(this);        
//    }
//    
//    public void loadDataSet(DataSetMirror from_mirror) {
//        m_metadata.loadDataSet(from_mirror);
//    }
//
//    /*
//     * Create samples for each file in the dataset on the remote mirror.
//     */
//    public void loadSamplesFromRemoteMirror( DataSetMirror mirror )
//    {
//        String fileName = m_metadata.getMirrorListFile( mirror );
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
//        loadSampleFromRemoteMirrorFile(m_name, folder, rlist, null);
//        
//       
//    }
//    
//    
//      // Orchestrate entire unpacking sequence
//    protected void expandDataset_toLocal( String webURL, File web_SavedFile, String decompressedOutputName,
//                                          String expandBelow_Path, DataSetMirror.CompressionType uncompressType) throws MediaAnalystException {
//        // Grab file to local disk
//        try {
//            DownloadUtils.URL_readToFile(webURL, web_SavedFile);
//        }
//        catch(IOException e) {
//            String msg = "Exception in URL_readToFile: " + e.toString();
//            logger.warning(msg);
//        }
//
//        File tarOutput;
//        if(DataSetMirror.CompressionType.GZIP == uncompressType) {  // Unzipping GZip Produces (.tar) in /expansion/ folder
//            tarOutput = new File( m_metadata.getSubfolderPath() + 
//                                  File.separatorChar + "expansion" +
//                                  File.separatorChar + decompressedOutputName ); 
//
//            try {
//                uncompressDataset(web_SavedFile, tarOutput, expandBelow_Path, uncompressType);
//
//                String msg = "Uncompressed Dataset with 'GZIP', saved: " + tarOutput.getName();
//                logger.log(Level.INFO, msg);
//            }
//            catch(Exception e)
//            {
//                throw new MediaAnalystException("Error uncompressing DataSet with 'GZIP' protocol.  \n" + e.toString());
//            }
//        }
//        else {
//            tarOutput = new File(web_SavedFile.getPath());  // Web URL provided uncompressed (.tar)
//        }
//        File targetDatasetFolder = new File(expandBelow_Path);
//
//        // TODO: why was there a new dataset created here?
//        //DataSet dataSetInstance = new DataSet();
//        //dataSetInstance.unpackArchiveBelow(tarOutput, targetDatasetFolder, DataSetMirror.ArchiveType.TAR);
//        unpackArchiveBelow(tarOutput, targetDatasetFolder, DataSetMirror.ArchiveType.TAR);
//    }
//    
//    // Read source-File, uncompress and save into destination-File in the 'source' directory
//    private void uncompressDataset(File compressedFileSrc, File destFile, String destFileDir, DataSetMirror.CompressionType in_type) throws IOException {
//        
//          // Create new file if target !exists, or Uncompress over an existing target
//        if ((null == destFile) || (!destFile.exists())) {
//            Data_IO_Utils.createDir_orFile(destFile, FS_ObjectType.FILE);
//        }
//        
//          // Todo, operate on generic src and dest Stream Objects after switch code
//        switch(in_type) {
//            
//            case GZIP:  // GNU Gunzip
//                final int GUNZIP_BUFFER = 2048;
//                byte buff[];
//                int amtRead;
//                FileOutputStream outToFile = null;
//                GZIPInputStream GZIP_InStrm = null;
//
//                try {
//                    GZIP_InStrm = new GZIPInputStream(new FileInputStream(compressedFileSrc));
//                    outToFile = new FileOutputStream(destFile);
//                    buff = new byte[GUNZIP_BUFFER];
//
//                    while ((amtRead = GZIP_InStrm.read(buff)) > 0) {
//                        outToFile.write(buff, 0, amtRead);  // Transfer blocks of data 'src' to 'dest'
//                    }
//                    GZIP_InStrm.close();
//                    outToFile.close();
//                }
//                catch(FileNotFoundException e) {
//                    String msg = ("Error using specified files for extracting with Gzip.\n" + 
//                                  "Compressed Source: " + compressedFileSrc.getPath() + "\n" +
//                                  "Destination File: "  + destFile.getPath()    + "\n" + e.toString());
//                    logger.warning(msg);
//                    m_metadata.removeMetaFlg();
//                }
//                    catch(Exception e) {
//                    String msg = ("Error while extracting from Gzip-Input-Stream." + e.toString());
//                    logger.warning(msg);
//                }
//                break;
//
//            default:
//                logger.warning("De-compression type not yet supported.  Usable types include: 'GZIP' (Gnu-Gunzip).  ");
//                m_metadata.removeMetaFlg();
//        }
//    }
//
//    // Creates a new folder using the Dataset's m_name String to create folder containing all dataset content
//    private void unpackArchiveBelow(File archiveFile, File dsTargetFolder, DataSetMirror.ArchiveType archiveType) {
//        
//        Data_IO_Utils.dieRuntime_IfNull(dsTargetFolder, 
//                                        "'contentFolder' File-object is not allowed to be null in 'unpackArchiveBelow(..)'");
//        switch(archiveType) {
//            
//            case TAR:
//
//                TarInputStream tis = null;
//                try {                                                                               // Input (.tar)
//                    tis = new TarInputStream(new BufferedInputStream(new FileInputStream(archiveFile.getAbsoluteFile())));
//                }
//                catch(IOException fileIOEx)
//                {
//                    String msg = "Exception creating Input Stream with 'archiveFile' \n" + fileIOEx.toString();
//                    logger.warning(msg);
//                    m_metadata.removeMetaFlg();
//                }
//                
//                TarEntry entry;
//                try {
//                    while ((entry = tis.getNextEntry()) != null) {
//
//                        int byteCount;
//                        byte data[] = new byte[2048];
//                        String newFS_Item_Name = (dsTargetFolder.getPath() + File.separatorChar + entry.getName());
//                        
//                        if(entry.isDirectory()) {
//
//                              // DataSet folders are branches under main 'dsTargetFolder'
//                            File newDir = new File(newFS_Item_Name);
//                            if(!newDir.exists()) {
//                                try {
//                                    Data_IO_Utils.createDir_orFile(newDir, FS_ObjectType.DIRECTORY);
//                                }
//                                catch(IOException e) {
//                                    String msg = "Error creating Directory with method 'createDir_orFile(..)'" + 
//                                                 "Path for 'newDir': " + newDir.getPath();
//                                    logger.warning(msg);
//                                    m_metadata.removeMetaFlg();
//                                }
//                            }
//                        }
//                        else {  // 'Entry' is File from (.tar) Archive                        
//                            File newUnarchived = new File(newFS_Item_Name);
//                            try {
//                                Data_IO_Utils.createDir_orFile(newUnarchived, FS_ObjectType.FILE);
//                            }
//                            catch(IOException e) {
//                                    String msg = "Error creating new File.'  \n" + 
//                                                 "Path for 'newUnarchived': " + newUnarchived.getPath() + " \n" +
//                                                 "Filename: " + newUnarchived.getName();
//                                    logger.warning(msg);
//                                    m_metadata.removeMetaFlg();
//                            }
//
//                            BufferedOutputStream dest = null;
//                            try {
//                                dest = new BufferedOutputStream(new FileOutputStream(newUnarchived));
//                            }
//                            catch(Exception e) {
//                                String msg = "Error creating Output Stream(s).'  \n" + 
//                                             "Path for wrapped 'newUnarchived' File: " + newUnarchived.getPath() + " \n" +
//                                             "Wrapped filename: " + newUnarchived.getName();
//                                logger.warning(msg);
//                                m_metadata.removeMetaFlg();
//                            }
//
//                            try {
//                                while ((byteCount = tis.read(data)) != -1) {
//                                    dest.write(data, 0, byteCount);
//                                }
//
//                                dest.flush();
//                                dest.close();
//                            }
//                            catch(Exception e) {
//
//                                String msg = "Error transferring Bytes of data into Output from Tar-Input Stream.";
//                                logger.warning(msg);                                
//                                m_metadata.removeMetaFlg();
//                            }
//                        }
//                    }
//                }
//                catch(Exception e) {
//                    String msg = "Error attempting to get next Entry from Tar Stream.'  \n" + e.toString();
//                    logger.warning(msg);
//                    m_metadata.removeMetaFlg();
//                }
//                
//                try {
//                    tis.close();
//                }
//                catch(Exception e) {
//                    String msg = "Error closing Tar Input Stream.'  \n" + e.toString();
//                    logger.warning(msg);
//                    m_metadata.removeMetaFlg();
//                }
//                break;
//                
//            default:
//                logger.warning("Un-Archive type not yet supported.  Usable types include: 'TAR' (unix tarball)");
//                m_metadata.removeMetaFlg();
//        }        
//    }

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
                
                 // We found the list with that m_name.
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
    
    
    public boolean configureFromProperties(Properties config) {
        return true;
    }
}
