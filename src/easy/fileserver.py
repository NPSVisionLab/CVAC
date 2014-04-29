#
# Python implementation of a CVAC FileServer
# matz, April 2014
#

import os, sys
import threading, string
import ConfigParser
import StringIO
import Ice, IcePy
import cvac

                                         
''' 
FileServiceI is the implementation for the CVAC FileService.
This FileService implementation keeps a history of which files a client
put on the server so that only those files may be removed again.
It does not permit "up" and absolute paths: ../path and /path.
'''
class FileServiceI(cvac.FileService, threading.Thread):   

    def __init__(self, communicator):
        threading.Thread.__init__(self)
        self._communicator = communicator
        self._destroy = False
        self._clients = []
        self._cond = threading.Condition()
        self.mListTestFile=[]    
        self.CVAC_DataDir = self._communicator.getProperties().\
                  getProperty( "CVAC.DataDir" )
        self.ConnectionName = "localhost"
        self.ServiceName = ""
        self.corpToImp = {}
        print("Service started: Python FileService.")

    def destroy(self):
        self._cond.acquire()

        print("Exiting Python FileService")
        self._destroy = True

        try:
            self._cond.notify()
        finally:
            self._cond.release()

        self.join()
        

    '''
    True if the file exists on the FileServer.
    The FileService might not permit clients to query for the existence of
    arbitrary files, instead, it will grant permissions only to files that
    were uploaded by the respective client.
    '''
    def exists( self, cvacPath ):
        # todo: use a function like easy.getFSPath, but one that
        # checks permissions of the current client;
        propFile = os.path.join(self.CVAC_DataDir, 
                                cvacPath.directory.relativePath,
                                cvacPath.filename)
        # return file.exists( propFile )
        pass
    
    ''' 
      Copies a local file at the specified FilePath to the same location
      on the remote file FileService.
    '''
    def putFile( cvacPath, byteSeq ):
        pass
        
    ''' 
      Copies a remote file at the specified FilePath to the same location
      on the local hard disk.
    '''
    def getFile( cvacPath ):
        # return ByteSeq
        pass

    '''
      Do the obvious.  Not permitted unless this client put the file there earlier.
    '''
    def deleteFile( cvacPath ):
        # return void
        pass

    '''
      Creates a new file *_snap.jpg next to the original file.  The download
      request needs to be made separately.
    '''
    def createSnapshot( cvacPath ):
        # return FilePath
        pass

    ''' Obtains read/write permissions of a file or directory.
      To obtain the permissions of a directory, use an empty
      filename component or a dot "."
    '''
    def getProperties( cvacPath ):
        # return FileProperties
        pass


class Server(Ice.Application):
    def run(self, args):
        adapter = self.communicator().\
                     createObjectAdapter("PythonFileService")
        sender = FileServiceI(self.communicator())
        adapter.add(sender, self.communicator().\
                    stringToIdentity("PythonFileService"))
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
