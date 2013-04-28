package util;

import java.io.File;
import java.io.IOException;
//import java.util.ArrayList;
//import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.regex.Pattern;
import cvac.Corpus;
//import Code_Utils.OS_Type;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.FilenameUtils;


/**
 *
 * @author jcs
 */
public class Data_IO_Utils {

//    TODO: check what's used;

//    private static final String userDir_Token = "|USER_HOME_DIR|";
//    private static final String dataDir_Token = "|ENGINE_DATA_DIR|";
    private static String os_sepChar = System.getProperty("file.separator");
    private static String usersHomeDir = osPathStr(System.getProperty("user.home"));

    public enum FS_ObjectType {

        DIRECTORY, FILE
    };

    private static final Logger logger = Logger.getLogger(Corpus.class.getName());

    public static String unixPathStr(String anyPath) {

        // Change front and back slashes to the OS Path separator-char direction
        String unixPath = anyPath.replace("\\", "/");
        return (unixPath);
    }

    public static String osPathStr(String anyPath) {

        // Change front and back slashes to the OS Path separator-char direction
        String curSysPath = anyPath.replace("/", os_sepChar);
        curSysPath = curSysPath.replace("\\", os_sepChar);

        return (curSysPath);
    }

    // Substitute working path with '|USER_HOME_DIR|'
    // $ is a special character in XML strings, avoid.
//    public static String generalize_toUserDir_Prefix(String original) {
//
//        if (null == original) {
//            return null;
//        }
//
//        // Unify all slashes by filtering to the current OS separator character
//        String original_path = osPathStr(original),
//                dataDir_path = osPathStr(Main.getEngine().getDataDir().getAbsolutePath());
//
//
//        int userDir_result = (original_path).indexOf(usersHomeDir);
//        if (userDir_result > 0) {
//            logger.warning("Cannot insert '|USER_HOME_DIR|' in XML tag.  The user's home directory substring "
//                    + "MUST be a prefix (and begin at the left-most character in the original String being replaced).");
//            return (original_path);
//        }
//
//        int dataDir_result = (original_path).indexOf(dataDir_path);
//        if (dataDir_result > 0) {
//            logger.warning("Cannot insert '|ENGINE_DATA_DIR|' in XML tag.  The engine's data directory substring "
//                    + "MUST be a prefix (and begin at the left-most character in the original String being replaced).");
//            return (original_path);
//        }
//
//        // User Home Directory detected, substituting token: |USER_HOME_DIR|
//        if (0 == original_path.indexOf(usersHomeDir)) {
//            int firstValidIdx = (usersHomeDir.length());
//            int lastValidIdx = original_path.length();
//
//            String validPart = (original_path).substring(firstValidIdx, lastValidIdx);
//            return ("|USER_HOME_DIR|" + validPart);
//        }
//
//        // Engine Data Directory detected, substituting token: |ENGINE_DATA_DIR|
//        if (0 == (original_path).indexOf(dataDir_path)) {
//            int firstValidIdx = (dataDir_path.length());
//            int lastValidIdx = original_path.length();
//
//            String validPart = (original_path).substring(firstValidIdx, lastValidIdx);
//            return ("|ENGINE_DATA_DIR|" + validPart);
//        }
//
//        return (original_path); // No substitutions
//    }

    // win: ~/Caches/CVAC/
    // x86/64/mac: ~/Library/Caches/CVAC/
    private static String getUserCacheDir() {

        if ((Code_Utils.OS_Type.WIN == Code_Utils.getOsName()) || (Code_Utils.OS_Type.LINUX == Code_Utils.getOsName())) {
            return (osPathStr(usersHomeDir + File.separatorChar + "Caches" + File.separatorChar + "CVAC" + File.separatorChar));
        } else if (Code_Utils.OS_Type.MAC == Code_Utils.getOsName()) {
            return (osPathStr(usersHomeDir + File.separatorChar + "Library" + File.separatorChar + "Caches" + File.separatorChar + 
                              "CVAC" + File.separatorChar));
        } else {
            throw new RuntimeException("OS Type is not supported: {win, mac, linux}.  Cannot set path for user-space cache.");
        }
    }

