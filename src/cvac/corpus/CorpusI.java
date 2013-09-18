package cvac.corpus;

import cvac.CorpusCallbackPrx;
import cvac.FilePath;
import cvac.Labelable;
import cvac.Substrate;
import cvac.corpus.LabelableListI;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileFilter;
import java.io.FilenameFilter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.logging.Level;
import java.util.logging.Logger;

import cvac.Corpus;
import cvac.DirectoryPath;
import cvac.Label;
import cvac.Semantics;
import util.Data_IO_Utils;

/**
 * A corpus represents a set of images, videos, or other media,
 * often annotated with labels.
 * 
 * Corpora/datasets that have a "web" source such as Caltech101 will be downloaded
 * to some extent, and the local copy will be considered an 
 * "immutable mirror".  Only locally created datasets will be mutable.
 *
 * @author tomb, jcs, matz
 */
abstract public class CorpusI extends Corpus 
{
    static class CorpusConfigurationException extends Exception
    {

        protected CorpusConfigurationException(String string) {
            super( string );
        }
    }
    
    public static final String[] IMAGE_SUFFIXES = {".jpg",".jpeg",".png", ".pgm"};
    public static final String[] VIDEO_SUFFIXES = {".3g2", ".3gp", ".4xm", ".ajp", ".amc", ".amv", ".asf", ".ass", ".avi", ".bik", ".dpg", ".dsv", ".dv", ".dvf", ".dvx", ".flc", ".flv", ".gvi", ".hdmov", ".hdv", ".heu", ".ivf", ".k3g", ".m2p", ".m2ts", ".mjp", ".mkv", ".mmv", ".mod", ".modd", ".moff", ".mov", ".mp4", ".mpb", ".mpg", ".mswmm", ".mvb", ".mve", ".mxf", ".nsv", ".ogv", ".pva", ".rm", ".rms", ".rv", ".s2k", ".scm", ".smk", ".smv", ".sol", ".srt", ".ssa", ".str", ".sub", ".svi", ".tod", ".tp", ".ts", ".vc1", ".vid", ".vod", ".vp6", ".vp7", ".vqa", ".wm", ".wma", ".wmdb", ".wmv", ".xas", ".xma"};

    protected static Logger logger = Logger.getLogger(Corpus.class.getName());
    static String rootDataDir;  // set by CorpusServiceI during initialization
    public static void setLogger( Logger lg ) { logger = lg; }
    
    protected String m_dataSetFolder = null;     // Top-Level containing folder for DataSet
    
//    private LoaderThread              m_loader_thread;
    protected Map<String, LabelableListI> m_images;
    private boolean                   m_spacesUsed = false;
    private boolean errorShown = false;
    
    public CorpusI(String name, String description, String homepageURL, boolean isImmutableMirror)
    {
        super( name, description, homepageURL, isImmutableMirror );
        m_images = new HashMap<String, LabelableListI>();
        setDataDirBasedOnName();
//        m_loader_thread = null;
    }
    
    public String getHomepage() {
        return homepageURL;
    }

    public boolean pathsUseSpaces() {
        throw new RuntimeException("not implemented");
        // TODO: search through all samples and paths the first time, 
        // and store the result of whether there are spaces in the paths;
        // note that this might differ from mirror to mirror
        // m_spacesUsed = Code_Utils.stringHasSpaces(fullDatasetPath);
        // return m_spacesUsed;
    }
    
    /** Has a local mirror already been created?  This will return true only
      * if this corpus requires a download, not for one that is local to
      * begin with.
      */
    public abstract boolean localMirrorExists();

    // Advises if load operation is still ongoing
    private boolean hasFinishedLoading() {
        return true;
//        return (null!=m_loader_thread && m_loader_thread.isAlive());
    }

    private void setDataDirBasedOnName()
    {
        if(null != m_dataSetFolder) {
            throw new RuntimeException("we dont expect m_dataSetFolder to be set");
        }
        assert(null!=rootDataDir && !rootDataDir.equals(""));
        m_dataSetFolder = rootDataDir + File.separator + this.name;
    }

    public String getName() {
        return name;
    }

    public void setDescription(String d){
        if (null==d)
        {
            description = "";
        }
        else
        {
            description = d;
        }
    }

    public String getDescription() {
        return description;
    }
    
    void setHomepage(String homepage) {
        homepageURL = homepage;
    }
    
