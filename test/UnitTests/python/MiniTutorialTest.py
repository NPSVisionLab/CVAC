from __future__ import print_function
from past.builtins import execfile
# test the Python Tutorials
# paths sets up the PYTHONPATH so this is not needed to be setup by the user
# to run just this test, use "ctest -R PythonFileServerTest --verbose"
import sys
import unittest
import os

class MiniTutorialTest(unittest.TestCase):

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
        self.demoDir = filepath + "/../../../demo"
        if not os.path.exists( self.demoDir ):
            print("Present working directory: " + os.getcwd())
            print("Looking for CVAC.DemoDir at: " + self.demoDir)
            raise RuntimeError("Cannot obtain path to CVAC.DemoDir, see comments")

    #
    # Run prerequisites.py
    #
    def test_1prerequisites(self):
        #debug
        import cvac
        print ("Importing cvac from " + cvac.__file__)
        print("running prerequisites.py")
        sys.stdout.flush()
        execfile(self.demoDir + "/prerequisites.py");

    #
    #
    # Run demo.py
    #
    def test_detect(self):
        print("running detect.py") 
        sys.stdout.flush()
        execfile(self.demoDir + "/detect.py")

    #
    # Run training.py
    #
    def test_training(self):
        print("running training.py")
        sys.stdout.flush()
        execfile(self.demoDir + "/training.py")

    #
    # Run runset.py
    #
    def test_runset(self):
        print("running runset.py")
        sys.stdout.flush()
        execfile(self.demoDir + "/runset.py");

    #
    # Run full_image_corpus.py
    #
    def xtest_full_image_corpus(self):
        print("running full_image_corpus.py")
        sys.stdout.flush()
        execfile(self.demoDir + "/full_image_corpus.py");

    #
    # Run bootstrapping.py
    #
    def test_bootstrapping(self):
        print("running bootstrapping.py")
        sys.stdout.flush()
        execfile(self.demoDir + "/bootstrapping.py");

    #
    # Run evaluation.py
    #
    # Can't run evaluation as it has svn code in it.
    def xtest_evaluation(self):
        print("running evaluation.py")
        sys.stdout.flush()
        execfile(self.demoDir + "/evaluation.py");


    def tearDown(self):
        # Clean up
        if self.ic:
            try:
                self.ic.destroy()
            except:
                traceback.print_exc()
                status = 1

if __name__ == '__main__':
    unittest.main()
