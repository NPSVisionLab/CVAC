from __future__ import print_function
# test the Python Tutorials
# paths sets up the PYTHONPATH so this is not needed to be setup by the user
# to run just this test, use "ctest -R PythonFileServerTest --verbose"
import sys
import unittest
import os
import easy
import cvac

class EasyTest(unittest.TestCase):

    ic = None
    fs = None
    dataDir = None
    
    #
    # Test the initialization of Ice and the service proxy
    #
    def setUp(self):

        # Since this is a test, it's probably run in the build directory. We
        # need to know the path to the original files for various operations, but we
        # don't have easy access to the CVAC.DataDir variable.  Let's guess.
        # Lets get the directory of the script and work upwards to 
        # get the data directory
        filepath = os.path.dirname(os.path.abspath(__file__))
        self.dataDir = filepath + "/../../../data"
        if not os.path.exists( self.dataDir ):
            print("Present working directory: " + os.getcwd())
            print("Looking for CVAC.DataDir at: " + self.dataDir)
            raise RuntimeError("Cannot obtain path to CVAC.DataDir, see comments")
        #set environment variable so easy knows where the data is
        os.environ["CVAC_DATADIR"] = self.dataDir
       

   
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

    def tearDown(self):
        # Clean up
        pass

    def isProperDetector( self, configString, detectorData=None, detectorProperties=None ):
        '''Try to create or use the specified detector.
        Create a test runset and call the detector with it.
        Check for proper handling of intentional problems.
        '''
        detector = getDetector( configString )
        if not detector:
            print("cannot find or connect to detector")
            return False
        rs = easy.createRunSet( "testImg/italia.jpg" )
        doesnotexist = easy.getLabelable( "testImg/doesnotexist.jpg" )
        easy.addToRunSet( rs, doesnotexist )
        results = easy.detect( detector, detectorData, rs, detectorProperties )
        # check for number of results: must be the same as runset length
        if len(results)!=len(rs):
            print("incorrect result set size")
            return False
        # check that the doesnotexist file caused hasLabel==False
        found=False
        for lbl in results:
            if lbl.sub==doesnotexist.sub:
                if lbl.lab.hasLabel:
                    print("Incorrectly assigned label for a non-existant file.")
                    return False
                else:
                    found=True
                    break
            else:
                # confidence 0..1
                if not 0.0<=lbl.confidence and lbl.confidence<=1.0:
                    print("Label confidence out of bounds ({0}).".format(lbl.confidence))
                    return False
                # check that either this was not assigned a label, or the
                # assigned label is of proper syntax
                if lbl.lab.hasLabel:
                    if not isinstance(lbl.lab.name, str):
                        print("Label name must be of string type.")
                        return False
                    
        if not found:
            print("Every runset labelable must produce a return result.")
            return False

        return True


if __name__ == '__main__':
    unittest.main()
