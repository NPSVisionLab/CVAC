package cvac.corpus;

import cvac.corpus.CorpusI.CorpusConfigurationException;
import java.lang.reflect.InvocationTargetException;
import java.util.logging.Logger;
import java.util.logging.Level;
import java.util.Properties;
import java.util.ArrayList;
import java.io.FilenameFilter;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.lang.reflect.Constructor;



/**
 * Read the Corpus configuration file and create a CorpusI object.
 *
 * @author tomb, matz
 */
class CorpusConfig {

    protected static final Logger logger = Logger.getLogger(CorpusServiceI.class.getName());
    private ArrayList<CorpusI> availCorpora;

    /*
     * Read the config file and configure the availCorporas;
     */

    CorpusConfig(){
        availCorpora = new ArrayList<CorpusI>();
    }

    ArrayList<CorpusI> getAvailableCorpora() {
        return availCorpora;
    }

    public void addCorpus(CorpusI ds){
        availCorpora.add(ds);
    }
    
    /** read all property files from a directory, create a corpus for each
     * 
     * @param corpusConfigDir where the "properties" files are located
     */
    public void readConfigurationsFromDir(String corpusConfigDir){
               
        class myFilenameFilter implements FilenameFilter {
            @Override
            public boolean accept(File dir, String name){
                if (name.endsWith(".properties")){
                    return true;
                }else {
                    return false;
                }
            }
        }
        
        File dataSetConfig = new File(corpusConfigDir);
        File[] propFiles = dataSetConfig.listFiles(new myFilenameFilter());
        int i;

        if(null == propFiles)  
        {
            // just return when no configs are present
            return;
        }
        
        // Read each dataset config file in directory and process it
        for (i = 0; i < propFiles.length; i++)
        {
            try {
                addCorpusFromConfig( propFiles[i] );
            }
            catch (CorpusI.CorpusConfigurationException ex)
            {
                 logger.throwing("Dataset", "opening config file", ex);
                 return;
            }
        }
    }
    
    /**
     * 
     * @param propFile
     * @return
     * @throws cvac.corpus.CorpusI.CorpusConfigurationException 
     */
    public CorpusI addCorpusFromConfig( File propFile )
        throws CorpusI.CorpusConfigurationException
    {
        FileInputStream fis = null;
        Properties config = new Properties();
        try {
            fis = new FileInputStream( propFile );
            config.load( fis );
        } catch (IOException ex) {
            throw new CorpusI.CorpusConfigurationException( ex.getMessage() );
        }
        CorpusI ds = parseCorpusProperties(config, propFile.toString());
        logger.log(Level.INFO, "Adding DataSet properties from file {0}", propFile.toString());
        if (null!=ds)
        {
            availCorpora.add(ds);
        }
        return ds;
    }

