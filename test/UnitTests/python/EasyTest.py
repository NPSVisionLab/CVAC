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
                
        if note easy.isProperRunSet(runset):
            raise RuntimeError("test getLabelableList failed with an invalid runset with non-recursive call")

    def tearDown(self):
        # Clean up
        pass

if __name__ == '__main__':
    unittest.main()
