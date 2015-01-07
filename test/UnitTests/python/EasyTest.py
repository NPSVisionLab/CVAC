from __future__ import print_function
# test the Python Tutorials
# paths sets up the PYTHONPATH so this is not needed to be setup by the user
# to run just this test, use "ctest -R PythonFileServerTest --verbose"
import sys
import unittest
import os

'''Since we need to setup this env variable before
   we import easy lets do it now!
'''

filepath = os.path.dirname(os.path.abspath(__file__))
dataDir = filepath + "/../../../data"
if not os.path.exists( dataDir ):
    print("Present working directory: " + os.getcwd())
    print("Looking for CVAC.DataDir at: " + dataDir)
    raise RuntimeError("Cannot obtain path to CVAC.DataDir, see comments")
#set environment variable so easy knows where the data is
os.environ["CVAC_DATADIR"] = dataDir
# wait to import easy after we setup CVAC_DATADIR
import easy
import cvac

class EasyTest(unittest.TestCase):

    ic = None
    fs = None
    dataDir = None
    
    #
    # Test the initialization 
    #
    def setUp(self):
        pass
   
    #
    # Create a runset from a directory.
    #
    def test_getLabelableList(self):
        print("testing getLabelableList")
        sys.stdout.flush()
        labelList = easy.getLabelableList("easyTestData")
        '''verify that the labels are correct
           They should be , the root directory "easyTestData",
           "kitchen" and "MITmountain"
           
        '''
        for entry in labelList:
            print ("label name " + entry.lab.name)
            if entry.lab.name != "easyTestData" and \
               entry.lab.name != "kitchen" and \
               entry.lab.name != "MITmountain":
                raise RuntimeError("Not a valid label name in test getLabelableList")
            if entry.lab.name == "kitchen":
                # should find easyTestData, inside, house properties
                if "easyTestData" not in entry.lab.properties:
                    raise RuntimeError("easyTestData not in properties")
                if "inside" not in entry.lab.properties:
                    raise RuntimeError("inside not in properties")
                if "house" not in entry.lab.properties:
                    raise RuntimeError("house not in properties")
                    
        
        ''' verify that we can make a runset and use it with the labelableList
        '''
        runset = cvac.RunSet()
        posPurpose = easy.getPurpose('pos')
        easy.addToRunSet(runset, labelList, posPurpose)
        if not easy.isProperRunSet(runset):
            raise RuntimeError("test getLabelableList failed with an invalid runset")
        labelList = easy.getLabelableList("easyTestData", recursive=False)
        ''' should only have one label "easyTestData"
        '''
        for entry in labelList:
            if entry.lab.name != "easyTestData":
                raise RuntimeError("recursive option failed in test getLabelableList")
            if len(entry.lab.properties) > 0:
                raise RuntimeError("labelable should not have any properties")
                
        if not easy.isProperRunSet(runset):
            raise RuntimeError("test getLabelableList failed with an invalid runset with non-recursive call")

    #
    def test_cvacdatadir(self):
        print("testing cvac data dir")
        if sys.platform == 'win32':
            datadir = os.getenv("CVAC_DATADIR", None) 
            datadir = datadir.replace("/", "\\")
            easy.CVAC_DataDir = datadir
            print("Testing using back slashes")
            print("Using CVAC_DATADIR as " + datadir)
            testset = []
            easy.misc.searchDir(testset, datadir + '\\testImg', recursive=True, video=False, image=True)
            runset = cvac.RunSet()
            easy.addToRunSet(runset, testset, 'pos')
            modelfile = 'detectors/bowUSKOCA.zip'
            detector = easy.getDetector("BOW_Detector")
            props = easy.getDetectorProperties(detector)
            props.verbosity = 3
            results = easy.detect(detector, modelfile, runset, 
                            detectorProperties=props)
            easy.printResults(results)
        else:
            print("Skipping back slash test on this platform")
        
        #run it again with all forward slashes
        datadir = os.getenv("CVAC_DATADIR", None) 
        datadir = datadir.replace("\\", "/")
        print("Testing using all forward slashes")
        print("Using CVAC_DATADIR as " + datadir)
        easy.CVAC_DataDir = datadir
        testset = []
        easy.misc.searchDir(testset, datadir + '/testImg', recursive=True, video=False, image=True)
        runset = cvac.RunSet()
        easy.addToRunSet(runset, testset, 'pos')
        modelfile = 'detectors/bowUSKOCA.zip'
        detector = easy.getDetector("BOW_Detector")
        props = easy.getDetectorProperties(detector)
        props.verbosity = 3
        results = easy.detect(detector, modelfile, runset,
                               detectorProperties=props)
        easy.printResults(results)

        #run it for forward slashes and relative path
        origDir = datadir
        curDir = os.getcwd()
        curDir = curDir.replace("\\", "/")
        datadir = datadir.replace("\\", "/")
        print("using relative paths for " + curDir + " in  data dir " + datadir)
        if datadir.startswith(curDir):
            datadir = datadir[len(curDir)+1:]
            easy.CVAC_DataDir = datadir
            print("Using CVAC_DataDir as "  + datadir)
            testset = []
            easy.misc.searchDir(testset, origDir + '/testImg', recursive=True, video=False, image=True)
            runset = cvac.RunSet()
            easy.addToRunSet(runset, testset, 'pos')
            modelfile = origDir + '/detectors/bowUSKOCA.zip'
            detector = easy.getDetector("BOW_Detector")
            props = easy.getDetectorProperties(detector)
            props.verbosity = 3
            results = easy.detect(detector, modelfile, runset,
                                   detectorProperties=props)
            easy.printResults(results)
        else:
            RuntimeError("Bad datadir")
            
    def test_gui(self):
        print("Testing gui")
        import Tkinter as tk
        import time
        from PIL import Image, ImageTk, ImageDraw

        datadir = os.getenv("CVAC_DATADIR", None) 
        img = Image.open( datadir +  "/testImg/TestCaFlag.jpg" )
        img2 = Image.open( datadir +  "/testImg/TestKrFlag.jpg" )
        img3 = Image.open( datadir +  "/testImg/TestUsFlag.jpg" )
        easy.guiqueue.imgWindow(img)
        time.sleep(2)
        easy.guiqueue.imgWindow(img2)
        time.sleep(2)
        easy.guiqueue.imgWindow(img3)
        w1 = easy.guiqueue.startWindow(img)
        w2 = easy.guiqueue.startWindow(img2)
        time.sleep(2)
        easy.guiqueue.imgWindow(img3, window = w1)
        easy.guiqueue.imgWindow(img3, window = w2)
        time.sleep(5)
        easy.guiqueue.closeAllWindows()
        
    def tearDown(self):
        # Clean up
        pass

    def isProperDetector( self, configString, detectorData=None, detectorProperties=None ):
        '''Try to create or use the specified detector.
        Create a test runset and call the detector with it.
        Check for proper handling of intentional problems.
        '''
        detector = easy.getDetector( configString )
        if not detector:
            print("cannot find or connect to detector")
            return False
        
        # make a runset including erroneous files
        rs = cvac.RunSet()
        file_normal = easy.getLabelable( "testImg/italia.jpg" )
        file_notexist = easy.getLabelable( "testImg/doesnotexist.jpg" )
        file_notconvertible = easy.getLabelable( "testImg/notconvertible.xxx")
        easy.addToRunSet( rs, file_normal, cvac.Purpose( cvac.PurposeType.UNPURPOSED, -1 ))
        easy.addToRunSet( rs, file_notexist, cvac.Purpose( cvac.PurposeType.UNPURPOSED, -1 ))
        easy.addToRunSet( rs, file_notconvertible, cvac.Purpose( cvac.PurposeType.UNPURPOSED, -1 ) )
        results = easy.detect( detector, detectorData, rs, detectorProperties )
        
        # check for number of results: must be the same as runset length
        nRunset = 0 
        for pur in rs.purposedLists:
            nRunset += len(pur.labeledArtifacts)
        if len(results)!=nRunset:
            print("incorrect result set size")
            return False
        
        # check that the doesnotexist file caused hasLabel==False
        found=False
        for lbl in results:
            # for an erroneous file
            if lbl.original.sub==img_notexist.sub or lbl.original.sub==img_notconvertible.sub:
                for foundlbl in lbl.foundLabels:
                    if foundlbl.lab.hasLabel:
                        print("Incorrectly assigned label for an erroneous file.")
                        return False
            else:   #for a normal file
                for foundlbl in lbl.foundLabels:
                    # confidence 0..1
                    if not 0.0<=foundlbl.confidence and foundlbl.confidence<=1.0:
                        print("Label confidence out of bounds ({0}).".format(foundlbl.confidence))
                        return False
                    # check that either this was not assigned a label, or the
                    # assigned label is of proper syntax
                    if foundlbl.lab.hasLabel:
                        if not isinstance(foundlbl.lab.name, str):
                            print("Label name must be of string type.")
                            return False
        return True


if __name__ == '__main__':
    unittest.main()
