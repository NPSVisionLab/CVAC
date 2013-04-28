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
import cvac
import unittest


class CorpusServerTest(unittest.TestCase):

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
        dataRoot = cvac.DirectoryPath( "corpus" );
        corpusConfigFile = cvac.FilePath( dataRoot, "Caltech101.properties" )
        corpus2 = self.cs.openCorpus( corpusConfigFile )
        if not corpus2:
            raise RuntimeError("could not open corpus from config file at '"
                               +dataRoot.relativePath+"/"+corpusConfigFile.filename+"'")

    #
    # Test obtaining a Labelable set
    #
    def test_getDataSet(self):
        labels = self.cs.getDataSet( self.corpus )
        if not labels:
            raise RuntimeError("could not obtain labels from Corpus '"
                               +self.corpus.name+"'")

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

