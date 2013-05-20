# test the CorpusServer
# before calling "python CorpusServerTest.py", make sure this is set:
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

class TestCorpusCallback(cvac.CorpusCallback):
    def corpusMirrorProgress( corpus, 
                numtasks, currtask, taskname, details, percentCompleted, current=None ):
        print "hello1"
    def corpusMirrorCompleted( current=None ):
        print "hello2"

class CorpusServerTest(unittest.TestCase,cvac.CorpusCallback):

    ic = None
    cs = None
    
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
        print 'openCorpus'
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
        print 'openCorpusLabelMe'
        dataRoot = cvac.DirectoryPath( "corpus" );
        corpusConfigFile = cvac.FilePath( dataRoot, "LabelMeCarsTest.properties" )
#        corpusConfigFile = cvac.FilePath( dataRoot, "NpsVisionLabelMe.properties" )
        corpus3 = self.cs.openCorpus( corpusConfigFile )
        if not corpus3:
            raise RuntimeError("could not open corpus from config file at '"
                               +dataRoot.relativePath+"/"+corpusConfigFile.filename+"'")
        adapter = self.ic.createObjectAdapter("")
        ident = Ice.Identity()
        ident.name = IcePy.generateUUID()
        ident.category = ""
        adapter.add( TestCorpusCallback(), ident )
        adapter.activate()
        
        self.cs.createLocalMirror( corpus3, ident )

        labels = self.cs.getDataSet( corpus3 )
        if not labels:
            raise RuntimeError("could not obtain labels from Corpus '"
                               +corpus3.name+"'")

    #
    # Test obtaining a Labelable set: first, expect a failure because the corpus
    # is not downloaded yet (local mirror)
    #
    def test_getDataSet(self):
        print 'getDataSet'
        labels = self.cs.getDataSet( self.corpus )
        if not labels:
            raise RuntimeError("could not obtain labels from Corpus '"
                               +self.corpus.name+"'")


    #
    # Obtain a local mirror of the data set.
    #
    def test_createLocalMirror(self):
        print 'createLocalMirror'
        dataRoot = cvac.DirectoryPath( "corpus" );
        corpusConfigFile = cvac.FilePath( dataRoot, "CvacCorpusTest.properties" )
        self.corpus = self.cs.openCorpus( corpusConfigFile )
        if not self.corpus:
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
        
        self.cs.createLocalMirror( self.corpus, ident )

    #
    # Create a Corpus from a directory of labeled data
    #
    def test_createCorpus(self):
        print 'createCorpus'
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

