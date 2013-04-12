# test the CorpusServer
# before calling "python CorpusServerTest.py", make sure this is set:
# export PYTHONPATH="/opt/Ice-3.4.2/python:test/UnitTests/python"
import sys, traceback
sys.path.append('''c:\Program Files (x86)\Zeroc\Ice-3.4.2\python''')
sys.path.append('''c:\Program Files (x86)\Zeroc\Ice-3.4.2\slice''')
sys.path.append('''C:\Users\tomb\Documents\nps\git\myCVAC\CVACvisualStudio\test\UnitTests\python''')
sys.path.append('''C:\Users\tomb\Documents\nps\git\myCVAC\CVACvisualStudio\test\UnitTests\python\cvac''')
sys.path.append('''.''')
import Ice
import unittest

Ice.loadSlice('-Ic:/Program\ Files\ (x86)/Zeroc/Ice-3.4.2/slice --all ../../../src/Services.ice')
Ice.loadSlice('-Ic:/Program\ Files\ (x86)/Zeroc/Ice-3.4.2/slice --all ../../../src/Data.ice')
Ice.loadSlice('-Ic:/Program\ Files\ (x86)/Zeroc/Ice-3.4.2/slice --all ../../../src/Corpus.ice')

import cvac

class CorpusServerTest(unittest.TestCase):

    ic = None
    cs = None
    
    def setUp(self):
        self.ic = Ice.initialize(sys.argv)
        base = self.ic.stringToProxy("CorpusServer:default -p 10011")
        self.cs = cvac.CorpusServicePrx.checkedCast(base)
        if not self.cs:
            raise RuntimeError("Invalid proxy")

    def test_openCorpus(self):
        dataRoot = cvac.DirectoryPath( "test/UnitTests/python" );
        # corpusConfigFile = cvac.FilePath( dataRoot, "Caltech101.properties" )
        # corpus = self.cs.openCorpus( corpusConfigFile )
        corpusConfigFile = cvac.FilePath( dataRoot, "LabelMeAK47.properties" )
        corpus = self.cs.openCorpus( corpusConfigFile )
        labelList = self.cs.getDataSet(corpus)


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

