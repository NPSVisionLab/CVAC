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

class FileServerTest(unittest.TestCase):

    ic = None
    fs = None
    
    #
    # Test the initialization of Ice and the service proxy
    #
    def setUp(self):
        self.ic = Ice.initialize(sys.argv)
        base = self.ic.stringToProxy("FileServer:default -p 10013")
        self.fs = cvac.FileServicePrx.checkedCast(base)
        if not self.fs:
            raise RuntimeError("Invalid proxy")

    #
    # Test if we can get a file
    #
    def test_getFile(self):
        print 'getFile'
        dataRoot = cvac.DirectoryPath( "testImg" );
        filePath = cvac.FilePath( dataRoot, "testUsFlag.jpg" )
        bytes = self.fs.getFile( filePath )
        if not bytes:
            raise RuntimeError("could not obtain file from '"
                               +dataRoot.relativePath+"/"+filePath.filename+"'")


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
