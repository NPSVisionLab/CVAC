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
        
    def openCorpus( self, cvacPath, current=None):
        if not type(cvacPath) is cvac.FilePath:
            raise RuntimeError("wrong argument type")
        print( 'openCorpus called' )
        return cvac.Corpus()
    
    def closeCorpus( self, corp, current=None ):
        if not type(corp) is cvac.Corpus:
            raise RuntimeError("wrong argument type")
        print( 'closeCorpus called' )
    
    def saveCorpus( self, corp, cvacPath, current=None ):
        if not type(corp) is cvac.Corpus:
            raise RuntimeError("wrong argument type")
        if not type(cvacPath) is cvac.FilePath:
            raise RuntimeError("wrong argument type")
        print( 'saveCorpus called' )
    
    def getDataSetRequiresLocalMirror( self, corp, current=None ):
        if not type(corp) is cvac.Corpus:
            raise RuntimeError("wrong argument type")
        print( 'getDataSetRequiresLocalMirror called' )
    
    def localMirrorExists( self, corp, current=None ):
        if not type(corp) is cvac.Corpus:
            raise RuntimeError("wrong argument type")
        print( 'localMirrorExists called' )
    
    def createLocalMirror( self, corp, callback, current=None ):
        if not type(corp) is cvac.Corpus:
            raise RuntimeError("wrong argument type")
        print( 'createLocalMirror called' )
    
    def getDataSet( self, corp, current=None ):
        if not type(corp) is cvac.Corpus:
            raise RuntimeError("wrong argument type")
        print( 'getDataSet called' )
    
    def addLabelable( self, corp, addme, current=None ):
        if not type(corp) is cvac.Corpus:
            raise RuntimeError("wrong argument type")
        if not type(addme) is cvac.LabelableList:
            raise RuntimeError("wrong argument type")
        print( 'addLabelable called' )
    
    def createCorpus( self, cvacdir, current=None ):
        if not type(cvacdir) is cvac.DirectoryPath:
            raise RuntimeError("wrong argument type")
        print( 'createCorpus called' )


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