    // Substitute '|USER_HOME_DIR|' token with Java user directory
    // $ is a special character in XML strings, avoid.
//    public static String expand_userDir_Prefix(String original) {
//
//        if (null == original) {
//            return null;
//        }
//
//        String original_path = osPathStr(original),
//                dataDir_path = osPathStr(Main.getEngine().getDataDir().getAbsolutePath());
//        usersHomeDir = osPathStr(usersHomeDir);
//
//
//        int userDir_result = original.indexOf(userDir_Token);
//        if (userDir_result > 0) {
//            logger.warning("Cannot replace |USER_HOME_DIR| in XML tag with the User's Home Directory.  The tag"
//                    + "MUST begin at the left-most character in the original String being replaced.");
//            return (original);
//        }
//        int dataDir_result = original.indexOf(dataDir_Token);
//        if (dataDir_result > 0) {
//            logger.warning("Cannot replace |ENGINE_DATA_DIR| in XML tag with the User's Home Directory.  The tag"
//                    + "MUST begin at the left-most character in the original String being replaced.");
//            return (original);
//        }
//
//        if (0 == userDir_result) {  // Pull out User's Directory token
//
//            int firstValidIdx = userDir_Token.length();
//            int lastValidIdx = original.length();
//            String validPart_atRight = original.substring(firstValidIdx, lastValidIdx);
//
//            return (usersHomeDir + validPart_atRight);
//        } else if (0 == dataDir_result) {  // Pull out Data-directory token
//
//            int firstValidIdx = userDir_Token.length();
//            int lastValidIdx = original.length();
//            String validPart_atRight = original_path.substring(firstValidIdx, lastValidIdx);
//
//            return (dataDir_path + validPart_atRight);
//        } else {
//            return (original_path);  // No substitution necessary
//        }
//    }

    // Note, this should not be used in any method that might fail for a User!
    public static void dieRuntime_IfNull(Object testObj, String msg) {

        if (null == testObj) {
            throw new RuntimeException(msg);
        }
    }

    public static void createPgm_forImageFile(String fnameWithPath) {
    }

    // Stub for making new File or Dir using an existing File Obj
    public static void createDir_orFile(File objToCreate) throws IOException {

        if (objToCreate.isDirectory()) {

            createDir_orFile(objToCreate, FS_ObjectType.DIRECTORY);
        } else {
            createDir_orFile(objToCreate, FS_ObjectType.FILE);
        }
    }

    /** Check type of incoming file and convert if necessary
     *   Save (.pgm) file in temporary directory-tree for s_o_c detector use
     */
//    public static String prepare_pgm_imageFile(String fnameWithPath) {
//
//        int dotIndex = fnameWithPath.indexOf(".");
//        String imgExt = fnameWithPath.substring((1 + dotIndex));
//
//        if ("PNG".equalsIgnoreCase(imgExt)) {
//            logger.warning("Cannot prepare (PGM) file.  The file type"
//                    + "Portable Network Graphic (PNG) is not currently"
//                    + "supported as a source format.");
//        }
//
//        if ("PGM".equalsIgnoreCase(imgExt)) {
//            return osPathStr(fnameWithPath);  // No path changes
//        }
//
//        // Launch conversion tool to produce (pgm) in Java Temp Dir
//        // Return: path of converted (pgm)
//        if ("JPG".equalsIgnoreCase(imgExt) || "JPEG".equalsIgnoreCase(imgExt)) {
//
//            String srcPath_noExtension = fnameWithPath.substring(0, dotIndex);
//
//            // Ensure Cache Path folder exists
//            File userCachePath_folder = new File(getUserCacheDir());
//            if (!userCachePath_folder.exists()) {
//                userCachePath_folder.mkdir();
//            }
//
//            // Build the full cache-dir-path for the (.pgm) output
//            String srcPath_pgmExt = osPathStr(srcPath_noExtension + ".pgm");
//            File tmpPgmFile = new File(srcPath_pgmExt);
//
//            // User's cache directory with pgm file extension
//            String pgmFnamePath = (getUserCacheDir() + tmpPgmFile.getName());
//
//            // Execute conversion, return full path PGM-output
//            Data_IO_Utils.batchfile_PGM_Conversion(fnameWithPath, pgmFnamePath);
//            return pgmFnamePath;
//        }
//
//        // Incorrect input image format
//        return null;
//    }

