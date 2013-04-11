package cvac.CVACcorpus;

import java.util.logging.Logger;
import java.util.logging.Level;
import java.util.Properties;
import java.util.ArrayList;
import java.io.FilenameFilter;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;



/**
 * Read the Corpus configuration file and create a CorpusI object.
 *
 * @author tomb, matz
 */
class CorpusConfig {

    protected static final Logger logger = Logger.getLogger(CorpusConfig.class.getName());
    private ArrayList<CorpusI> availCorpora;

    /*
     * Read the config file and configure the availCorporas;
     */

    CorpusConfig(){
        availCorpora = new ArrayList<CorpusI>();
    }

    ArrayList<CorpusI> getavailCorporas() {
        return availCorpora;
    }

    public void addDataSet(CorpusI ds){
        availCorpora.add(ds);
    }
    public boolean ReadDataSetConfig(String dataSetConfigDir){
               
        class myFilenameFilter implements FilenameFilter {
            public boolean accept(File dir, String name){
                if (name.endsWith(".properties")){
                    return true;
                }else {
                    return false;
                }
            }
        }
        
        File dataSetConfig = new File(dataSetConfigDir);
        File[] propFiles = dataSetConfig.listFiles(new myFilenameFilter());
        int i;

        if(null == propFiles)  // Don't throw a null pointer exception when no configs present
            return false;
        
        // Read each dataset config file in directory and process it
        for (i = 0; i < propFiles.length; i++){
            FileInputStream fis = null;
            Properties config = new Properties();
            try {
                 fis = new FileInputStream(propFiles[i]);
            }
            catch (IOException ioe)
            {
                 logger.throwing("Dataset", "opening config file", ioe);
                 continue;
            }
            try {
                 config.load( fis );
            }
            catch (IOException ioe)
            {
                 logger.throwing("Dataset", "loadConfiguration", ioe);
                 continue;
            }
            
            // check config file version compatibility
            String file_version = config.getProperty("dataset_config_version");
            double curr_file_version = 1.1;
            if (null == file_version || Double.parseDouble(file_version)<curr_file_version){
                logger.log(Level.WARNING, 
                        "Config file {0} is of a prior version, need to update it for {1}", 
                        new Object[]{propFiles[i].toString(),curr_file_version});
                continue;
            }

            // read properties common to all types of Corpus implementations
            String nameProp  = config.getProperty("name");
            if (null == nameProp){
                logger.log(Level.WARNING, "No name property in file {0} aborting", propFiles[i].toString());
                continue;
            }
            
            String descProp = config.getProperty("description");
            if (null == descProp){
                logger.log(Level.WARNING, "No description property in file {0}", propFiles[i].toString());
                continue;
            }
            
            String homepage = config.getProperty("homepage");
            if (null == homepage){
                logger.log(Level.INFO, "No homepage property in file {0}", propFiles[i].toString());
                homepage = "";
            }
            
            String imgTypeProp = config.getProperty("imageType");
            if (null == imgTypeProp){
                logger.log(Level.WARNING, "No imageType property in file {0}", propFiles[i].toString());
                continue;
            }
            
            CorpusI ds = null;
            String dsTypeProp = config.getProperty("datasetType");
            boolean isImmutableMirror = true;
            
            if (dsTypeProp == null || dsTypeProp.equalsIgnoreCase("common"))
            {
                ds = new CommonDataSet( nameProp, descProp, homepage, isImmutableMirror );
                ((CommonDataSet)ds).configureFromProperties(config);
            }
            else if (dsTypeProp.equalsIgnoreCase("labelme"))
            {
                ds = new LabelMeDataSet( nameProp, descProp, homepage, isImmutableMirror );
                ((LabelMeDataSet)ds).configureFromProperties(config);
            }
            else
            {
                logger.log(Level.WARNING, "Incorrect datasetType {0} in file {1}, assuming common",
                        new Object[]{dsTypeProp, propFiles[i].toString()});
                ds = new CommonDataSet( nameProp, descProp, homepage, isImmutableMirror );
                ((CommonDataSet)ds).configureFromProperties(config);
            }

            String mirrorsString = config.getProperty("mirrors");
            if (null == mirrorsString){
                logger.log(Level.WARNING, 
                        "No 'mirrors' property in file {0}, need at least 'main', skipping data set", 
                        propFiles[i].toString());
                continue;
            }
            String[] mirrors = mirrorsString.split(", |,");
            boolean got_main_location = false;
            for (String mirror : mirrors) 
                if (mirror.toLowerCase().equals("main")) { got_main_location = true; break; }
            if (!got_main_location){
                logger.log(Level.WARNING, 
                        "Need 'main' mirror with 'main_location' etc in {0}, skipping data set", 
                        propFiles[i].toString());
                continue;
            }

            
            String categoriesString  = config.getProperty("categories");
            if (null == categoriesString){
                logger.log(Level.FINE, "No categories specified in file {0}", propFiles[i].toString());
            }else {

                String[] categories = categoriesString.split(", |,");
                for (String category : categories) 
                {
                    if (category.contains(" "))
                    {
                        logger.log(Level.WARNING, 
                                "category {0} in file {1} must not contain a space, skipping category",
                                new Object[]{category, propFiles[i].toString()});
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
            
            logger.log(Level.INFO, "Adding DataSet properties from file {0}", propFiles[i].toString());
            availCorpora.add(ds);
        }
        
        if (availCorpora.size() > 0){
            return true;
        }else {
            return false;
        }
   
    }
}
