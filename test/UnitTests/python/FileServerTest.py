# test the FileServer
# before calling "python FileServerTest.py", make sure this is set:
# export PYTHONPATH="/opt/Ice-3.4.2/python:test/UnitTests/python"
import sys, traceback
#sys.path.append('''c:\Program Files (x86)\Zeroc\Ice-3.4.2\python''')
#sys.path.append('''C:\Users\tomb\Documents\nps\git\myCVAC\CVACvisualStudio\test\UnitTests\python''')
#sys.path.append('''C:\Users\tomb\Documents\nps\git\myCVAC\CVACvisualStudio\test\UnitTests\python\cvac''')
sys.path.append('''.''')
import Ice
if "C:\Program Files (x86)\ZeroC_Ice\python" not in sys.path:
    sys.path.append("C:\Program Files (x86)\ZeroC_Ice\python")
import Ice
import IcePy
import cvac
import unittest
import os
import inspect
import tempfile
import filecmp

class FileServerTest(unittest.TestCase):

    ic = None
    fs = None
    
    #
    # Test the initialization of Ice and the service proxy
    #
    def setUp(self):
        if not 'FileServicePrx' in dir( cvac ):
            # import os
            # print os.environ['PYTHONPATH'].split(os.pathsep)
            raise RuntimeError("cvac module not loaded correctly (from file "+cvac.__file__+")")
        if not inspect.ismodule( cvac ) or not inspect.isclass( cvac.FileServicePrx ):
            raise RuntimeError("cvac module not loaded")
        self.ic = Ice.initialize(sys.argv)
        base = self.ic.stringToProxy("FileServer:default -p 10013")
        self.fs = cvac.FileServicePrx.checkedCast(base)
        if not self.fs:
            raise RuntimeError("Invalid proxy")

    #
    # Test if we can get a file, copy bytes to a tempfile
    #
    def test_getFile(self):
        print 'getFile'
        # test with a small file and a larger file
        dataRoot = cvac.DirectoryPath( "testImg" );
        filePath = cvac.FilePath( dataRoot, "TestUsFlag.jpg" )
        self.getFileAndCompare( filePath )

    def getFileAndCompare( self, filePath ):
        bytes = self.fs.getFile( filePath )
        if not bytes:
            raise RuntimeError("could not obtain file from '"
                               +filePath.directory.relativePath+"/"+filePath.filename+"'")

        # Write bytes to a temp file and compare the contents to the orig.
        # Since this is a test, it's probably run in the build directory. We
        # need to know the path to the original file for the compare, but we
        # don't have easy access to the CVAC.DataDir variable.  Let's try to
        # figure it out from the 'pwd'
        orig = "../../../../data"+"/"+filePath.directory.relativePath+"/"+filePath.filename
        if not os.path.exists( orig ):
            print "Present working directory: " + os.getcwd()
            print "Looking for file: " + orig
            raise RuntimeError("Cannot obtain path to original file, see comments")
        
        ftmp = tempfile.NamedTemporaryFile( suffix='.jpg', delete=True )
        # ftmp.write( bytes )
        ftmp.flush()
        if not self.filesAreEqual( orig, ftmp.name ):
            print "comparison failed, new file: " + ftmp.name
            ftmp.close()
            raise RuntimeError("file was not copied correctly")
        ftmp.close()

    def filesAreEqual(self, fname1, fname2):
        # import difflib
        # print difflib.SequenceMatcher(None, orig, tfile.name)
        # forig = open( orig )
        # diff = difflib.ndiff(forig.readlines(), ftmp.readlines())
        # delta = ''.join(x[2:] for x in diff if x.startswith('- '))
        return filecmp.cmp( fname1, fname2 )

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