    // Special case for forcing use of knowledge about Directory vs. File
    // from outside of 'objToCreate' File obj
    public static void createDir_orFile(File objToCreate, FS_ObjectType objType) throws IOException {

        if (FS_ObjectType.FILE == objType) {

            if (!objToCreate.exists()) {  // Only create if not existing

                // Error if problem creating non-existing File
                boolean successCreating = objToCreate.createNewFile();
                if (!successCreating) {
                    String msg = "Uncompression was unable to create new file for output at: \n'"
                            + objToCreate.getPath() + "'";
                    throw new IOException(msg);
                }
            }
        } else { // Obj must be 'FS_ObjectType.DIRECTORY'

            if (!objToCreate.exists()) {  // Only create if not existing            

                // Error if problem creating non-existing directory
                boolean madeNewDir_ok = (objToCreate.mkdirs());
                if (!madeNewDir_ok) {
                    String msg = ("Could not create directory structure in 'unpackArchive' for path: \n" + objToCreate.getPath());
                    throw new IOException(msg);
                }
            }
        }
    }

    public static void clearDir_ifNonempty(File rootDirToClear) throws IOException {

        String[] rootDirContents = rootDirToClear.list();
        if ((rootDirToClear.isDirectory()) && (rootDirToClear.exists()) && (rootDirContents.length > 0)) {  // Does nothing otherwise

            if (rootDirContents != null) {

                for (int i = 0; i < rootDirContents.length; i++) {

                    File targetFile = new File(rootDirToClear, rootDirContents[i]);
                    FileUtils.forceDelete(targetFile);
                }
            }

            // Confirm all deleted or warn failure.
            int numFilesLeft = (rootDirToClear.listFiles().length);
            if (0 != numFilesLeft) {

                logger.warning("Error clearing files.  The following could not be cleared: \n");
                for (int i = 0; i < numFilesLeft; i++) {

                    File targetFile = new File(rootDirToClear, rootDirContents[i]);
                    logger.warning(rootDirToClear.getPath());
                }
            }
        }
    }

    /**
     * Get the relative path from one file to another, specifying the directory separator. 
     * If one of the provided resources does not exist, it is assumed to be a file unless it ends with '/' or
     * '\'.
     * 
     * @param target targetPath is calculated to this file
     * @param base basePath is calculated from this file
     * @param separator directory separator. The platform default is not assumed so that we can test Unix behavior when running on Windows (for example)
     * Code source: http://stackoverflow.com/questions/204784/how-to-construct-a-relative-path-in-java-from-two-absolute-paths-or-urls/3054692#3054692
     * Note the Unit-tests demonstrating correctness of features:  "/test/CVAC/utility_code/Data_IO_UtilsTest.java"
     */
    public static String getRelativePath(String targetPath, String basePath, String pathSeparator) {

        // Normalize the paths
        String normalizedTargetPath = FilenameUtils.normalizeNoEndSeparator(targetPath);
        String normalizedBasePath = FilenameUtils.normalizeNoEndSeparator(basePath);

        // Undo the changes to the separators made by normalization
        if (pathSeparator.equals("/")) {
            normalizedTargetPath = FilenameUtils.separatorsToUnix(normalizedTargetPath);
            normalizedBasePath = FilenameUtils.separatorsToUnix(normalizedBasePath);

        } else if (pathSeparator.equals("\\")) {
            normalizedTargetPath = FilenameUtils.separatorsToWindows(normalizedTargetPath);
            normalizedBasePath = FilenameUtils.separatorsToWindows(normalizedBasePath);

        } else {
            throw new IllegalArgumentException("Unrecognised dir separator '" + pathSeparator + "'");
        }

        String[] base = normalizedBasePath.split(Pattern.quote(pathSeparator));
        String[] target = normalizedTargetPath.split(Pattern.quote(pathSeparator));

        // First get all the common elements. Store them as a string,
        // and also count how many of them there are.
        StringBuilder common = new StringBuilder();

        int commonIndex = 0;
        while (commonIndex < target.length && commonIndex < base.length
                && target[commonIndex].equals(base[commonIndex])) {
            String tgtAndPath = (target[commonIndex] + pathSeparator);
            common.append(tgtAndPath);
            commonIndex++;
        }

        if (commonIndex == 0) {
            // No single common path element. This most
            // likely indicates differing drive letters, like C: and D:.
            // These paths cannot be relativized.
            throw new RuntimeException("No common path element found for '" + normalizedTargetPath + "' and '" + normalizedBasePath
                    + "'");
        }

        // The number of directories we have to backtrack depends on whether the base is a file or a dir
        // For example, the relative path from
        //
        // /foo/bar/baz/gg/ff to /foo/bar/baz
        // 
        // ".." if ff is a file
        // "../.." if ff is a directory
        //
        // The following is a heuristic to figure out if the base refers to a file or dir. It's not perfect, because
        // the resource referred to by this path may not actually exist, but it's the best I can do
        boolean baseIsFile = true;

        File baseResource = new File(normalizedBasePath);

        if (baseResource.exists()) {
            baseIsFile = baseResource.isFile();

        } else if (basePath.endsWith(pathSeparator)) {
            baseIsFile = false;
        }

        StringBuilder relative = new StringBuilder();

        if (base.length != commonIndex) {
            int numDirsUp = baseIsFile ? base.length - commonIndex - 1 : base.length - commonIndex;

            String dirRel = null;
            for (int i = 0; i < numDirsUp; i++) {
                dirRel = ".." + pathSeparator;
                relative.append(dirRel);
            }
        }
        relative.append(normalizedTargetPath.substring(common.length()));
        return relative.toString();
    }

