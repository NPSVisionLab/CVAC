/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package cvac.corpus;

import java.io.File;
import java.util.Properties;
import java.util.logging.Logger;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.*;

/**
 *
 * @author jonSchmid
 * Starting to port origin-Class test: 'DataSetTest.java' from MediaAnalyst tests
 * Currently Mirror related features are not addressed yet.
 */
public class CommonDataSetTest {
    
    CommonDataSet instance;
    File[] filesInOutputDir;
    private static String mediaRootDir = "C:/Users/LSO/Analyst_Media";
    private static Class<?> tgtClass;
    private String  Caltech101_URL =                                              // Caltech url used as test data set
                            "http://www.vision.caltech.edu/Image_Datasets/Caltech101/101_ObjectCategories.tar.gz",
                    Caltech256_URL = 
                            "http://www.vision.caltech.edu/Image_Datasets/Caltech256/256_ObjectCategories.tar",
                    unitTest_inputFileDir = (mediaRootDir + "/UnitTestData"),
                    web_decompress_unTar_OutputDir;
    
    
      // Select tests to use in Suite, the bottom two take a long time.
    boolean run_simpleTests = false;
    boolean run_Test_loadAsynch = true;
    
    public CommonDataSetTest() {
    }
    
    @BeforeClass
    public static void setUpClass() throws Exception {
        try {
            tgtClass = Class.forName("mediaanalyst.dataset.DataSet");
        } catch(Exception e) {
            String msg = "Unable to set the 'tgtClass' pointer to the DataSet class. \n" +
                         "Using  Class.forName('mediaanalyst.dataset.DataSet' " + e.toString();
            System.out.println(msg);
            throw(e);
        }
    }
    
    @AfterClass
    public static void tearDownClass() {
    }

    
    @Before
    public void setUp() {
        
        //String name, String description, String homepageURL, boolean isImmutableMirror
        //instance = new CommonDataSet();
        instance = null; // Temp, init object
    }
    
    @After
    public void tearDown() {
    }

    // ***
    @Test
    public void testAddCategory() {
    }
    
    // ***
    @Test
    public void testAddSample() {
    }

    // ***
    @Test
    public void testRemoveSample() {
        // Assert that 'm_images' no longer contains the removed String: 'category'
    }

    @Test
    public void testGetProperties() {
        Properties returnProps = instance.getProperties();
        
        assert(null != returnProps);
        // check for properties...
    }

/*  // Nothing to test, currently
    @Test
    public void testConfigureFromProperties() {
    }
*/
}
