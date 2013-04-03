package cvac.corpus;

import cvac.FilePath;
import cvac.Labelable;
import cvac.Substrate;
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

/*
import mediaanalyst.GUI_Desktop.Constants;
import mediaanalyst.Main;
import mediaanalyst.dataset.DataSetMirror.DataSetLocType;
import mediaanalyst.utility_code.Code_Utils;
import mediaanalyst.utility_code.Data_IO_Utils;
*/
/**
 * A corpus represents a set of images, videos, or other media,
 * often annotated with labels.
 * @author tomb, jcs, matz
 */
abstract public class CorpusI extends Corpus {
    public static final String[] IMAGE_SUFFIXES = {".jpg",".jpeg",".png", ".pgm"};
    public static final String[] VIDEO_SUFFIXES = {".3g2", ".3gp", ".4xm", ".ajp", ".amc", ".amv", ".asf", ".ass", ".avi", ".bik", ".dpg", ".dsv", ".dv", ".dvf", ".dvx", ".flc", ".flv", ".gvi", ".hdmov", ".hdv", ".heu", ".ivf", ".k3g", ".m2p", ".m2ts", ".mjp", ".mkv", ".mmv", ".mod", ".modd", ".moff", ".mov", ".mp4", ".mpb", ".mpg", ".mswmm", ".mvb", ".mve", ".mxf", ".nsv", ".ogv", ".pva", ".rm", ".rms", ".rv", ".s2k", ".scm", ".smk", ".smv", ".sol", ".srt", ".ssa", ".str", ".sub", ".svi", ".tod", ".tp", ".ts", ".vc1", ".vid", ".vod", ".vp6", ".vp7", ".vqa", ".wm", ".wma", ".wmdb", ".wmv", ".xas", ".xma"};

    protected static Logger logger = Logger.getLogger(Corpus.class.getName());
    public static void setLogger( Logger lg ) { logger = lg; }
    
    protected String                  m_name,
                                      m_description,
                                      m_dataSetFolder;     // Top-Level containing folder for DataSet
    protected String                  m_homepage;
    
//    private LoaderThread              m_loader_thread;
    protected Map<String, LabelableListI> m_images;
    private boolean                   m_spacesUsed = false;
    private boolean errorShown = false;
    
    public CorpusI() {
        m_images = new HashMap<String, LabelableListI>();
//        m_loader_thread = null;
    }
    
    public String getHomepage() {
        return m_homepage;
    }

    public boolean pathsUseSpaces() {
        throw new RuntimeException("not implemented");
        // TODO: search through all samples and paths the first time, 
        // and store the result of whether there are spaces in the paths;
        // note that this might differ from mirror to mirror
        // m_spacesUsed = Code_Utils.stringHasSpaces(fullDatasetPath);
        // return m_spacesUsed;
    }

    // Advises if load operation is still ongoing
    public boolean isStillLoading() {
        return false;
//        return (null!=m_loader_thread && m_loader_thread.isAlive());
    }

    abstract public void setName(String n);

    public String getName() {
        return m_name;
    }

    public void setDescription(String d){
        m_description = d;
        if (null==d)
        {
            m_description = "";
        }
    }

    public String getDescription() {
        return m_description;
    }
    
