
from __future__ import print_function
# paths should setup the PYTHONPATH.  If you special requirements
# then use the following to set it up prior to running.
# export PYTHONPATH="/opt/Ice-3.4.2/python:./src/easy"
import os,sys
sys.path.append('''.''')

import random
import shutil
import zipfile
import datetime

DEF_TRAIN_PREFIX = "train_"
DEF_SANDBOX = "sboxes"

class ClientSandbox(object):
    def __init__(self,_clientName,_cvac_dataDir):
        self.mClientName = _clientName
        self.mCVAC_DataDir = _cvac_dataDir
        self.mTrainDir = "" #including mCVAC_DataDir
        self.mClientDir = "" #including mCVAC_DataDir
        
    def deleteTrainingDir(self):
        if self.mTrainDir: 
            shutil.rmtree(self.mTrainDir)
        self.mTrainDir = ""
        
    def getClientDir(self):
        relativeClientDir = DEF_SANDBOX + "/" + self.mClientName
        if not self.mClientDir:
            self.mClientDir = self.mCVAC_DataDir + "/" + relativeClientDir
            if not os.path.isdir(self.mClientDir):
                os.makedirs(self.mClientDir)
        return self.mClientDir,relativeClientDir
        
    def getTempDirname(self,_prefix):
        fstr = str(random.randint(1,sys.maxint)).zfill(len(str(sys.maxint)))
        return _prefix + fstr 
    
    def createTrainingDir(self):
        self.deleteTrainingDir()
        self.getClientDir()
        self.mTrainDir = self.getTempDirname(self.mClientDir + "/" + DEF_TRAIN_PREFIX)
        if os.path.isdir(self.mTrainDir):
            shutil.rmtree(self.mTrainDir)            
        os.makedirs(self.mTrainDir)
        return self.mTrainDir
        
class SandboxManager(object):
    def __init__(self,_cvac_dataDir):
        self.mCVAC_DataDir = _cvac_dataDir
        self.mDictSandBoxes = {}        
        
    def createClientName(self,_serviceName,_connectionName):
        tClientName = _serviceName + "_" + _connectionName
        if not self.mDictSandBoxes.get(tClientName):
            self.mDictSandBoxes[tClientName] =  ClientSandbox(tClientName,self.mCVAC_DataDir)
        return tClientName
    
    def createTrainingDir(self,_clientName):
        if self.mDictSandBoxes[_clientName]:
            return (self.mDictSandBoxes[_clientName]).createTrainingDir()
        else:
            return ""
        
    def deleteTrainingDir(self,_clientName):
        if self.mDictSandBoxes[_clientName]:
            (self.mDictSandBoxes[_clientName]).deleteTrainingDir()
        
    def getTrainingDir(self,_clientName):
        if self.mDictSandBoxes[_clientName]:
            return (self.mDictSandBoxes[_clientName]).getTrainingDir()
        else:
            return ""
        
    def createClientDir(self,_clientName):
        if self.mDictSandBoxes[_clientName]:
            return (self.mDictSandBoxes[_clientName]).getClientDir()
        else:
            return "",""
        
class DetectorDataArchive(object):
    def __init__(self):
        self.mProperty = {}
        self.mSrcFiles = []
        self.mPropertyFilename = "trainer.properties"  
        self.mArchivePath = ""      
    
    def setArchivePath(self,_dir,_prefix):
        _postfix = (datetime.datetime.now()).strftime("%m%d%y_%H%M")
        if _dir: _dir += "/"
        tArchiveName = _prefix + "_" + _postfix + ".zip"
        self.mArchivePath = _dir + tArchiveName
        return tArchiveName
    
    def setProperty(self,_key,_value):
        self.mProperty[_key] = _value
        
    def getProperty(self,_key):
        return self.mProperty.get(_key,None)
    
    def getFile(self,_key):
        return self.getProperty(_key)
    
    def addInfo(self,_key,_infoStr):
        self.setProperty(_key, _infoStr)        
    
    def addFile(self,_key,_filepath):                
        if not _filepath in self.mSrcFiles:            
            self.mSrcFiles.append(_filepath)
            filename = os.path.basename(_filepath)
            self.setProperty(_key, filename)        
    
    def createArchive(self,_dirForSrcfile):
        filepath = _dirForSrcfile + "/" + self.mPropertyFilename
        f = open(filepath,"w")
        for tkey in self.mProperty.keys():
            tvalue = self.mProperty[tkey]               
            f.write(str(tkey)+"="+str(tvalue)+"\n")
        f.close()
        self.addFile("DEFAULT_LOGFILE",filepath)        
                
        if not self.mArchivePath:
            raise RuntimeError("Empty archive path")
            
        compFile = zipfile.ZipFile(self.mArchivePath,'w',zipfile.ZIP_DEFLATED)
        for sfile in self.mSrcFiles:
            #onlyPath,onlyName = os.path.split(sfile)
            #compFile.write(sfile,onlyName)
            compFile.write(sfile,os.path.basename(sfile))
        compFile.close()     
        
    def unArchive(self,_zipfilepath,_dir):
        if not os.path.isfile(_zipfilepath):
            raise RuntimeError("No zip file: " +  _zipfilepath)

        fzip = []
        try: 
            fzip = zipfile.ZipFile(_zipfilepath,'r')
        except:                
            raise RuntimeError("Error: " + _zipfilepath + " is corrupted.")
        else:
            fzip.extractall(_dir)
            fzip.close()
        
        self.mProperty = {}     
        f = open(_dir + "/" + self.mPropertyFilename,"r")
        for line in f:
            if line[0]=='#':
                continue;                           
            line = line.rstrip('\r\n')                            
            temp = line.split('=')
            #debug
            print("name %s value %s" % (temp[0], temp[1]))
            if len(temp)>1:
                self.mProperty[temp[0]] = temp[1]                        
        f.close()
        return self.mProperty
        
        
class ArchiveHandler(object):
    def __init__(self,_cvac_dataDir):
        self.mSndBxManager = SandboxManager(_cvac_dataDir)
        self.mDDA = DetectorDataArchive()
        
    def createClientName(self,_serviceName,_connectionName):
        return self.mSndBxManager.createClientName(_serviceName,_connectionName)
    
    def createTrainingDir(self,_clientName):
        return self.mSndBxManager.createTrainingDir(_clientName)
    
    def createClientDir(self,_clientName):
        return self.mSndBxManager.createClientDir(_clientName)
    
    def deleteTrainingDir(self,_clientName):
        self.mSndBxManager.deleteTrainingDir(_clientName)
    
    def setArchivePath(self,_dir,_prefix):
        return self.mDDA.setArchivePath(_dir, _prefix)
        
    def addFile(self,_key,_filepath):
        self.mDDA.addFile(_key,_filepath)
        
    def addInfo(self,_key,_infoStr):
        self.mDDA.addInfo(_key,_infoStr)
        
    def getFile(self,_key):
        return self.mDDA.getFile(_key)
        
    def createArchive(self,_dirForSrcfile):
        self.mDDA.createArchive(_dirForSrcfile)
        
    def unArchive(self,_zipfilepath,_dir):
        return self.mDDA.unArchive(_zipfilepath, _dir)