    // TODO: change "return null" to throwing an exception
    private CorpusI parseCorpusProperties(Properties config, String filename) 
            throws CorpusI.CorpusConfigurationException 
    {
        // check config file version compatibility
        String file_version = config.getProperty("dataset_config_version");
        double curr_file_version = 1.1;
        if (null == file_version || Double.parseDouble(file_version)<curr_file_version) {
            logger.log(Level.WARNING, 
                    "Config file {0} is of a prior version, need to update it for {1}", 
                    new Object[]{filename,curr_file_version});
            return null;
        }
        // read properties common to all types of Corpus implementations
        String nameProp  = config.getProperty("name");
        if (null == nameProp) {
            logger.log(Level.WARNING, "No name property in file {0} aborting", filename);
            return null;
        }
        String descProp = config.getProperty("description");
        if (null == descProp) {
            logger.log(Level.WARNING, "No description property in file {0}", filename);
            return null;
        }
        String homepage = config.getProperty("homepage");
        if (null == homepage){
            logger.log(Level.INFO, "No homepage property in file {0}", filename);
            homepage = "";
        }
        String imgTypeProp = config.getProperty("imageType");
        if (null == imgTypeProp) {
            logger.log(Level.WARNING, "No imageType property in file {0}", filename);
            return null;
        }
        CorpusI ds = null;

        // Datasets that have a "web" source such as Caltech101 will be downloaded
        // to some extent, and the local copy will be considered an 
        // "immutable mirror".  Only locally created datasets will be mutable.
        boolean isImmutableMirror = true;
        String dsTypeProp = config.getProperty("datasetType");
        if (dsTypeProp == null || dsTypeProp.equalsIgnoreCase("common"))
        {
            ds = new CommonDataSet( nameProp, descProp, homepage, isImmutableMirror );
        }
        else if (dsTypeProp.equalsIgnoreCase("labelme"))
        {
            // LabelMeDataSet might or might not have been compiled into jar,
            // obtain it dynamically and essentially do this:
            // ds = new LabelMeDataSet( nameProp, descProp, homepage, isImmutableMirror );
            try {
                // the parameterization with <?> gets rid of the obvious warning
                // "unchecked call to getConstructor"
                Class<?> labelMeDataSet = Class.forName("cvac.corpus.LabelMeDataSet");
                Constructor<?> cons = labelMeDataSet.getConstructor(
                        new Class[]{String.class, String.class, String.class, Boolean.class});
                cons.newInstance( nameProp, descProp, homepage, isImmutableMirror );
            }
            catch (InstantiationException ex) {
                logger.log(Level.WARNING, "Problem invoking LabelMe constructor:", ex);
            } catch (IllegalAccessException ex) {
                logger.log(Level.WARNING, "Problem invoking LabelMe constructor:", ex);
            } catch (IllegalArgumentException ex) {
                logger.log(Level.WARNING, "Problem invoking LabelMe constructor:", ex);
            } catch (InvocationTargetException ex) {
                logger.log(Level.SEVERE, "Problem invoking LabelMe constructor:", ex);
            } catch (ClassNotFoundException ex) {
                logger.log(Level.WARNING, 
                        "Cannot create LabelMe Corpus because your jar file does not include this class.", ex);
            } catch (NoSuchMethodException ex) {
                logger.log(Level.WARNING, "LabelMeDataSet does not seem to have the expected constructor", ex);
            } catch (SecurityException ex) {
                logger.log(Level.WARNING, "LabelMeDataSet does not seem to have a public constructor", ex);
            }
        }
        else
        {
            logger.log(Level.WARNING, "Incorrect datasetType {0} in file {1}, assuming common",
                    new Object[]{dsTypeProp, filename});
            ds = new CommonDataSet( nameProp, descProp, homepage, isImmutableMirror );
        }
        ds.configureFromProperties(config);
        String mirrorsString = config.getProperty("mirrors");
        if (null == mirrorsString) {
            logger.log(Level.WARNING, 
                    "No 'mirrors' property in file {0}, need at least 'main', skipping data set", 
                    filename);
            return null;
        }
        String[] mirrors = mirrorsString.split(", |,");
        boolean got_main_location = false;
        for (String mirror : mirrors) 
            if (mirror.toLowerCase().equals("main")) { got_main_location = true; break; }
        if (!got_main_location) {
            logger.log(Level.WARNING, 
                    "Need 'main' mirror with 'main_location' etc in {0}, skipping data set", 
                    filename);
            return null;
        }
        String categoriesString  = config.getProperty("categories");
        if (null == categoriesString){
            logger.log(Level.FINE, "No categories specified in file {0}", filename);
        }else {

            String[] categories = categoriesString.split(", |,");
            for (String category : categories) 
            {
                if (category.contains(" "))
                {
                    logger.log(Level.WARNING, 
                            "category {0} in file {1} must not contain a space, skipping category",
                            new Object[]{category, filename});
                    continue;
                }
                String cat_name = config.getProperty("category_" + category + "_name");
                if (null==cat_name)
                {
                    cat_name = category;
                }
                String cat_subdir = config.getProperty("category_" + category + "_subdir");
                if (null==cat_subdir)
                {
                    cat_subdir = category;
                }
                LabelableListI samplelist = new LabelableListI(cat_name, ds, cat_subdir);
                ds.addCategory(samplelist);
                logger.log(Level.FINE, "added category {0}: \"{1}\", subdir: {2}", 
                        new Object[]{category, cat_name, cat_subdir});
            }
        }
        return ds;
    }

    /** expect a directory with subdirectories, each of which is going
     * to be the label for all the files in the subdir.  The name is chosen
     * based on the path.  Other values are set to defaults.  They can be
     * edited in the properties file after saving the corpus and will be
     * assumed upon the next load.
     * 
     * @param directory The directory that contains the subdirectories.
     * @return A Corpus implementation.
     */
    CorpusI createCorpusFromPath( File directory ) throws CorpusConfigurationException 
    {
        String nameProp = directory.getName();
        String descProp = "Corpus created from directory " + nameProp;
        String homepage = "";
        boolean isImmutableMirror = false;
        CommonDataSet ds = new CommonDataSet( nameProp, descProp, homepage, isImmutableMirror );
        ds.loadImageAssets();
        
        Properties props = new Properties();
        props.setProperty("main_location", directory.getAbsolutePath() );
        ds.configureFromProperties( props );
        
        return ds;
    }
}