    public String getDatasetFolder() {
        return(m_dataSetFolder);
    }

    public Map<String, LabelableListI> getImages() {
        return m_images;
    }
    
    abstract public void addCategory(LabelableListI samples);
    
    abstract public void addSample(String category, Labelable sam);
    
    abstract public void removeSample(String category);

    abstract Labelable[] getLabels();

    abstract void createLocalMirror(CorpusCallbackPrx cb);

    /*
     *Load all the images
     * from top-level folder and all categories
     */
    public void loadImages()
    {
        errorShown = false;
        
        File dir = new File( m_dataSetFolder );
        LinkedList<LabelableListI> queue = new LinkedList<LabelableListI>();
        for(LabelableListI sl : m_images.values()){
            queue.add(sl);   
        }
        logger.log( Level.FINE, "loading images from corpus main data directory, {0}",
                    m_dataSetFolder );
        int added = loadImagesFromDir( name, dir, "", queue, false );
        if (0==added)
        {
            // don't worry too much about this, it's the root directory and not an empty category
            logger.log(Level.FINE, "Sample directory {0} does not exist or is empty", name);
        }
        LabelableListI sl;
        while (queue.peek() != null)
        {
            sl = queue.remove();
            File subdir = new File( m_dataSetFolder+File.separator+sl.getSubdir() );
            logger.log( Level.FINE, "loading images from corpus sub-directory, {0}",
                        subdir.getPath() );
            added = loadImagesFromDir(sl.getName(), subdir, sl.getSubdir(), queue, true);
            if (0==added)
            {
                // worry a bit more about this, this is an empty category
                logger.log(Level.WARNING, "Category directory {0} does not exist or is empty", 
                       subdir.getPath());
                // remove samplelist if its empty
                removeSample(sl.getName());
   
            }
        }
    }

    
    public class DirNameFilter implements FilenameFilter {
        @Override
        public boolean accept (File dir, String name)
        {
            // ignore any .name directories
            if ('.' == name.charAt(0))
                return false;
            File temp = new File(dir.toString() + File.separator + name);
            if (temp.isDirectory())
                return true;
            else
                return false;
        }
    }
    
    // Return true if we have this subdirectory in the sample list already
    public boolean samplesHasSubDir(String subdir){
        Collection<LabelableListI> samples = m_images.values();
        for (LabelableListI i : samples){
            if (i.getSubdir().matches(subdir))
                return true;
        }
        return false;
    }
    /*
     * Load all the images in this directory into this DataSet;
     * don't add a LabelableListI if no files are in this directory.
     *
     */
    protected int loadImagesFromDir(String labelName, File directory, 
                                    String subdir, LinkedList<LabelableListI> queue, 
                                    boolean recurse)
    {
        logger.log( Level.INFO, "loadImagesFromDir {0}, {1}, {2}", 
                    new Object[]{labelName, directory.getPath(), subdir } );
        LabelableListI dir_SampleList = m_images.get(labelName);
        if (null==dir_SampleList)
        {
            dir_SampleList = new LabelableListI(labelName, this, subdir);
        }
        // for now, clear list so we don't add
        // duplicates if the dataset gets "opened" multiple times
        dir_SampleList.clear(); 
        // add any directories we find if they are not already been added
        File subdirs[] = directory.listFiles(new DirNameFilter());
        if (null != subdirs && subdirs.length > 0){
            for (int i = 0; i < subdirs.length; i++){
                if (!samplesHasSubDir(subdirs[i].getName()) )
                {
                    // no idea what this does - will remove.
//                    String dir = subdirs[i].getAbsolutePath();
//                    String dsfolder = this.m_dataSetFolder;
//                    String replacedUnixPath = Data_IO_Utils.unixPathStr(dsfolder);
//                    String unixPath_RootDir = Data_IO_Utils.unixPathStr( rootDataDir );
//                    String folder = replacedUnixPath.replaceFirst("\\$MEDIA_ROOT", unixPath_RootDir);  // Backslashes convert to Reg-exp       
//                    int len = folder.length();
//                    String sub = dir.substring(len+1);
                    
                    // Since the data set can have folders within folders, we need
                    // to pass into the LabelablelistI this information.  Only the top level
                    // folders are the catagories.
                    String nextdir;
                    String sub = subdirs[i].getName();
                    if (sub.equals("")){
                        sub = getName();
                    }
                    if (directory.getPath().equals(m_dataSetFolder))
                        nextdir = sub;
                    else {
                        nextdir = directory.getPath().substring(this.m_dataSetFolder.length() + 1) +
                                   File.separator + sub;
                    }
  
                    LabelableListI samplelist = new LabelableListI(sub, this, nextdir);
                    this.addCategory(samplelist);   
                    queue.add(samplelist);
                }
            }
        }
        String relativePath = name;
        if (subdir.isEmpty())
        {
            subdir = "";  // ensure non-null for initializing Label
        }
        else
        {
            relativePath += "/" + subdir; // always forward slash never \
        }
        String lname = subdir;
        if (lname.equals(""))
            lname = labelName;
        Label label = new Label(true, lname, new HashMap<String,String>(0), new Semantics(""));

        float confidence = 1.0f;
        int added = dir_SampleList.addAllSamplesInDir( directory, label, confidence, 
                                                       new DirectoryPath( relativePath ), 
                                                       recurse );
        if (!dir_SampleList.isEmpty())
        {
            m_images.put(labelName, dir_SampleList);
        }
        return added;
    }
    
