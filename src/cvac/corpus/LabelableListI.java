/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package cvac.corpus;

import cvac.DirectoryPath;
import cvac.FilePath;
import cvac.Labelable;
import cvac.Substrate;
import java.io.File;
import java.io.FilenameFilter;
import java.util.ArrayList;

import cvac.Corpus;

/**
 * LabelableListI is either a category of images within a DataSet (such as "dolphins"
 * in the Caltech101 data set), or it is a collection of images that may or may
 * not have been created from a category.  Such collections are used in RunSets.
 * 
 * @author tomb
 */
public class LabelableListI extends ArrayList<Labelable> {

    /** user-given name of the Collection (which is a piece of the RunSet)
     */
    private String m_name;
    /** DataSet that this list came from, if any */
    private Corpus m_corpus;
    /**
     * Category (LabelableListI) within the above DataSet that these samples came from, if any
     */
    private LabelableListI m_dsCat;
    
    /** subdirectory, if any, relative to DataSet, in which the samples live
     * 
     */
    private String m_subdir;

    /**
     * Use this constructor if the LabelableListI is a derivative of a category, but
     * not a category itself.
     * @param ds The original LabelableListI that this LabelableListI was created from;
     * this pulls the DataSet link out of the origin
     */
    public LabelableListI(String name, LabelableListI origin) {
        super();
        m_name = name;
        if (null==origin)
        {
            m_corpus = null;
        } else {
            m_corpus = origin.getDataSet();
        }
        m_dsCat = origin;
        m_subdir = null;
    }

    /**
     * Use this constructor if this LabelableListI is directly part of a DataSet;
     * @param ds The DataSet that this LabelableListI belongs to, can be null
     * @param subdir The subdirectory under the DataSet's directory where this 
     * LabelableListI lives.  Must not include the 
     */
    public LabelableListI(String name, Corpus ds, String subdir) {
        super();
        m_name = name;
        m_corpus = ds;
        m_dsCat = null;
        m_subdir = subdir;
    }
    
    public LabelableListI() {
        super();
        m_name = null;
        m_corpus = null;
        m_dsCat = null;
        m_subdir = null;
    }

    public String getName() {
        return m_name;
    }

    protected void setName(String n) {
        m_name = n;
    }

    public Corpus getDataSet() {
        return m_corpus;
    }

    public void setDataSet(Corpus ds){
        m_corpus = ds;
    }

    public LabelableListI getDataSetCatagory() {
        return m_dsCat;
    }

    protected void setDataSetCatagory(LabelableListI ds){
        m_dsCat = ds;
    }

    String getSubdir() {
        return m_subdir;
    }
    
    void setSubdir(String dir) {
        m_subdir = dir;
    }
    public class ImageNameFilter implements FilenameFilter {
        public boolean accept (File dir, String name)
        {
            int len = CorpusI.IMAGE_SUFFIXES.length;
            int i;
            for (i = 0; i < len; i++){
                String suffix = CorpusI.IMAGE_SUFFIXES[i];
                if (name.toLowerCase().endsWith(suffix)) {
                    return true;
                }
            }
            return false;
        }
    }
    public class VideoNameFilter implements FilenameFilter {
        public boolean accept (File dir, String name)
        {
            int len = CorpusI.VIDEO_SUFFIXES.length;
            int i;
            for (i = 0; i < len; i++){
                String suffix = CorpusI.VIDEO_SUFFIXES[i];
                if (name.toLowerCase().endsWith(suffix)) {
                    return true;
                }
            }
            return false;
        }
    }
    /** LabelableI data points to each file
     * 
     * @param directory
     * @return the number of samples added
     */
    public int addAllSamplesInDir(File directory) 
    {
        boolean video = false;
        if (null != m_corpus){
            //if (m_corpus.getImageType() == Corpus.DataSetImageType.VIDEO) {
             //   video = true;
            //}
        }
        int cnt = 0;
        if (!video){
            File imageFiles[] = directory.listFiles(new ImageNameFilter());
            if (null != imageFiles && imageFiles.length > 0)
            { 
                for (int i = 0; i < imageFiles.length; i++){
                    // TODO: helper function createSubstrateFrom( directory, imageFiles[i] )
                    Labelable sample = new Labelable();
                    FilePath path;
                    path = new FilePath(new DirectoryPath(directory.getPath()), 
                            imageFiles[i].getName());
                    sample.sub = new Substrate(true, false, path, -1, -1);
                    this.add(sample);
                }
                cnt += imageFiles.length;
            }
        }else {
            // See if we have any video files
            File videoFiles[] = directory.listFiles(new VideoNameFilter());
            if (null != videoFiles && videoFiles.length > 0)
            { 
                for (int i = 0; i < videoFiles.length; i++){
                    Labelable sample = new Labelable();
                    FilePath path;
                    path = new FilePath(new DirectoryPath(directory.getPath()), 
                            videoFiles[i].getName());
                    sample.sub = new Substrate(false, true, path, -1, -1);
                    this.add(sample);
                }
                cnt +=  videoFiles.length;
            }
        }
        return cnt;
    }
    
    // For now we assume that the samples are either all video or all images and not mixed!
    // So if we our first sample is video then the rest must be.
    public boolean hasVideo() {
        if (this.size() == 0) {
            return false;
        }
        Labelable sample = this.get(0);
        return sample.sub.isVideo;
    }
}
