# test the CorpusServer
# before calling "python CorpusServerTest.py", make sure this is set:
# export PYTHONPATH="/opt/Ice-3.4.2/python:test/UnitTests/python"

# (windows)
# SET PYTHONPATH="C:\Program Files (x86)\ZeroC_Ice\python";C:\Python33\python.exe CorpusServerTest.py

import sys, traceback
if "C:\Program Files (x86)\ZeroC_Ice\python" not in sys.path:
    sys.path.append("C:\Program Files (x86)\ZeroC_Ice\python")
import Ice
import cvac
import unittest


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
        dataRoot = cvac.DirectoryPath( "someroot" );
        corpusConfigFile = cvac.FilePath( dataRoot, "Caltech101.props" )
        corpus = self.cs.openCorpus( corpusConfigFile )

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

