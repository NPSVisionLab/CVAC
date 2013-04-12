/*
 * To change this template, choose Tools | Templates
 * and open the template getData the editor.
 */

package util;

import java.util.*;
import java.net.*;
import java.io.*;
import java.util.logging.Level;
import java.util.logging.Logger;
import cvac.Corpus;

import util.Data_IO_Utils;

/**
 * Code to download given URL and save it as a local file
 * @author tomb, jcs
 */
public class DownloadUtils {
    
    private static final Logger logger = Logger.getLogger(Corpus.class.getName());
    private static byte[] byteData;
    static final int END_OF_STREAM = -1;
    static final int SVR_DATA_BUFF_SZ = 2048;

    /**
     * Read from the server at the urlString and return the response
     */
    public static void URL_readToFile(String urlString, File outputTarget) throws IOException {

        logger.log(Level.FINE, "trying to download {0} to {1}",
                new Object[]{urlString, outputTarget.getAbsolutePath()});
        // replace spaces with %20.  Could also use something like this:
        // URLEncoder.encode(urlString, "UTF-8") );
        // but the problem is that this also replaces the :// with UTF characters
        String nospaces = urlString.replace(" ", "%20");
        URL url = new URL( nospaces );
        URLConnection con = url.openConnection();
        BufferedInputStream serverData = new BufferedInputStream(con.getInputStream());

        if(outputTarget.isDirectory()) {
            String msg = ("URL_readToFile method cannot continue.  \n" +
                          "The output File target must be a file, and appears to be a directory.");
            logger.severe(msg);
            throw new IOException(msg);
        }
        Data_IO_Utils.createDir_orFile(outputTarget);

        String outputFileName = (outputTarget.getPath());
        FileOutputStream dataOutStr = new FileOutputStream(outputFileName);
        int byteCount;
        byteData = new byte[SVR_DATA_BUFF_SZ];

        try {
            while(END_OF_STREAM != (byteCount = serverData.read(byteData))) {
                dataOutStr.write(byteData, 0, byteCount);
            }
        }
        catch (Exception e) {
            logger.warning(e.toString());
            throw new IOException(e);
        }

        dataOutStr.flush();
        dataOutStr.close();
    }
}
