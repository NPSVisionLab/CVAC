/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package cvac.corpus;

import java.io.File;
import java.io.IOException;
import java.util.Properties;
import org.junit.Test;
//import static org.junit.Assert.*;
import util.Data_IO_Utils;

/**
 *
 * @author jonSchmid
 */
public class CommonDataSetTest 
{
    
    CommonDataSet instance = new CommonDataSet("TestImgs", 
            "CVAC Test Images", 
            "http://www.movesinstitute.org/~kolsch/CVAC/testImags.tar.gz",
            true);
    File[] filesInOutputDir;
    
    // Caltech url used as test data set
    private String Caltech101_URL =  
            "http://www.vision.caltech.edu/Image_Datasets/Caltech101/101_ObjectCategories.tar.gz";
    private String Caltech256_URL =                 
            "http://www.vision.caltech.edu/Image_Datasets/Caltech256/256_ObjectCategories.tar";
    private String web_decompress_unTar_OutputDir;
    
    String userDir = System.getProperty("user.home");
    String tmpTargetDir = System.getProperty("java.io.tmpdir", userDir);  // Default to user dir if no 'tmp'

    
    public CommonDataSetTest() {}
    
    @Test
    public void testExpandCommonDataset_toLocal() {
        
        System.out.println("|**********************************************************|");
        System.out.println("|Test |           Test_ExpandCommonDataset_toLocal \n" );
         
        File gzipInputFile = new File(tmpTargetDir + "/gzippedArchiveData.tar.gz");
        File test_OutputFileDir = new File(tmpTargetDir + "/UnitTestData/output/Test_Uncompression");
        String webURL = instance.homepageURL;
        String decompressedOutputName = "";
        String expandBelow_Path = "";
         
        // Warnings are displayed internally
        instance.expandDataset_toLocal(webURL, gzipInputFile, decompressedOutputName, expandBelow_Path, CommonDataSet.CompressionType.GZIP);
    }

    @Test
    public void testUnpackArchiveBelow() {

        File gzipInputFile = new File(tmpTargetDir + "/gzippedArchiveData.tar.gz");
        File targetDirFile = new File(tmpTargetDir);
        
        instance.unpackArchiveBelow(gzipInputFile, targetDirFile, CommonDataSet.ArchiveType.TAR);
    }
    
    @Test    
    public void testUncompressDataset() {
        
        System.out.println("|**********************************************************|");
        System.out.println("|Test |           Test_UncompressDataset:  [ Un-Gzipping. ] \n" );

        Class tgtClass = CommonDataSet.class;
        File gzipInputFile = new File(tmpTargetDir + "/gzippedArchiveData.tar.gz");
        File test_OutputFileDir = new File(tmpTargetDir + "/UnitTestData/output/Test_Uncompression");

        
        try {
            Data_IO_Utils.clearDir_ifNonempty(test_OutputFileDir);  // Clear output dir to begin
        }
        catch(IOException e) {
            System.out.println("|Test 1| Could not clear directory at: " + tmpTargetDir + "/UnitTestData/output/Test_Uncompression");
            assert(false);
        }

        String outputFileName = "extractedTgt.tar";  // Will be created by uncompression-output
        File test_OutputFile = new File(test_OutputFileDir.getPath() + File.separatorChar + outputFileName);

        try {
            instance.uncompressDataset(gzipInputFile, test_OutputFile, tmpTargetDir, CommonDataSet.CompressionType.GZIP);
        }
        catch(IOException e) {
            System.out.println("|Test | IOException from uncompressDataset.  " + e.getMessage());
            System.out.println("      | Input File: " + gzipInputFile);
            System.out.println("      | OutputFile: " + test_OutputFile);
            System.out.println("      | OutputDir: "  + tmpTargetDir);
            System.out.println("      | Compression Type: " + tmpTargetDir);
            assert(false);
        }
    }
    
    /** temporary main method so we can debug this
     */
    public static void main( String[] args )
    {
        System.out.println("hello22223!!");
        CommonDataSetTest test = new CommonDataSetTest();
        test.testExpandCommonDataset_toLocal();
    }
}