    public boolean isVideoFile(String name){
        int len = VIDEO_SUFFIXES.length;
        int i;
        for (i = 0; i < len; i++){
            String suffix = VIDEO_SUFFIXES[i];
            if (name.toLowerCase().endsWith(suffix))
                return true;
        }
        return false;
    }
    
    /*
     * Load this sample name with its images
     */
    public String loadSampleFromRemoteMirrorFile(String name, String base, BufferedReader file, String lastLine){
        LabelableListI dir_SampleList = m_images.get(name);
        int baseLen = base.length();
        if (null==dir_SampleList)
        {
            dir_SampleList = new LabelableListI(name, this, base);
        }
        // for now, clear list so we don't add
        // duplicates if the dataset gets "opened" multiple times
        dir_SampleList.clear(); 
        String line = null;
        try {
            boolean done = false;
            while (!done) {
                if (null != lastLine) {
                    line = lastLine;
                    lastLine = null;
                }else {
                    if (!file.ready()){
                        line = null;
                        break;
                    }
                    line = file.readLine();
                }
                int idx = line.lastIndexOf(base);
                if (-1 == idx){
                    // We are done with this sample so pop out
                    break;
                }
                // See if we have a new base that contains the name of the old base
                if ('/' != line.charAt(idx + base.length()))
                    break; // Its not really our old base do kick out
                idx += baseLen;
                if ('/' == line.charAt(idx))
                    idx++;
                String next = line.substring(idx);
                int idx2 = next.indexOf("/"); // can't use file separator since it might be different on remote.
                if (-1 != idx2){
                    //We have another level of directory structure
                    int idx3 = line.indexOf(base);
                    String newbase = line.substring(idx3,idx + idx2);
                    String newname = next.substring(0, idx2);
                    lastLine = loadSampleFromRemoteMirrorFile(newname, newbase, file, line);  
                }else {
                    //Add this name to the sample list
                    Substrate sub = null;
                    if (isVideoFile(line))
                    {
                        FilePath path = new FilePath(); // todo: use "line"
                        sub = new Substrate(false, true, path, -1, -1);
                    }else {
                        FilePath path = new FilePath(); // todo: use "line"
                        sub = new Substrate(true, false, path, -1, -1);
                    }
                   
                    Labelable sample = new Labelable();
                    sample.sub = sub;
                    dir_SampleList.add(sample);
                }
            }
        } catch(Exception e) {
            String msg = "Read error \n" +  e.toString();
            logger.warning(msg);
            line = null;
        }
        if (!dir_SampleList.isEmpty())
        {
            m_images.put(name, dir_SampleList);
        }
        return line;
    }
    
