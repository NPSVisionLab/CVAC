Installing EasyCV Autopsy Plugin

1. Create a directory in the Autopsy python_modules plugin directory called "easyCV"
2. copy the CVACFileIngestModule.py and DetectorCallbackHandlerI.java into that directory.
3. Build CVAC with the BUILD_FILESERVER option selected.  This file server is a JAVA file server and will cause
   CVAC to build the required java slice files required for the plugin.  This build should create a src/cvac/cvac
   directory under your build directory.  Copy this cvac directory to your plugin directory. Even though the plugin
   is written in python since its using jypthon it cannot communicate with the Ice python files and must use the
   Ice java files instead.  So you also need to copy over the files from the Ice.jar file to the plugin directory.
   Jypthon does not support calling jar files and must instead call the class files you must extract the Ice.jar
   file in the plugin directory.  When you are done extracting the Ice files you should have the following
   ice directories (Ice, IceInternal, IceMX, IceSSL, IceUtil, IceUtilInternal).  You shuld also have a cvac
   directory.  After all those files are in place compile them with the java compiler.
4. Copy the CVAC root directory config.client file to the plugin directory.  This file defines the Ice proxies
   required to communicate with the EasyCV Services as well as what model files to use with each detector.  The
   plugin will read this file and configure the detector options based upon its contents.  Each detector option
   requires a pair of entries in the config.client file.  A DETECTORNAME.proxy entry and a
   DETECTORNAME.DetectorFilename entry.  The Proxy defines how communicate with the service and the DetectorFileName
   indicates the model file to use with the detection.  For each pair defined, the DETECTORNAME option will
   appear in the option dialog for EasyCV.  Here the user can select which detectors to use for the ingest.