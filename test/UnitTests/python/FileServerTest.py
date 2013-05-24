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
            raise RuntimeError("Cannot obtain FS path to original file,",
                               "see comments. file: "+origFS)
        forig = open( origFS, 'rb' )
        bytes = bytearray( forig.read() )
        
        # "put" the file's bytes as a different file which must not exist,
        # and compare the result via file system access
        putFilePath = cvac.FilePath( testDir, "TargetFilename.jpg" )
        try:
            self.fs.putFile( putFilePath, bytes );
        except cvac.FileServiceException as ex:
            print( "if you do not have 'put' permissions, " +
                   "was the file deleted properly in a prior test run?\nfile: " +
                   putFilePath.filename)
            raise ex
        forig.close()
        putFS = self.dataDir+"/"+self.getFSPath( putFilePath )
        if not os.path.exists( putFS ):
            raise RuntimeError("Cannot obtain path to the just-'put' file",
                               "on the file system, see comments. file: "+putFS)
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

    #
    # on a remote FileServer, first put, then get, then delete a file
    #
    def test_remotePutGetDelete(self):
        print 'putFile remote'
        configStr = "FileService:default -h vision.nps.edu -p 10110"
        baser = self.ic.stringToProxy( configStr )
        if not baser:
            raise RuntimeError("Unknown service?", configStr)
        fsr = cvac.FileServicePrx.checkedCast(baser)
        if not fsr:
            raise RuntimeError("Invalid proxy:", configStr)

        # read the bytes from TestUsFlag.jpg
        testDir = cvac.DirectoryPath( "testImg" );
        origFilePath = cvac.FilePath( testDir, "TestUsFlag.jpg" )
        origFS = self.dataDir+"/"+self.getFSPath( origFilePath )
        if not os.path.exists( origFS ):
            raise RuntimeError("Cannot obtain FS path to original file,",
                               "see comments. file: "+origFS)
        forig = open( origFS, 'rb' )
        bytes = bytearray( forig.read() )
        
        # "put" the file's bytes as a different file which must not exist
        putFilePath = cvac.FilePath( testDir, "TargetFilename.jpg" )
        try:
            fsr.putFile( putFilePath, bytes );
        except cvac.FileServiceException as ex:
            print( "if you do not have 'put' permissions, " +
                   "was the file deleted properly in a prior test run?\nfile: " +
                   putFilePath.filename)
            raise ex
        forig.close()

        # "get" the bytes again from the same name that we used for putting,
        # store them in a file on the local file system with the same name,
        # and compare the obtained file to the original one
        print 'getFile remote'
        getFS = self.dataDir+"/"+self.getFSPath( putFilePath )
        if os.path.exists( getFS ):
            raise RuntimeError("Local file exists already - cannot continue. file: "+getFS)
#            raise RuntimeError("Cannot obtain path to store file on file system,",
#                               "see comments. file: "+getFS)
        fgetFS = open( getFS, 'wb' )
        recvbytes = fsr.getFile( putFilePath )
        fgetFS.write( recvbytes )
        fgetFS.close()

        # now compare the results and delete the local file
        if not self.filesAreEqual( origFS, getFS ):
            raise RuntimeError("File was not 'put' or subsequently 'get' correctly to.",
                               "Please check, then remove manually: "+getFS)
        os.remove( getFS )

        # delete the file on the remote side and check that it's gone
        print 'deleteFile remote'
        fsr.deleteFile( putFilePath )
        try:
            dummy = fsr.getFile( putFilePath )
            raise RuntimeError("File was not deleted on remote FileServer:", putFilePath)
        except:
            # we expect the except
            pass
        
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
        # forig = open( orig, 'rb' )
        # diff = difflib.ndiff(forig.readlines(), ftmp.readlines())
        # delta = ''.join(x[2:] for x in diff if x.startswith('- '))
        # forig.close()
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