    public void loadImageAssets() {
        errorShown = false;
        // Clear out old images
        m_images.clear();
        
        // For now, assuming that only one 'topLevel_Imgs_Directory' is found
        LabelableListI dir_SampleList = null;
        
        // Grab all directories inside the DataSet folder, filter out '.meta'
        FileFilter all_nonMetaFolders = new FileFilter() {
            
            @Override
            public boolean accept(File file) {  // boolean Filter callback
                return (file.isDirectory() && !(".meta".equals(file.getName())));
            }
        };
        
        File[] topLevelContents = (new File(m_dataSetFolder).listFiles(all_nonMetaFolders));
        if (null == topLevelContents) {
            // Lets assume that the top level has all the images
            File curImageCatDir = new File(m_dataSetFolder);
            if(curImageCatDir.isDirectory()) {
            
                    String labelname = curImageCatDir.getName();
                    dir_SampleList = new LabelableListI(labelname, this, curImageCatDir.getName());
                    Label label = new Label(true, labelname, new HashMap<String,String>(0), new Semantics(""));
                    float confidence = 1.0f;
                    DirectoryPath relPath = new DirectoryPath( name+"/"+curImageCatDir );
                    boolean recurse = true;
                    int added = dir_SampleList.addAllSamplesInDir(curImageCatDir, label, 
                                                                  confidence, relPath, recurse);
                    if (0==added)
                    {
                       logger.log(Level.WARNING, "Sample directory {0} does not exist or is empty", 
                               curImageCatDir.getPath());
                    }
                    m_images.put(curImageCatDir.getName(), dir_SampleList);
            }
            if(0 == m_images.size()) {
                
                logger.warning("No images found in subdirectories of Dataset.");
            }
            return;
        }
        
        for(File topLevelDir : topLevelContents) {

            File[] imageCategoryDirs = topLevelDir.listFiles(all_nonMetaFolders);
            if (null == imageCategoryDirs || 0 == imageCategoryDirs.length){
                // We assume that all the images are in this directory
                String topDirName = topLevelDir.getName();
                dir_SampleList = new LabelableListI(topDirName, this, topDirName);
                Label label = new Label(true, topDirName, new HashMap<String,String>(0), new Semantics(""));
                float confidence = 1.0f;
                DirectoryPath relPath = new DirectoryPath( name+"/"+topDirName );
                boolean recurse = true;
                int added = dir_SampleList.addAllSamplesInDir(topLevelDir, label, confidence, relPath, recurse);
                if (0==added)
                {
                   logger.log(Level.WARNING, "Sample directory {0} does not exist or is empty", 
                           topLevelDir.getPath());
                }
                m_images.put(topDirName, dir_SampleList);
            }else {
                for(File curImageCatDir : imageCategoryDirs) {
                    if(!curImageCatDir.isDirectory()) {
                        logger.log(Level.WARNING, "File {0} not a sample.  Mixing samples withing sub-directories", curImageCatDir.getPath());
                    }
                    else { // Create and add Samples for all images in directory to its LabelableListI in the array

                        String labelname = curImageCatDir.getName();
                        dir_SampleList = new LabelableListI(labelname, this, curImageCatDir.getName());
                        Label label = new Label(true, labelname, new HashMap<String,String>(0), new Semantics(""));
                        float confidence = 0.0f;
                        DirectoryPath relPath = new DirectoryPath( name+"/"+labelname );
                        boolean recurse = true;
                        int added = dir_SampleList.addAllSamplesInDir(curImageCatDir, label, 
                                                                      confidence, relPath, recurse);
                        if (0==added)
                        {
                           logger.log(Level.WARNING, "Sample directory {0} does not exist or is empty", 
                                   curImageCatDir.getPath());
                           if (!errorShown){
                                errorShown = true;
                                logger.log(Level.WARNING, "No data in directory {0}", curImageCatDir.getPath());
                            }
                        }
                        m_images.put(curImageCatDir.getName(), dir_SampleList);
                    }
                }
            }
        }
        
        if(0 == m_images.size()) {
            logger.warning("No Category-directories were found containing images for the dataset.");
        }
    }
    /*
///////////////////////// Separarate Run Thread for Transfer-Processing ////////////////////////////////////////
    private class LoaderThread extends Thread
    {
        private final DataSetMirror m_mirror;
        private final DataSet.DataSetImageType m_imageType;
        LoaderThread(DataSetMirror fromMirror, DataSet.DataSetImageType itype)
        {
            this.m_mirror = fromMirror;
            m_imageType = itype;
        }
        
        @Override
        public void run() {

            if(null == m_name) {
                logger.warning("DataSet must receive a value for 'm_name' using 'setName(..)'.\n" +
                                 "Aborting run(..) method.");
                return;
            }
            
            m_mirror.loadImages();
        }
    }
*/
    abstract public Properties getProperties();
    
    abstract void configureFromProperties(Properties config)
        throws CorpusConfigurationException;
}
