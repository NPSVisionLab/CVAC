#
# Python implementation of a CVAC CorpusServer
# matz, Nov 2013
#

import os, sys
import threading, Ice
import IcePy
import cvac

class CorpusServiceI(cvac.CorpusService, threading.Thread):   

    def __init__(self, communicator):
        threading.Thread.__init__(self)
        self._communicator = communicator
        self._destroy = False
        self._clients = []
        self._cond = threading.Condition()
        self.mListTestFile=[]    
        self.CVAC_DataDir = None
        self.ConnectionName = "localhost"
        self.ServiceName = ""
        print("Service started: Python CorpusService.")

    def destroy(self):
        self._cond.acquire()

        print("Exiting Python CorpusService")
        self._destroy = True

        try:
            self._cond.notify()
        finally:
            self._cond.release()

        self.join()
        
    def openCorpus( cvacPath ):
        if not type(cvacPath) is cvac.FilePath:
            raise RuntimeError("wrong argument type")
        pass
    
    def closeCorpus( corp ):
        if not type(corp) is cvac.Corpus:
            raise RuntimeError("wrong argument type")
        pass
    
    def saveCorpus( corp, cvacPath ):
        if not type(corp) is cvac.Corpus:
            raise RuntimeError("wrong argument type")
        if not type(cvacPath) is cvac.FilePath:
            raise RuntimeError("wrong argument type")
        pass
    
    def getDataSetRequiresLocalMirror( corp ):
        if not type(corp) is cvac.Corpus:
            raise RuntimeError("wrong argument type")
        pass
    
    def localMirrorExists( corp ):
        if not type(corp) is cvac.Corpus:
            raise RuntimeError("wrong argument type")
        pass
    
    def createLocalMirror( corp, callback ):
        if not type(corp) is cvac.Corpus:
            raise RuntimeError("wrong argument type")
        pass
    
    def getDataSet( corp ):
        if not type(corp) is cvac.Corpus:
            raise RuntimeError("wrong argument type")
        pass
    
    def addLabelable( corp, addme ):
        if not type(corp) is cvac.Corpus:
            raise RuntimeError("wrong argument type")
        if not type(addme) is cvac.LabelableList:
            raise RuntimeError("wrong argument type")
        pass
    
    def createCorpus( cvacdir ):
        if not type(cvacdir) is cvac.DirectoryPath:
            raise RuntimeError("wrong argument type")
        pass


class Server(Ice.Application):
    def run(self, args):
        adapter = self.communicator().createObjectAdapter("PythonCorpusService")
        sender = CorpusServiceI(self.communicator())
        adapter.add(sender, self.communicator().stringToIdentity("PythonCorpusService"))
        self.communicator().getProperties().setProperty( "CVAC.DataDir", "data" )
        
        adapter.activate()
        
        sender.start()
        try:
            #print self.communicator().getProperties()
            self.communicator().waitForShutdown()
        finally:
            sender.destroy()

        return 0

app = Server()
sys.exit(app.main(sys.argv, "config.service"))