    /*
     * Return a filename where we can store a local copy of this remote file.  Since
     * we might want to keep it around for a while lets mirror the file structure on the remote side.
     */
    public static String getLocalFileNameFromRemoteFile(File remoteFile) {
        File localFile = new File(System.getProperty("java.io.tmpdir") + File.separator + remoteFile.getPath());
        localFile.mkdirs();
        return localFile.getPath();
    }

    /*
     * Return a filename where we can store a local copy of this remote file.  Since
     * we might want to keep it around for a while lets mirror the file structure on the remote side.
     */
    public static String getLocalFileNameFromRemote(String remoteName) {
        File localFile = new File(System.getProperty("java.io.tmpdir") + File.separator + remoteName);
        localFile.getParentFile().mkdirs();
        return localFile.getPath();
    }

//    public static void batchfile_PGM_Conversion(String inputImageFile, String convertedFileTgt) {
//
//        Echo_Stream_fromProcess clearStr_in, clearStr_err;
//        ArrayList<String> cmdStrList = new ArrayList<String>(3);
//        ProcessBuilder cmd_Builder = new ProcessBuilder(cmdStrList);
//        Process cmdTgt_Process = null;
//
//
//        // Relative to 'trunk'
//        String binPath = Main.getEngine().getProperty("ma.bin.dir");
//
//        // The 'bat' file instead runs as a 'bash' shell script when platform is x86/64/Mac
//        File runConvertProgram = new File(binPath + File.separatorChar + "launch_jpegtopnm.bat");
//
//        cmdStrList.add(0, runConvertProgram.getPath());
//        cmdStrList.add(1, inputImageFile);
//        cmdStrList.add(2, convertedFileTgt);
//
//        try {
//            //System.out.println("Test of Convert - ((jpg || jpeg) -> pgm) Launching.\n" + cmdStrList.get(0));
//            cmdTgt_Process = cmd_Builder.start();
//        } catch (Exception e) {
//            String msg = "Error starting 'Cmd' ProcessBuilder: " + e.toString();
//            logger.warning(msg);
//        }
//
//        // Consume all output and error-output from the process using separate threads 
//        clearStr_in = new Echo_Stream_fromProcess("stdin", cmdTgt_Process.getInputStream());
//        clearStr_err = new Echo_Stream_fromProcess("stderr", cmdTgt_Process.getErrorStream());
//        clearStr_in.start();
//        clearStr_err.start();
//
//        try {
//            cmdTgt_Process.waitFor();
//            System.out.println("*** Cmd-process Returned:  \n" + "*** " + inputImageFile + "\n*** Converted and piped to: \n"
//                    + "*** " + convertedFileTgt);
//        } catch (Exception e) {
//            logger.warning("Launched process has thrown: 'Thread interrupted-Exc' during 'waitFor()'");
//        }
//
//        cmdTgt_Process.destroy();
//    }
}
