
from __future__ import print_function
# test the CorpusServer
# before calling "python CorpusServerTest.py", make sure this is set:
# export PYTHONPATH="/opt/Ice-3.4.2/python:test/UnitTests/python"
import sys, traceback
sys.path.append('''.''')
import paths
import Ice
import Ice
import IcePy
import cvac
import unittest
import subprocess
import time
import os

class TestCorpusCallback(cvac.CorpusCallback):
    def corpusMirrorProgress( corpus, 
                numtasks, currtask, taskname, details, percentCompleted, current=None ):
        print("hello1")
    def corpusMirrorCompleted( current=None ):
        print("hello2")

class CorpusServerTest(unittest.TestCase,cvac.CorpusCallback):

    ic = None
    cs = None

    @classmethod
    def setUpClass(cls):
        # needs Python 2.7 or higher
        pass

    @classmethod
    def tearDownClass(cls):
        # needs Python 2.7 or higher
        pass
    
    #
    # Test the initialization of Ice and the service proxy
    #
    def setUp(self):
        self.ic = Ice.initialize(sys.argv)
        base = self.ic.stringToProxy("CorpusServer:default -p 10011")
        self.cs = cvac.CorpusServicePrx.checkedCast(base)
        if not self.cs:
            raise RuntimeError("Invalid proxy")

    #
    # Test if we can open a Corpus with an existing properties file;
    # note that this doesn't try to create a local mirror which would download
    # the entire Caltech101 data set
    #
    def test_openCorpus(self):
        print('openCorpus')
        dataRoot = cvac.DirectoryPath( "corpus" );
        corpusConfigFile = cvac.FilePath( dataRoot, "Caltech101.properties" )
        corpus2 = self.cs.openCorpus( corpusConfigFile )
        if not corpus2:
            raise RuntimeError("could not open corpus from config file at '"
                               +dataRoot.relativePath+"/"+corpusConfigFile.filename+"'")

    #
    # Test if we can open a Label e Corpus with an existing properties file,
    # if so, try to obtain a Labelable dataset from it
    #
    def test_openCorpusLabelMe(self):
        print('openCorpusLabelMe')
        dataRoot = cvac.DirectoryPath( "corpus" );
        corpusConfigFile = cvac.FilePath( dataRoot, "LabelMeCarsTest.properties" )
