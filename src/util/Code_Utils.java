package util;

import java.io.File;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.imageio.ImageIO;
import javax.swing.ImageIcon;
import cvac.Corpus;


/**
 *
 * @author jcs
 */
public class Code_Utils {
    
    public enum OS_Type { WIN, LINUX, MAC, UNKNOWN }
    private static final Logger logger = Logger.getLogger(Corpus.class.getName());
    private static ImageIcon mirrorIcon = null;
    private static ImageIcon detectorIcon = null;
    
    
    
    public static void dieRuntime_IfNull(Object testObj, String msg) {  // Explain need to set directory

        if(null == testObj) {
            throw new RuntimeException(msg);
        }        
    }
    
    // Return as boolean whether the test string input contains one or more space: (" ") characters
    public static boolean stringHasSpaces(String testStr) {
        
        Pattern spaceChar = Pattern.compile("\\s");
        Matcher matcher = spaceChar.matcher(testStr);
        
        return(matcher.find());
    }
    
    // Convert input path string to File and return absolute-path as String
    public static String getAbsolutePath_Str(String pathStr) {
        
        File fullPath = new File(pathStr);
        String fullAbs_path = fullPath.getAbsolutePath();
        
        return(Data_IO_Utils.osPathStr(fullAbs_path));  // Customize file separator to the OS
    }
    
    public static OS_Type getOsName() {
      
      String os = "";
      if (System.getProperty("os.name").toLowerCase().indexOf("windows") > -1) {
        return(OS_Type.WIN);
      } else if (System.getProperty("os.name").toLowerCase().indexOf("linux") > -1) {
        return(OS_Type.LINUX);
      } else if (System.getProperty("os.name").toLowerCase().indexOf("mac") > -1) {
        return(OS_Type.MAC);
      }
      
      return OS_Type.UNKNOWN;
    }
    
    public static String getArchName(){
        String arch = System.getProperty("os.arch");
        String dmodel = System.getProperty("os.arch.data.model");
        return arch + "_" + dmodel;
        
    }
    
    // Feed results of 'getOsName()' to get corresponding Directory
    public static String OS_getTypeString(OS_Type type) {
        throw new RuntimeException("please use Main.engine.getProperty(\"ma.bin.dir\") instead");
    }
    
}   