    void setHomepage(String homepage) {
        m_homepage = homepage;
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

    /*
     *Load all the images
     * from top-level folder and all categories
     */
    /*
    public void loadImages(DataSetMirror dsm)
    {
        errorShown = false;
        String iLoc = dsm.getInitLocation();
        
        // Substitute System media_root_dir for '$MEDIA_ROOT' substring 
        if(DataSetLocType.LOCAL == dsm.getInitLocationType()) {
            
            String replacedUnixPath = Data_IO_Utils.unixPathStr(iLoc);
            String mediaRootDirStr = Main.getEngine().getDataDir().getPath();
            String unixPath_RootDir = Data_IO_Utils.unixPathStr(mediaRootDirStr);
            replacedUnixPath = replacedUnixPath.replaceFirst("\\$MEDIA_ROOT", unixPath_RootDir);  // Backslashes convert to Reg-exp

            iLoc = replacedUnixPath;
        }
        
        File dir = new File(iLoc);
        LinkedList<LabelableListI> queue = new LinkedList<LabelableListI>();
        for(LabelableListI sl : m_images.values()){
            queue.add(sl);   
        }
        int added = loadImagesFromDir(m_name, dir, "", queue);
        if (0==added)
        {
            // don't worry too much about this, it's the root directory and not an empty category
            logger.log(Level.FINE, "Sample directory {0} does not exist or is empty", m_name);
        }
        LabelableListI sl;
        while (queue.peek() != null)
        {
            sl = queue.remove();
            File subdir = new File( iLoc+File.separator+sl.getSubdir() );
            added = loadImagesFromDir(sl.getName(), subdir, sl.getSubdir(), queue);
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
     * 
     */
    
    public class DirNameFilter implements FilenameFilter {
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
    protected int loadImagesFromDir(String name, File directory, String subdir, LinkedList<LabelableListI> queue){
        LabelableListI dir_SampleList = m_images.get(name);
        if (null==dir_SampleList)
        {
            dir_SampleList = new LabelableListI(name, this, subdir);
        }
        // for now, clear list so we don't add
        // duplicates if the dataset gets "opened" multiple times
        dir_SampleList.clear(); 
        // add any directories we find if they are not already been added
        File subdirs[] = directory.listFiles(new DirNameFilter());
        if (null != subdirs && subdirs.length > 0){
            for (int i = 0; i < subdirs.length; i++){
                if (!samplesHasSubDir(subdirs[i].getName()) ){
                    String dir = subdirs[i].getAbsolutePath();
                    String dsfolder = this.m_dataSetFolder;
                    String replacedUnixPath = "TODO"; // Data_IO_Utils.unixPathStr(dsfolder);
                    String mediaRootDirStr = "TODO"; //Main.getEngine().getDataDir().getPath();
                    String unixPath_RootDir = "TODO"; //Data_IO_Utils.unixPathStr(mediaRootDirStr);
                    String folder = replacedUnixPath.replaceFirst("\\$MEDIA_ROOT", unixPath_RootDir);  // Backslashes convert to Reg-exp       
                    int len = folder.length();
                    String sub = dir.substring(len+1);
                    LabelableListI samplelist = new LabelableListI(sub, this, sub);
                    this.addCategory(samplelist);   
                    queue.add(samplelist);
                }
            }
        }
        int added = dir_SampleList.addAllSamplesInDir(directory);
        if (!dir_SampleList.isEmpty())
        {
            m_images.put(name, dir_SampleList);
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
            
            public boolean accept(File file) {  // boolean Filter callback
                return (file.isDirectory() && !(".meta".equals(file.getName())));
            }
        };
        
        File[] topLevelContents = (new File(m_dataSetFolder).listFiles(all_nonMetaFolders));
        if (null == topLevelContents) {
            // Lets assume that the top level has all the images
            File curImageCatDir = new File(m_dataSetFolder);
            if(curImageCatDir.isDirectory()) {
            
                    dir_SampleList = new LabelableListI(curImageCatDir.getName(), this, curImageCatDir.getName());
                    int added = dir_SampleList.addAllSamplesInDir(curImageCatDir);
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
                String name = topLevelDir.getName();
                dir_SampleList = new LabelableListI(name, this, name);
                int added = dir_SampleList.addAllSamplesInDir(topLevelDir);
                if (0==added)
                {
                   logger.log(Level.WARNING, "Sample directory {0} does not exist or is empty", 
                           topLevelDir.getPath());
                }
                m_images.put(name, dir_SampleList);
            }else {
                for(File curImageCatDir : imageCategoryDirs) {
                    if(!curImageCatDir.isDirectory()) {
                        logger.log(Level.WARNING, "File {0} not a sample.  Mixing samples withing sub-directories", curImageCatDir.getPath());
                    }
                    else { // Create and add Samples for all images in directory to its LabelableListI in the array

                        dir_SampleList = new LabelableListI(curImageCatDir.getName(), this, curImageCatDir.getName());
                        int added = dir_SampleList.addAllSamplesInDir(curImageCatDir);
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
}
