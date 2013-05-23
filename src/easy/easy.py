#
# Easy Computer Vision
#
# easy.py is a high-level interface to CVAC, the
# Computer Vision Algorithm Collection.
#
# before interpreting this file, make sure this is set:
# export PYTHONPATH="/opt/Ice-3.4.2/python:./src/easy"
import os
import sys, traceback
sys.path.append('''.''')
import Ice
import Ice
import IcePy
import cvac
import unittest
import stat

#
# one-time initialization code, upon loading the module
#
ic = Ice.initialize(sys.argv)
defaultCS = None


def getFSPath( cvacPath ):
    '''Turn a CVAC path into a file system path'''
    path = cvacPath.directory.relativePath+"/"+cvacPath.filename
    return path

def getCvacPath( fsPath ):
    '''Turn a file system path into a CVAC FilePath'''
    # todo: should figure out what CVAC.DataDir is and parse that out, too
    drive, path = os.path.splitdrive( fsPath )
    path, filename = os.path.split( path )
    dataRoot = cvac.DirectoryPath( path );
    return cvac.FilePath( dataRoot, filename )

def getCorpusServer( configstr ):
    '''Connect to a Corpus server based on the given configuration string'''
    cs_base = ic.stringToProxy( configstr )
    if not cs_base:
        raise RuntimeError("CorpusServer not found in config:", configstr)
    cs = cvac.CorpusServicePrx.checkedCast(cs_base)
    if not cs:
        raise RuntimeError("Invalid CorpusServer proxy")
    return cs

def getDefaultCorpusServer():
    '''Returns the CorpusServer that is expected to run locally at port 10011'''
    global defaultCS
    if not defaultCS:
        defaultCS = getCorpusServer( "CorpusServer:default -p 10011" )
    return defaultCS

def openCorpus( corpusServer, corpusPath ):
    '''Open a Corpus specified by a properties file,
       or create a new Corpus from all files in a given directory'''
    # switch based on whether corpusPath is likely a directory or not.
    # note that the corpus could be on a remote server, therefore
    # we can't check for existence and type of corpusPath (dir, file)
    # but instead have to guess from the file extension, if any
    likelyDir = False
    dotidx = corpusPath.rfind(".")  # find last .
    if dotidx is -1:
        likelyDir = True
    else:
        sepidx = corpusPath[dotidx:].rfind("/") # any / after .?
        if sepidx>-1:
            likelyDir = True
        
    if likelyDir:
        # create a new corpus
        cvacPath = cvac.DirectoryPath( corpusPath )
        corpus = corpusServer.createCorpus( cvacPath )
        if not corpus:
            raise RuntimeError("Could not create corpus from directory at '"
                               + getFsPath( cvacPath ))
    else:
        # open an existing corpus
        cvacPath = getCvacPath( corpusPath )
        corpus = corpusServer.openCorpus( cvacPath )
        if not corpus:
            raise RuntimeError("Could not open corpus from properties file '"
                               + getFsPath( cvacPath ))
    return corpus

def getDataSet( corpusServer, corpus ):
    '''Obtain the set of labels from the given corpus and return it as
    a dictionary of label categories.  Also return a flat list of all labels.'''
    labelList = corpusServer.getDataSet( corpus )
    categories = {}
    for lb in labelList:
        if lb.lab.name in categories:
            categories[lb.lab.name].append( lb )
        else:
            categories[lb.lab.name] = [lb]
    return (categories, labelList)

def printCategoryInfo( categories ):
    sys.stdout.softspace=False;
    for key in sorted( categories.keys() ):
        klen = len( categories[key] )
        print "{0} ({1} artifact{2})".format( key, klen, ("s","")[klen==1] )

def createRunSet( categories ):
    '''Add all samples from the categories to a new RunSet.
    Determine whether this is a two-class (positive and negative)
    or a multiclass dataset and create the RunSet appropriately.
    Note that the positive and negative classes might not be
    determined correctly automatically.
    Return the mapping from Purpose (class ID) to label name.'''

    runset = None
    pur_categories = []
    pur_categories_keys = sorted( categories.keys() )
    cnt = 0
    for key in pur_categories_keys:
        purpose = cvac.Purpose( cvac.PurposeType.MULTICLASS, cnt )
        pur_categories.append( cvac.PurposedLabelableSeq( purpose, categories[key] ) )
        cnt = cnt+1
        runset = cvac.RunSet( pur_categories )

    return (runset, pur_categories_keys)

