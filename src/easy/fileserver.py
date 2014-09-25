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
import re

                                         
''' 
FileServiceI is the implementation for the CVAC FileService.
This FileService implementation keeps a history of which files a client
put on the server so that only those files may be removed again.
It does not permit "up" and absolute paths: ../path and /path.
'''
class FileServiceI(cvac.FileService, threading.Thread):   

    def __init__(self, communicator):
        threading.Thread.__init__(self)
        # Change stdout to automaticly flush output
        sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)
        #print("debug: starting service: FileService (Python)")
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
        self.fileToOwner = {}
        print("info: service started: FileService (Python)")

    def destroy(self):
        #print("debug: stopping service: FileService (Python)")
        self._cond.acquire()

        self._destroy = True

        try:
            self._cond.notify()
        finally:
            self._cond.release()

        self.join()
        print("info: service stopped: FileService (Python)")
        

    '''
    True if the file exists on the FileServer.
    The FileService might not permit clients to query for the existence of
    arbitrary files, instead, it will grant permissions only to files that
    were uploaded by the respective client.
    '''
    def exists( self, cvacPath, communicator ):
        # todo: use a function like easy.getFSPath, but one that
        # checks permissions of the current client;
        propFile = os.path.join(self.CVAC_DataDir, 
                                cvacPath.directory.relativePath,
                                cvacPath.filename)
        return os.path.isfile(propFile)

    
    def getOwnerName(self, communicator):
        comStr = communicator.con.toString()
        #split into 2 lines and scan the second
        lines = comStr.splitlines()
        scanner = re.Scanner([
                (r"[_a-zA-Z]+", lambda scanner, token:("IDENT", token)),
                (r"[0-9]+.[0-9]+.[0-9]+.[0-9]+", lambda scanner, token:("IPADDR", token)),
                (r":", lambda scanner, token:("COLON", token)),
                (r"=", lambda scanner, token:("EQUAL", token)),
                (r"\s+", None)
                              ])
        parselist, remainder = scanner.scan(lines[1])
        ownerName = "localhost"
        for entry in parselist:
            if entry[0] == "IPADDR":
                ownerName = entry[1]
        return ownerName
    

    ''' 
      Copies a local file at the specified FilePath to the same location
      on the remote file FileService.
    '''
    def putFile( self, cvacPath, byteSeq, communicator ):
        endpoint = self.getOwnerName(communicator)
        putFileName = os.path.join(self.CVAC_DataDir, 
                                cvacPath.directory.relativePath,
                                cvacPath.filename) 
        fse = cvac.FileServiceException()
        if putFileName.find("..") != -1:
            fse.msg("Relative paths outside data directory not supported")
            raise fse
        if os.path.isfile(putFileName):
            fse.msg = "File " + putFileName + " already exists"
            raise fse
        if putFileName in self.fileToOwner:
                owner = self.fileToOwner[putFileName]
                if owner != endpoint:
                    fse.msg = "Not owner of file " + putFileName + "!"
                    raise fse                 
        dirname = os.path.join(self.CVAC_DataDir, 
                                cvacPath.directory.relativePath)
        if not os.path.exists(dirname):
            os.makedirs(dirname)
        putFile = open(putFileName, 'wb')
        putFile.write(byteSeq)
        putFile.flush()
        putFile.close()
        self.fileToOwner[putFileName] = endpoint
        
          
        
    ''' 
      Copies a remote file at the specified FilePath to the same location
      on the local hard disk.
    '''
    def getFile( self, cvacPath, communicator):
        # return ByteSeq
        getFileName = os.path.join(self.CVAC_DataDir, 
                                cvacPath.directory.relativePath,
                                cvacPath.filename)  
        fse = cvac.FileServiceException()
        if getFileName.find("..") != -1:
            fse.msg = "Relative paths outside data directory not supported"
            raise fse
        if not os.path.isfile(getFileName):
            fse.msg = "File " + getFileName + " does not exist!"
            raise fse
        getFile = open(getFileName, 'rb')
        fileBytes = bytearray(getFile.read())
        return fileBytes
    '''
      Do the obvious.  Not permitted unless this client put the file there earlier.
    '''
    def deleteFile( self, cvacPath, communicator ):
        endpoint = self.getOwnerName(communicator)
        delFileName = os.path.join(self.CVAC_DataDir, 
                                cvacPath.directory.relativePath,
                                cvacPath.filename)  
        fse = cvac.FileServiceException()
        if os.path.isfile(delFileName):
            if delFileName in self.fileToOwner:
                owner = self.fileToOwner[delFileName]
                if owner != endpoint:
                    fse.msg = "File " + delFileName + " not created by caller!"
                    raise fse   
            os.unlink(delFileName)    
         

    '''
      Creates a new file *_snap.jpg next to the original file.  The download
      request needs to be made separately.
    '''
    def createSnapshot( self, cvacPath, communicator ):
        # return FilePath
        bytes = self.getFile(cvacPath, communicator)
        snapFilePath = cvac.FilePath()
        snapFilePath.directory = cvacPath.directory;
        snapFilePath.filename = cvacPath.filename + "_snap.jpg"
        self.putFile(snapFilePath, bytes, communicator)
        


    def hasVideoExtension(self, filename):
        filename, extension = os.path.splitext(filename)
        for ext in ['.jpg', '.jpeg', 'png', 'gif', 'pbm']:
            if ext == extension:
                return True
        return False
        
        
    def hasImageExtension(self, filename):
        filename, extension = os.path.splitext(filename)
        for ext in ['.avi', '.wmv', '.mpg']:
            if ext == extension:
                return True
        return False
        
        
    ''' Obtains read/write permissions of a file or directory.
      To obtain the permissions of a directory, use an empty
      filename component or a dot "."
    '''
    def getProperties( self, cvacPath, communicator ):
        # return FileProperties
        props = cvac.FileProperties()
        endpoint = self.getOwnerName(communicator)
        fileName = os.path.join(self.CVAC_DataDir, 
                                cvacPath.directory.relativePath,
                                cvacPath.filename)  
        if os.path.isfile(fileName):
            statinfo = os.stat(fileName)
            props.bytesize = statinfo.st_size;
            props.width = -1
            props.height = -1
            props.isImage = self.hasImageExtension(fileName)
            props.isVideo = self.hasVideoExtension(fileName)
            props.readPermitted = os.access(fileName, os.R_OK)
            if fileName in self.fileToOwner:
                owner = self.fileToOwner[fileName]
            props.writePermitted = owner == endpoint
            props.videoLength = cvac.VideoSeekTime(-1, -1)
        else:
            # file is not a file so assume directory
            props.bytesize = 0;
            props.width = -1
            props.height = -1
            props.isImage = False
            props.isVideo = False
            props.readPermitted = os.access(fileName, os.R_OK)
            props.writePermitted = os.access(fileName, os.W_OK)
            


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
