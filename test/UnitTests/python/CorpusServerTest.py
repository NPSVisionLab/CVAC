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
    corpus = None
    
    #
    # Test the initialization of Ice and the service proxy, then
    # obtain a corpus with which the remaining operations will be performed
    #
    def setUp(self):
        self.ic = Ice.initialize(sys.argv)
        base = self.ic.stringToProxy("CorpusServer:default -p 10011")
        self.cs = cvac.CorpusServicePrx.checkedCast(base)
        if not self.cs:
            raise RuntimeError("Invalid proxy")

        dataRoot = cvac.DirectoryPath( "corpus" );
        corpusConfigFile = cvac.FilePath( dataRoot, "CvacCorpusTest.properties" )
        self.corpus = self.cs.openCorpus( corpusConfigFile )
        if not self.corpus:
            raise RuntimeError("could not open corpus from config file at '"
                               +dataRoot.relativePath+"/"+corpusConfigFile.filename+"'")

    #
    # Test if we can open another Corpus with an existing properties file
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

