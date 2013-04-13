/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package cvac.corpus;

import java.io.File;
import java.io.IOException;
import java.util.Properties;
import org.junit.Test;
import util.Data_IO_Utils;

/**
 *
 * @author jonSchmid
 * Port of origin-Class test: 'DataSetTest.java' from MediaAnalyst tests
 */
public class CommonDataSetTestAccess extends CommonDataSet {

    public CommonDataSetTestAccess(String name, String description, String homepageURL, boolean isImmutableMirror) {
         super(name, description, homepageURL, isImmutableMirror);
    }

    public void accessExpandCommonDataset_toLocal(String webURL, File web_SavedFile, String decompressedOutputName, String expandBelow_Path, CompressionType uncompressType) {
         
        expandDataset_toLocal(webURL, web_SavedFile, decompressedOutputName, expandBelow_Path, uncompressType);
    }

    public void accessUnpackArchiveBelow(File archiveFile, File dsTargetFolder, ArchiveType archiveType) {
        
        unpackArchiveBelow(archiveFile, dsTargetFolder, archiveType);
    }
    
    public void accessUncompressDataset(File compressedFileSrc, File destFile, String destFileDir, CompressionType in_type) throws IOException {
        
        uncompressDataset(compressedFileSrc, destFile, destFileDir, in_type);
    }    
}
