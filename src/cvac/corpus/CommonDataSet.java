package cvac.corpus;

import com.ice.tar.TarEntry;
import com.ice.tar.TarInputStream;
import cvac.CorpusCallbackPrx;
import cvac.Labelable;
import org.apache.commons.io.FileUtils;
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
import java.util.zip.ZipInputStream;
import java.util.zip.ZipEntry;
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
    boolean labelsLoaded;

    public enum DataSetLocType {LOCAL, URL, STREAMING, REMOTE, LABELME}
    public enum CompressionType {GZIP, BZ2, Z, UNCOMPRESSED, ZIP}
    public enum ArchiveType {A, AR, CPIO, SHAR, LBR, ISO, TAR, UNARCHIVED}
    
    private File m_metaDataFolder, // '.meta' sub-folder of local corpus folder
                 m_metaStatusFile;   // File: 'status.txt'
    
    public CommonDataSet(String name, String description, String homepageURL, boolean isImmutableMirror)
    {
        super(name, description, homepageURL, isImmutableMirror);
        labelsLoaded = false;
                
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
    
    CompressionType getCompressionType( String type )
    {
        if (type.equals("gzip") || type.equals("unarchived"))
            return CompressionType.GZIP;
        else if (type.equals("zip"))
            return CompressionType.ZIP;
        else 
            throw new RuntimeException("compression type " + type + " not recognized");
    }
    
    ArchiveType getArchiveType( String type )
    {
        if (type.equals("none"))
            return ArchiveType.UNARCHIVED;
        else if (type.equals("tar"))
            return ArchiveType.TAR;
        else 
            throw new RuntimeException("compression type " + type + " not recognized");
    }

    void moveAllFiles( File fromDir, File toDir )
    {
        File files[] = fromDir.listFiles();
        if (null!=files)
        {
            for ( File file : files )
            {
                if ( file.getName().equals(".") 
                     || file.getName().equals("..")
                     || file.getName().equals(".meta"))
                {
                    continue;
                }
                if ( file.isDirectory() )
                {
                    try {
                        FileUtils.moveDirectory( file, toDir );
                    }
                    catch (IOException ioe)
                    {
                        logger.log( Level.WARNING, "couldn't move directory {0}\n",
                                    file.getPath() );
                    }
                }
                else
                {
                    try {
                        FileUtils.moveFileToDirectory( file, toDir, false );
                    }
                    catch (IOException ioe)
                    {
                        logger.log( Level.WARNING, "couldn't move file {0}\n",
                                    file.getPath() );
                    }
                }
            }
        }
    }
    
    @Override
    void createLocalMirror( CorpusCallbackPrx cb )
    {
        // does a local mirror exist already?
        if ( localMirrorExists() )
        {
            logger.log( Level.INFO, "Local mirror for Corpus {0} exists already.", this.name );
            loadImages();
            labelsLoaded = true;
            return;
        }
        
        // download and uncompress the Corpus archive
        String main_location = properties.getProperty("main_location");
        File ml = new File( main_location );
        String fname = ml.getName();
        File localArchiveFile = new File( m_metaDataFolder + File.separator + fname );
        String decompressed = "decompressed.tar";
        CompressionType compressionType = 
            getCompressionType( properties.getProperty("main_compressType") );
        ArchiveType archiveType = 
            getArchiveType( properties.getProperty("main_archiveType") );
        expandDataset_toLocal( main_location, localArchiveFile, decompressed, 
                               m_dataSetFolder, compressionType, archiveType, false );

        // if specified, move the data from this subdir to the main dataset dir
        String moveSubdir = properties.getProperty("main_subdir");
        System.out.println("---------- got main_subdir: " + moveSubdir);
        if (null!=moveSubdir && !moveSubdir.isEmpty())
        {
            String fromDir = m_dataSetFolder + File.separator + moveSubdir;
            moveAllFiles( new File(fromDir), new File(m_dataSetFolder) );
            logger.log( Level.INFO, "Moved files from subdir {0} to main dataset dir.",
                        fromDir );
        }

        // if no errors, write a little metadata file
        writeStatusFile();
        
        // read in the Label data
        loadImages();
        labelsLoaded = true;
    }

    /** Has a local mirror already been created?  This will return true only
      * if this corpus requires a download, not for one that is local to
      * begin with.
      */
    @Override
    public boolean localMirrorExists()
    {
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
            //            throw new RuntimeException("data dir exists already but no metadata file ("
            //      +m_metaStatusFile.getAbsolutePath()
            //      +"); aborting because no plan B available");
        }
        return false;
    }
   
    
      // Orchestrate entire unpacking sequence
    protected void expandDataset_toLocal( String webURL, File web_SavedFile, String decompressedOutputName,
                                          String expandBelow_Path, CompressionType uncompressType,
                                          ArchiveType archiveType, boolean forceDownload)
    {
        // Grab file to local disk, if not local already
        if ( web_SavedFile.exists() && !forceDownload )
        {
            logger.log( Level.FINE, "Local file exists, not downloading again\n" );
        }
        else if ( !web_SavedFile.exists() || forceDownload )
        {
            logger.log( Level.FINE, "Downloading corpus from {0}\n", webURL );
            try {
                DownloadUtils.URL_readToFile(webURL, web_SavedFile);
            }
            catch(IOException e) {
                String msg = "Exception in URL_readToFile: " + e.toString();
                logger.warning(msg);
            }
        }

        File archiveFile = null;
        if(CompressionType.GZIP == uncompressType)
        {
            archiveFile = new File( m_metaDataFolder.getAbsolutePath() +
                                  File.separatorChar + decompressedOutputName ); 

            try {
                uncompressDataset(web_SavedFile, archiveFile, 
                                  expandBelow_Path, uncompressType);
                logger.log(Level.INFO, "Uncompressed dataset into {0}", 
                           archiveFile.getName());
            }
            catch(IOException e)
            {
                throw new RuntimeException("Error uncompressing DataSet with GZIP\n"
                                           + e.toString());
            }
        }
        else if(CompressionType.ZIP == uncompressType) 
        {
            try {
                uncompressDataset(web_SavedFile, null,
                                  expandBelow_Path, uncompressType);
                logger.log(Level.INFO, "Uncompressed dataset into {0}\n", 
                           expandBelow_Path);
            }
            catch(IOException e)
            {
                throw new RuntimeException("Error uncompressing DataSet with ZIP\n"
                                           + e.toString());
            }
        }
        else {
            archiveFile = new File(web_SavedFile.getPath());  // Web URL provided uncompressed (.tar)
        }
        if (ArchiveType.UNARCHIVED != archiveType)
        {
            // TODO: why was there a new dataset created here?
            //DataSet dataSetInstance = new DataSet();
            //dataSetInstance.unpackArchiveBelow(archiveFile, expandBelow_Path, ArchiveType.TAR);
            unpackArchiveBelow(archiveFile, expandBelow_Path, archiveType);
        }
    }
    
    // Read source-File, uncompress and save into destination-File in the 'source' directory
    protected void uncompressDataset(File compressedFileSrc, File destFile, String destFileDir, 
                                     CompressionType in_type) throws IOException {

          // Todo, operate on generic src and dest Stream Objects after switch code
        switch(in_type) {
            
            case GZIP:  // GNU Gunzip
            {
                // Create new file if target !exists, or Uncompress over an existing target
                if ((null == destFile) || (!destFile.exists())) {
                    Data_IO_Utils.createDir_orFile(destFile, FS_ObjectType.FILE);
                }
        
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
            }

            case ZIP:
            {
                final int UNZIP_BUFFER = 2048;
                byte buff[];
                int amtRead;
                ZipInputStream zis = null;
                String outFilename = "";

                try {
                    zis = new ZipInputStream(new FileInputStream(compressedFileSrc));
                    ZipEntry entry;
 
                    while ((entry = zis.getNextEntry()) != null) {
                        outFilename = destFileDir + File.separator + entry.getName();
                        if (entry.isDirectory())
                        {
                            logger.info("Creating directory: " + outFilename);
                            boolean success = (new File(outFilename)).mkdirs();
                            if (!success)
                            {
                                logger.warning("Could not create directory, anticipate further unzip problems: "
                                               + outFilename);
                            }
                            continue;
                        }
                        logger.info("Unzipping: " + outFilename);
 
                        int size;
                        byte[] buffer = new byte[2048];
 
                        FileOutputStream fos =
                            new FileOutputStream(outFilename);
                        BufferedOutputStream bos =
                            new BufferedOutputStream(fos, buffer.length);
 
                        while ((size = zis.read(buffer, 0, buffer.length)) != -1) {
                            bos.write(buffer, 0, size);
                        }
                        bos.flush();
                        bos.close();
                    }
                }
                catch(FileNotFoundException e) {
                    String msg = ("Error using specified files for extracting with zip.\n" + 
                                  "Compressed Source: " + compressedFileSrc.getPath() + "\n" +
                                  "Destination File: "  + outFilename + "\n" + e.toString());
                    logger.warning(msg);
                    removeMetaFlg();
                }
                    catch(Exception e) {
                    String msg = ("Error while extracting from zip-Input-Stream." + e.toString());
                    logger.warning(msg);
                }
                break;
            }

            default:
                logger.severe("De-compression type not yet supported.  Usable types include: ZIP, GZIP.");
                removeMetaFlg();
        }
    }

    // Creates a new folder using the Dataset's name String to create folder containing all dataset content
    protected void unpackArchiveBelow(File archiveFile, String expandBelow_Path, ArchiveType archiveType) {
        File dsTargetFolder = new File(expandBelow_Path);        
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
    
    @Override
    Labelable[] getLabels()
    {
        if ( !labelsLoaded )
        {
            // read in the Label data
            loadImages();
            labelsLoaded = true;
        }
        // There's probably a much more elegant way to do this...
        LabelableListI[] llistA = m_images.values().toArray( new LabelableListI[0] );
        int totalSize = 0;
        for ( LabelableListI list : llistA )
        {
            totalSize += list.size();
        }
        Labelable[] labels = new Labelable[totalSize];
        int icnt = 0;
        for ( LabelableListI list : llistA )
        {
            for ( Labelable label : list )
            {
                labels[icnt++] = label;
            }
        }
        logger.log( Level.INFO, "Created list of {0} labels from corpus {1}.",
                    new Object[]{totalSize, name} );
        return labels;
    }
}