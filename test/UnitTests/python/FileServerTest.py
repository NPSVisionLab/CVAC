# test the FileServer
# before calling "python FileServerTest.py", make sure this is set:
# export PYTHONPATH="/opt/Ice-3.4.2/python:test/UnitTests/python"
# to run just this test, use "ctest -R PythonFileServerTest --verbose"
import sys, traceback
sys.path.append('''.''')
import Ice
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
    dataDir = None
    
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
        base = self.ic.stringToProxy("FileService:default -p 10110")
        self.fs = cvac.FileServicePrx.checkedCast(base)
        if not self.fs:
            raise RuntimeError("Invalid proxy")

        # Since this is a test, it's probably run in the build directory. We
        # need to know the path to the original files for various operations, but we
        # don't have easy access to the CVAC.DataDir variable.  Let's guess.
        self.dataDir = "../../../../data"
        if not os.path.exists( self.dataDir ):
            print "Present working directory: " + os.getcwd()
            print "Looking for CVAC.DataDir at: " + self.dataDir
            raise RuntimeError("Cannot obtain path to CVAC.DataDir, see comments")

    #
    # See if exists works on an existing and non-existent file
    #
    def test_exists(self):
        testDir = cvac.DirectoryPath( "testImg" );
        existingFile = cvac.FilePath( testDir, "TestUsFlag.jpg" )
        noFile = cvac.FilePath( testDir, "NonExistentFile.jpg" )
        if not self.fs.exists( existingFile ):
            raise RuntimeError("This file should exist: "+self.getFSPath(existingFile) )
        if self.fs.exists( noFile ):
            raise RuntimeError("This file should not exist: "+self.getFSPath( noFile ) )

    #
    # Test if we can get a file, copy bytes to a tempfile, compare
    #
    def test_getFile(self):
        print 'getFile'
        # test with a small file for now
        testDir = cvac.DirectoryPath( "testImg" );
        filePath = cvac.FilePath( testDir, "TestUsFlag.jpg" )
        self.getFileAndCompare( filePath )

    #
    # Test if we can put a file, compare results;
    # Test that we can delete this file on the server again;
    # Test that overwriting an existing file fails;
    #
    def test_putFile(self):
        print 'putFile'
        # read the bytes from TestKrFlag.jpg
        testDir = cvac.DirectoryPath( "testImg" );
        origFilePath = cvac.FilePath( testDir, "TestKrFlag.jpg" )
        origFS = self.dataDir+"/"+self.getFSPath( origFilePath )
        if not os.path.exists( origFS ):
            raise RuntimeError("Cannot obtain path to original file, see comments: "+origFS)
        forig = open( origFS )
        bytes = bytearray( forig.read() )
        
        # "put" the file's bytes as a different file which must not exist,
        # and compare the result via file system access
        putFilePath = cvac.FilePath( testDir, "TargetFilename.jpg" )
        self.fs.putFile( putFilePath, bytes );
        putFS = self.dataDir+"/"+self.getFSPath( putFilePath )
        if not os.path.exists( putFS ):
            raise RuntimeError("Cannot obtain path to put file on file system, see comments: "+putFS)
        if not self.filesAreEqual( origFS, putFS ):
            raise RuntimeError("file was not 'put' correctly to "+putFS)

        # try to "put" an existing file; this should fail
        permitted = True
        try:
            self.fs.putFile( origFilePath, bytes )
        except cvac.FileServiceException:
            permitted = False
        if permitted:
            raise RuntimeException("should not have permission to put this file")

        # delete the "put" file on the server
        print 'deleteFile'
        self.fs.deleteFile( putFilePath );
        if os.path.exists( putFS ):
            raise RuntimeError("FileServer did not delete 'put' file: "+putFS)
        
    def getFileAndCompare( self, filePath ):
        bytes = self.fs.getFile( filePath )
        if not bytes:
            raise RuntimeError("could not obtain file from '"+self.getFSPath( filePath ))

        # Write bytes to a temp file and compare the contents to the orig.
        # Since this is a test, it's probably run in the build directory. We
        # need to know the path to the original file for the compare, but we
        # don't have easy access to the CVAC.DataDir variable.  Let's try to
        # figure it out from the 'pwd'
        orig = self.dataDir+"/"+self.getFSPath( filePath )
        if not os.path.exists( orig ):
            raise RuntimeError("Cannot obtain path to original file, see comments: "+orig)
        
        ftmp = tempfile.NamedTemporaryFile( suffix='.jpg', delete=True )
        ftmp.write( bytes )
        ftmp.flush()
        if not self.filesAreEqual( orig, ftmp.name ):
            print "comparison failed, new file: " + ftmp.name
            ftmp.close()
            raise RuntimeError("file was not 'get' correctly")
        ftmp.close()

    def filesAreEqual(self, fname1, fname2):
        # import difflib
        # print difflib.SequenceMatcher(None, orig, tfile.name)
        # forig = open( orig )
        # diff = difflib.ndiff(forig.readlines(), ftmp.readlines())
        # delta = ''.join(x[2:] for x in diff if x.startswith('- '))
        return filecmp.cmp( fname1, fname2 )

    def getFSPath(self, cvacPath):
        path = cvacPath.directory.relativePath+"/"+cvacPath.filename
        return path

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
