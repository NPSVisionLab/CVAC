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
    private static String mediaRootDir = "C:/Users/LSO/Analyst_Media";  // Default
    private String  Caltech101_URL =                                              // Caltech url used as test data set
                            "http://www.vision.caltech.edu/Image_Datasets/Caltech101/101_ObjectCategories.tar.gz",
                    Caltech256_URL = 
                            "http://www.vision.caltech.edu/Image_Datasets/Caltech256/256_ObjectCategories.tar",
                    unitTest_inputFileDir = (mediaRootDir + "/UnitTestData"),
                    web_decompress_unTar_OutputDir;
    
    public CommonDataSetTest() {
    }
/**   
    @BeforeClass
    public static void setUpClass() throws Exception {
    }
    
    @AfterClass
    public static void tearDownClass() {
    }
*/
    
    @Before
    public void setUp() {
        
        //String name, String description, String homepageURL, boolean isImmutableMirror
        instance = new CommonDataSet("Caltech101", "Caltech101_URL", "http://www.vision.caltech.edu/Image_Datasets/Caltech101/101_ObjectCategories.tar.gz", true);
    }
    
    @After
    public void tearDown() {
        instance = null;
    }

    @Test
    public void testGetProperties() {
        Properties returnProps = instance.getProperties();
        
        assert(null != returnProps);
        // check for properties...
        
        Properties instProps = instance.getProperties();
        String nameStr = instProps.getProperty("name");
        assert(nameStr.equals("Caltech101"));
        String descStr = instProps.getProperty("description");
        assert(descStr.equals("Caltech101_URL"));
    }
}



//    // Nothing to test, currently
//    @Test
//    public void testConfigureFromProperties() {
//    }
//
//    // ***
//    @Test
//    public void testAddSample() {
//    }
//
//
//    // ***
//    @Test
//    public void testAddCategory() {
//        cvac.Labelable testLabel = new cvac.Labelable();
//        LabelableListI testSamples = new LabelableListI();
//        testSamples.add(testLabel);
//        
//        assert(instance.m_images.containsKey(testLabel));
//    }
//
//    // ***
//    @Test
//    public void testRemoveSample() {
//        
//        assert(instance.m_images.containsKey("category"));  // Must be there
//        
//        // Now assert 'm_images' no longer contains the removed String: "category"
//        instance.removeSample("category");
//        assert(false == instance.m_images.containsKey("category"));
//    }