def getFileServer( configString ):
    '''Obtain a reference to a remote FileServer.
    Generally, every host of CVAC services also has one FileServer.'''
    fileserver_base = ic.stringToProxy( configString )
    if not fileserver_base:
        raise RuntimeError("no such FileService: "+configString)
    fileserver = cvac.FileServicePrx.checkedCast( fileserver_base )
    if not fileserver:
        raise RuntimeError("Invalid FileServer proxy")
    return fileserver

def getDefaultFileServer( detector ):
    '''Assume that a FileServer is running on the host of the detector
    at the default port (10110).  Obtain a connection to that.'''
    # what host is the detector running on?
    endpoints = detector.ice_getEndpoints()
    # debug output:
    #print endpoints
    #print type(endpoints[0])
    #print dir(endpoints[0])
    #print endpoints[0].getInfo()
    #print endpoints[0].getInfo().type()
    #print dir(endpoints[0].getInfo().type())
    #print "host: ", endpoints[0].getInfo().host, "<-"
    # expect to see only one Endpoint, of type IP
    if not len(endpoints) is 1:
        raise RuntimeError( "don't know how to deal with more than one Endpoint" )
    if not isinstance( endpoints[0].getInfo(), IcePy.IPEndpointInfo ):
        raise RuntimeError( "detector has unexpected endpoint(s):", endpoints )
    host = endpoints[0].getInfo().host

    # if host is empty, the detector is probably a local service and
    # there is no Endpoint
    if not host:
        host = "localhost"

    # get the FileServer at said host at the default port
    configString = "FileService:default -h "+host+" -p 10110"
    try:
        fs = getFileServer( configString )
    except RuntimeError:
        raise RuntimeError( "No default FileServer at the detector's host",
                            host, "on port 10110" )
    return fs

def putAllFiles( fileserver, runset ):
    '''Make sure all files in the RunSet are available on the remote site;
    it is the client\'s responsibility to upload them if not.
    For reporting purposes, return what has and has not been uploaded.'''
    assert( fileserver and runset )

    # collect all "substrates"
    substrates = set()
    for plist in runset.purposedLists:
        if type(plist) is cvac.PurposedDirectory:
            raise RuntimeException("cannot deal with PurposedDirectory yet")
        elif type(plist) is cvac.PurposedLabelableSeq:
            for lab in plist.labeledArtifacts:
                if not lab.sub in substrates:
                    substrates.add( lab.sub )
        else:
            raise RuntimeException("unexpected subclass of PurposedList")

    # upload if not present
    uploadedFiles = []
    existingFiles = []
    for sub in substrates:
        if not fileserver.exists( sub.path ):
            fileserver.putFile( sub.path )
            uploadedFiles.append( sub.path )
        else:
            existingFiles.append( sub.path )

    return (uploadedFiles, existingFiles)

def getTrainer( configString ):
    '''Connect to a trainer service'''
    trainer_base = ic.stringToProxy( configString )
    trainer = cvac.DetectorTrainerPrx.checkedCast( trainer_base )
    if not trainer:
        raise RuntimeError("Invalid DetectorTrainer proxy")
    return trainer

# a default implementation for a TrainerCallbackHandler, in case
# the easy user doesn't specify one;
# this will get called once the training is done
class TrainerCallbackReceiverI(cvac.TrainerCallbackHandler):
    detectorData = None
    def createdDetector(self, detData, current=None):
        self.detectorData = detData
        print "Finished training, obtained DetectorData of type", self.detectorData.type