#        corpusConfigFile = cvac.FilePath( dataRoot, "NpsVisionLabelMe.properties" )
        corpus = self.cs.openCorpus( corpusConfigFile )
        if not corpus:
            raise RuntimeError("could not open corpus from config file at '"
                               +dataRoot.relativePath+"/"+corpusConfigFile.filename+"'")
        adapter = self.ic.createObjectAdapter("")
        ident = Ice.Identity()
        ident.name = IcePy.generateUUID()
        ident.category = ""
        adapter.add( TestCorpusCallback(), ident )
        adapter.activate()
        
        self.cs.createLocalMirror( corpus, ident )

        labels = self.cs.getDataSet( corpus )
        if not labels:
            raise RuntimeError("could not obtain labels from Corpus '"
                               +corpus.name+"'")

    #
    # Test obtaining a Labelable set: first, expect a failure because the corpus
    # is not downloaded yet (local mirror)
    #
    def test_getDataSet(self):
        print('getDataSet')
        dataRoot = cvac.DirectoryPath( "corpus" );
        corpusConfigFile1 = cvac.FilePath( dataRoot, "Caltech101.properties" )
        corpus1 = self.cs.openCorpus( corpusConfigFile1 )
        try:
            labels = self.cs.getDataSet( corpus1 )
            raise RuntimeError("the CorpusServer should not be able to get the data",
                               "without creating a local mirror")
        except RuntimeError:
            # we expect this error: "could not obtain labels from Corpus..."
            pass

        # this time we expect to be able to access the labels
        corpusConfigFile2 = cvac.FilePath( dataRoot, "CvacCorpusTest.properties" )
        corpus2 = self.cs.openCorpus( corpusConfigFile2 )
        labels = self.cs.getDataSet( corpus2 )
        if not labels:
            raise RuntimeError("could not obtain labels from Corpus '"
                               +corpus2.name+"'")

    #
    # Does this corpus need a download to create local metadata?
    #
    def test_getDataSetRequiresLocalMirror(self):
        print('getDataSetRequiresLocalMirror')
        # try with one where we do expect it:
        dataRoot = cvac.DirectoryPath( "corpus" );
        corpusConfigFile1 = cvac.FilePath( dataRoot, "Caltech101.properties" )
        corpus1 = self.cs.openCorpus( corpusConfigFile1 )
        required = self.cs.getDataSetRequiresLocalMirror( corpus1 )
        if not required:
            raise RuntimeError("Corpus", corpusConfigFile1.filename,
                               "is expected to require a local mirror for data access, but it does not")

        # now try with one that should not require it
        corpusTestDir = cvac.DirectoryPath( "corpusTestDir" );
        corpus2 = self.cs.createCorpus( corpusTestDir )
        required = self.cs.getDataSetRequiresLocalMirror( corpus2 )
        if required:
            raise RuntimeError("Corpus at", corpusTestDir.relativePath,
                               "is expected to not require a local mirror for data access, but it does")

    #
    # Has a local mirror already been created?  This will return true only
    # if this corpus requires a download, not for one that is local to begin with.
    #
    def test_localMirrorExists(self):
        print('localMirrorExists')
        # try with one where we expect the mirror to exist already,
        # mainly because test_createLocalMirror has been called already
        dataRoot = cvac.DirectoryPath( "corpus" );
        corpusConfigFile = cvac.FilePath( dataRoot, "CvacCorpusTest.properties" )
        corpus = self.cs.openCorpus( corpusConfigFile )
        if not corpus:
            raise RuntimeError("could not open corpus from config file at '"
                               +dataRoot.relativePath+"/"+corpusConfigFile.filename+"'")
        exists = self.cs.localMirrorExists( corpus )
        if not exists:
            raise RuntimeError("expected corpus to already have a local mirror")
    
    #
    # Obtain a local mirror of the data set.
    #
    def test_createLocalMirror(self):
        print('createLocalMirror')
        dataRoot = cvac.DirectoryPath( "corpus" );
        corpusConfigFile = cvac.FilePath( dataRoot, "CvacCorpusTest.properties" )
        corpus = self.cs.openCorpus( corpusConfigFile )
        if not corpus:
            raise RuntimeError("could not open corpus from config file at '"
                               +dataRoot.relativePath+"/"+corpusConfigFile.filename+"'")

        adapter = self.ic.createObjectAdapter("")
        ident = Ice.Identity()
        ident.name = IcePy.generateUUID()
        ident.category = ""
#        adapter.add( self, ident )
        adapter.add( TestCorpusCallback(), ident )
        adapter.activate()
#        adapter = self.ic.createObjectAdapter("CorpusServer")
#        adapter.add( TestCorpusCallback(), ic.stringToIdentity("CorpusServer:default -p 10011"))
#        adapter.activate()
#        receiver = cvac.CorpusCallbackPrx.uncheckedCast(
#            adapter.createProxy( self.ic.stringToIdentity("callbackReceiver")))
        
        self.cs.createLocalMirror( corpus, ident )
        if not self.cs.localMirrorExists( corpus ):
            raise RuntimeError( "could not create local mirror for",
                                corpusConfigFile.filename )

    #
    # Create a Corpus from a directory of labeled data
    #
    def test_createCorpus(self):
        print('createCorpus')
        corpusTestDir = cvac.DirectoryPath( "corpusTestDir" );
        corpus3 = self.cs.createCorpus( corpusTestDir )
        if not corpus3:
            raise RuntimeError("could not create corpus from path '"
                               +dataRoot.relativePath+"/"+corpusTestDir+"'")
        

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