def train( trainer, runset, callbackRecv=None ):
    '''A callback receiver can optionally be specified'''
    
    # ICE functionality to enable bidirectional connection for callback
    adapter = ic.createObjectAdapter("")
    cbID = Ice.Identity()
    cbID.name = Ice.generateUUID()
    cbID.category = ""
    if not callbackRecv:
        callbackRecv = TrainerCallbackReceiverI()
    adapter.add( callbackRecv, cbID)
    adapter.activate()
    trainer.ice_getConnection().setAdapter(adapter)

    # connect to trainer, initialize with a verbosity value, and train
    trainer.initialize( 3 )
    trainer.process( cbID, runset )
    
    if callbackRecv.detectorData.type == cvac.DetectorDataType.BYTES:
        raise RuntimeError('detectorData as BYTES has not been tested yet')
    elif callbackRecv.detectorData.type == cvac.DetectorDataType.PROVIDER:
        raise RuntimeError('detectorData as PROVIDER has not been tested yet')

    return callbackRecv.detectorData

def getDetector( configString ):
    '''Connect to a detector service'''
    detector_base = ic.stringToProxy( configString )
    detector = cvac.DetectorPrx.checkedCast(detector_base)
    if not detector:
        raise RuntimeError("Invalid Detector service proxy")
    return detector

# a default implementation for a DetectorCallbackHandler, in case
# the easy user doesn't specify one;
# this will get called when results have been found;
# replace the multiclass-ID label with the string label
class DetectorCallbackReceiverI(cvac.DetectorCallbackHandler):
    allResults = []
    purLabelMap = None
    def foundNewResults(self, r2, current=None):
        # If we have a map from ordinal class number to label name,
        # replace the result label
        if self.purLabelMap:
            for res in r2.results:
                for lbl in res.foundLabels:
                    lbl.lab.name = self.purLabelMap[int(lbl.lab.name)]
        # collect all results
        self.allResults.extend( r2.results )

def detect( detector, detectorData, runset, purLabelMap=None, callbackRecv=None ):
    '''Synchronously run detection with the specified detector,
    trained model, and optional callback receiver.
    The detectorData can be either a cvac.DetectorData object or simply
     a filename of a pre-trained model.
    The runset can be either a cvac.RunSet object, filename to a single
     file that is to be tested, or a directory path.
    If a callback receiver is specified, this function returns nothing.'''

    # create a cvac.DetectorData object out of a filename
    if type(detectorData) is str:
        ddpath = getCvacPath( detectorData );
        detectorData = cvac.DetectorData( cvac.DetectorDataType.FILE, None, ddpath, None )
    elif not type(detectorData) is cvac.DetectorData:
        raise RuntimeException("detectorData must be either filename or cvac.DetectorData")

    # create a RunSet out of a filename or directory path
    if type(runset) is str:
        runset = createRunSet( runset )
    elif not type(runset) is cvac.RunSet:
        raise RuntimeException("runset must either be a filename, directory, or cvac.RunSet")
    
    # ICE functionality to enable bidirectional connection for callback
    adapter = ic.createObjectAdapter("")
    cbID = Ice.Identity()
    cbID.name = Ice.generateUUID()
    cbID.category = ""
    ourRecv = False  # will we use our own simple callback receiver?
    if not callbackRecv:
        ourRecv = True
        callbackRecv = DetectorCallbackReceiverI();
        callbackRecv.purLabelMap = purLabelMap
    adapter.add( callbackRecv, cbID )
    adapter.activate()
    detector.ice_getConnection().setAdapter(adapter)

    # connect to detector, initialize with a verbosity value
    # and the trained model, and run the detection on the runset
    detector.initialize( 3, detectorData )
    detector.process( cbID, runset )

    if ourRecv:
        return callbackRecv.allResults

def printResults( results ):
    '''Print detection results as specified in a ResultSet'''
    print 'received a total of {0} results:'.format( len( results ) )
    for res in results:
        names = []
        for lbl in res.foundLabels:
            names.append(lbl.lab.name)
        numfound = len(res.foundLabels)
        origname = ("unlabeled", res.original.lab.name)[res.original.lab.hasLabel==True]
        print "result for {0} ({1}): found {2} label{3}: {4}".format(
            res.original.sub.path.filename, origname,
            numfound, ("s","")[numfound==1], ', '.join(names) )
