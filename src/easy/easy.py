'''
Easy Computer Vision

easy.py is a high-level interface to the underlying metadata
description syntax and to the multi-language service-oriented
architecture.  Its goal is to hide the lower-leve complexities
and to provide, uhm, easy access functions to its functionality.
'''

from __future__ import print_function
import os
import sys, traceback
import shutil
# paths should setup the PYTHONPATH.  If you special requirements
# then use the following to set it up prior to running.
# export PYTHONPATH="/opt/Ice-3.4.2/python:./src/easy"
sys.path.append('''.''')
import Ice
import Ice
import IcePy
import cvac
import unittest
import stat
import threading

# for drawing only:
try:
    import Tkinter as tk
    from PIL import Image, ImageTk, ImageDraw
except:
    # don't raise an error now
    pass

#
# one-time initialization code, upon loading the module
#
ic = Ice.initialize(sys.argv)
defaultCS = None
# IF the environment variable is set, then use that else use data
CVAC_DataDir = os.getenv("CVAC_DATADIR", "data")

def getFSPath( cvacPath ):
    '''Turn a CVAC path into a file system path'''
    if isinstance(cvacPath, cvac.Labelable):
        cvacPath = cvacPath.sub.path
    elif isinstance(cvacPath, cvac.Substrate):
        cvacPath = cvacPath.path
    if isinstance( cvacPath, str ):
        path = CVAC_DataDir+"/"+cvacPath
    elif not cvacPath.directory.relativePath:
        path = CVAC_DataDir+"/"+cvacPath.filename
    else:
        path = CVAC_DataDir+"/"+cvacPath.directory.relativePath+"/"+cvacPath.filename
    return path

def getCvacPath( fsPath ):
    '''Turn a file system path into a CVAC FilePath'''
    # todo: should figure out what CVAC.DataDir is and parse that out, too
    drive, path = os.path.splitdrive( fsPath )
    path, filename = os.path.split( path )
    dataRoot = cvac.DirectoryPath( path );
    return cvac.FilePath( dataRoot, filename )

def isLikelyVideo( cvacPath ):
    videoExtensions = ['avi', 'mpg', 'wmv']
    for ext in videoExtensions:
        if cvacPath.filename.endswith(ext):
            return True
    return False

def isLikelyDir( path ):
    '''Return true if path is likely a directory.
    Note that the path could be on a remote server, therefore
    we cannot check for existence and type of path (dir, file)
    but instead have to guess from the file extension, if any'''
    dotidx = path.rfind(".")  # find last .
    if dotidx is -1:
        return True
    else:
        sepidx = path[dotidx:].rfind("/") # any / after .?
        if sepidx>-1:
            return True
    return False
        
def getLabelable( filepath, labelText=None ):
    '''Create a Labelable wrapper around the file, assigning
    a textual label if specified.'''
    if type(filepath) is str:
        filepath = getCvacPath( filepath )
    if labelText:
        label = cvac.Label( True, labelText, None, cvac.Semantics() )
    else:
        label = cvac.Label( False, "", None, cvac.Semantics() )
    isVideo = isLikelyVideo( filepath )
    substrate = cvac.Substrate( not isVideo, isVideo, filepath, 0, 0 )
    labelable = cvac.Labelable( 0.0, label, substrate )
    return labelable

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

def openCorpus( corpusPath, corpusServer=None ):
    '''Open a Corpus specified by a properties file,
       or create a new Corpus from all files in a given directory;
       if no corpusServer is specified, a default local server is
       expected at port 10011'''
    if not corpusServer:
        corpusServer = getDefaultCorpusServer()
        
    # switch based on whether corpusPath is likely a directory or not.
    # note that the corpusPath could be on a remote server, therefore
    # we can't check for existence and type of corpusPath (dir, file)
    # but instead have to guess from the file extension, if any
    if isLikelyDir(corpusPath):
        # create a new corpus
        cvacPath = cvac.DirectoryPath( corpusPath )
        corpus = corpusServer.createCorpus( cvacPath )
        if not corpus:
            raise RuntimeError("Could not create corpus from directory at '"
                               + getFSPath( cvacPath ))
    else:
        # open an existing corpus
        cvacPath = getCvacPath( corpusPath )
        corpus = corpusServer.openCorpus( cvacPath )
        if not corpus:
            raise RuntimeError("Could not open corpus from properties file '" 
                               + corpusPath + "' (" \
                               + getFSPath( cvacPath ) + ")")
    return corpus

def closeCorpus( corpus, corpusServer=None ):
    '''Close a previously opened Corpus, presumable to re-open
    it with an updated properties file or new files'''
    if not corpusServer:
        corpusServer = getDefaultCorpusServer()
    corpusServer.closeCorpus( corpus )
    
class CorpusCallbackI(cvac.CorpusCallback):
    corpus = None
    def corpusMirrorProgress( self, corp, numtasks, currtask, taskname, details,
            percentCompleted, current=None ):
        print("CorpusServer: mirroring '{0}', task {1}/{2}: {3} ({4}%)".\
              format( corp.name, currtask, numtasks, taskname, percentCompleted ))
    def corpusMirrorCompleted(self, corp, current=None):
        self.corpus = corp

def createLocalMirror( corpusServer, corpus ):
    '''Call the corpusServer to create the local mirror for the
    specified corpus.  Provide a simple callback for tracking.'''
    # ICE functionality to enable bidirectional connection for callback
    adapter = ic.createObjectAdapter("")
    cbID = Ice.Identity()
    cbID.name = Ice.generateUUID()
    cbID.category = ""
    callbackRecv = CorpusCallbackI()
    adapter.add( callbackRecv, cbID)
    adapter.activate()
    corpusServer.ice_getConnection().setAdapter(adapter)
    # this call will block and the callback will receive the corpus
    # before it returns
    corpusServer.createLocalMirror( corpus, cbID )
    if not callbackRecv.corpus:
        raise RuntimeError("could not create local mirror")

def getDataSet( corpus, corpusServer=None, createMirror=False ):
    '''Obtain the set of labels from the given corpus and return it as
    a dictionary of label categories.  Also return a flat list of all labels.
    The default local corpusServer is used if not explicitly specified.
    If the corpus argument is not given as an actual cvac.Corpus object
    but the argument is a string instead, an attempt is made to
    open (but not create) a Corpus object from the corpusServer.
    Note that this will fail if the corpus needs a local mirror but has not
    been downloaded yet, unless createMirror=True.'''

    # get the default CorpusServer if not explicitly specified
    if not corpusServer:
        corpusServer = getDefaultCorpusServer()

    if type(corpus) is str:
        corpus = openCorpus( corpusServer, corpus )
    elif not isinstance(corpus, cvac.Corpus):
        raise RuntimeError( "unexpected type for corpus:", type(corpus) )

    # print 'requires:', corpusServer.getDataSetRequiresLocalMirror( corpus )
    # print 'exists:', corpusServer.localMirrorExists( corpus )
    if corpusServer.getDataSetRequiresLocalMirror( corpus ) \
        and not corpusServer.localMirrorExists( corpus ):
        if createMirror:
            createLocalMirror( corpusServer, corpus )
        else:
            raise RuntimeError("local mirror required, won't create automatically " \
                               "(specify createMirror=True to do so)")

    labelList = corpusServer.getDataSet( corpus )
    categories = {}
    for lb in labelList:
        if lb.lab.name in categories:
            categories[lb.lab.name].append( lb )
        else:
            categories[lb.lab.name] = [lb]
    return (categories, labelList)

def printCategoryInfo( categories ):
    '''Categories are a dictionary, key being the label and the
    value being all samples of that label.'''
    if not categories:
        print("no categories, nothing to print")
        return
    sys.stdout.softspace=False;
    for key in sorted( categories.keys() ):
        klen = len( categories[key] )
        print("{0} ({1} artifact{2})".format( key, klen, ("s","")[klen==1] ))

def printSubstrateInfo( labelList, indent="", printLabels=False ):
    if not labelList:
        print("no labelList, nothing to print")
        return
    substrates = {}
    labels = {}
    for lb in labelList:
        # subpath = getFSPath( lb.sub.path )  # print file system paths
        subpath = lb.sub.path.directory.relativePath +\
                  "/" + lb.sub.path.filename # print cvac.FilePath
        if subpath in substrates:
            substrates[subpath] = substrates[subpath]+1
            if printLabels:
                labels[subpath].append( [getLabelText(lb.lab)] )
        else:
            substrates[subpath] = 1
            if printLabels:
                labels[subpath] = [getLabelText(lb.lab)]
    sys.stdout.softspace=False;
    for subpath in sorted( substrates.keys() ):
        numlabels = substrates[subpath]
        if printLabels:
            print("{0}{1} ({2} label{3}): {4}".\
                  format( indent, subpath, numlabels, ("s","")[numlabels==1],
                          ', '.join(labels[subpath]) ))
        else:
            print("{0}{1} ({2} label{3})".\
                  format( indent, subpath, numlabels, ("s","")[numlabels==1] ))

def printRunSetInfo( runset, printLabels=False ):
    '''You can pass in an actual cvac.RunSet or a dictionary with
    the runset and a classmap, as returned by createRunSet.'''
    classmap = None
    if type(runset) is dict and not runset['runset'] is None\
        and isinstance(runset['runset'], cvac.RunSet):
        classmap = runset['classmap']
        runset = runset['runset']
    if not runset or not isinstance(runset, cvac.RunSet) or not runset.purposedLists:
        print("no (proper) runset, nothing to print")
        return
    sys.stdout.softspace=False;
    for plist in runset.purposedLists:
        purposeText = plist.pur.ptype
        if purposeText is cvac.PurposeType.MULTICLASS:
            purposeText = "{0}, classID={1}".format( purposeText, plist.pur.classID)
        if isinstance(plist, cvac.PurposedDirectory):
            print("directory with Purpose '{0}'; not listing members"\
                  .format( purposeText ) )
        elif isinstance(plist, cvac.PurposedLabelableSeq):
            print("sequence with Purpose '{0}' and {1} labeled artifacts:"\
                  .format( purposeText, len(plist.labeledArtifacts) ) )
            printSubstrateInfo( plist.labeledArtifacts, indent="  ",
                                printLabels=printLabels )
        else:
            raise RuntimeError("unexpected plist type "+type(plist))
    if classmap:
        print("classmap:")
        for key in sorted( classmap.keys() ):
            print("  label '{0}': purpose {1}".\
                  format( key, getPurposeName( classmap[key] )) )

def getPurpose( purpose ):
    '''Try to convert the input in to cvac.Purpose'''
    if isinstance(purpose, cvac.Purpose):
        pass # all ok
    elif type(purpose) is str:
        # try to convert str to Purpose
        if "unpurpose" in purpose.lower():
            purpose = cvac.Purpose( cvac.PurposeType.UNPURPOSED, -1 )
        elif "pos" in purpose.lower():
            purpose = cvac.Purpose( cvac.PurposeType.POSITIVE, -1 )
        elif "neg" in purpose.lower():
            purpose = cvac.Purpose( cvac.PurposeType.NEGATIVE, -1 )
        else:
            try:
                classID = int(purpose)
                purpose = cvac.Purpose( cvac.PurposeType.MULTICLASS, classID )
            except:
                raise RuntimeError("unexpected type for purpose: {0}".\
                                   format(purpose))
    return purpose

def getPurposedLabelableSeq( runset, purpose ):
    '''Return the PurposedLabelableSeq in the runset that has
    the specified purpose, or None.'''
    if not runset.purposedLists:
        return None
    for purposedList in runset.purposedLists:
        if isinstance(purposedList, cvac.PurposedLabelableSeq) \
           and purposedList.pur==purpose:
            return purposedList
    return None

def addPurposedLabelablesToRunSet( runset, purpose, labelables ):
    '''Append labelables to a sequence with the same purpose.
    If the runset does not have one, add a new sequence.'''
    # make sure we're getting a list of Labelables
    assert( labelables and type(labelables) is list and \
            isinstance( labelables[0], cvac.Labelable ) )
    # see if runset already has a list with these purposes
    seq = getPurposedLabelableSeq( runset, purpose )
    if seq:
        seq.labeledArtifacts.extend( labelables )
    else:
        seq = cvac.PurposedLabelableSeq( purpose, labelables )
        if runset.purposedLists is None:
            runset.purposedLists = [seq]
        else:
            runset.purposedLists.append( seq )

def addToClassmap( classmap, key, purpose ):
    if not classmap is None:
        if key in classmap and not classmap[key]==purpose:
            raise RuntimeError("purpose mismatch, won't add new samples")
        classmap[key] = purpose

def _determineDefaultPurpose( label, purpose, classmap ):
    '''
    If a purpose is given, return that.  If not, return the purpose in the
    classmap mapping from label to Purpose.  If not found, return UNPURPOSED.
    '''
    if not purpose is None:
        return purpose
    if isinstance(label, cvac.Labelable) and label.lab.hasLabel and label.lab.name in classmap:
        return classmap[label.lab.name]
    assert( type(label) is str )
    if label in classmap:
        return classmap[label]
    return cvac.Purpose( cvac.PurposeType.UNPURPOSED )
    
def addToRunSet( runset, samples, purpose=None, classmap=None ):
    '''Add samples to a given RunSet.
    Take a look at the documentation for createRunSet for details.'''
    rnst = runset
    if type(runset) is dict and not runset['runset'] is None\
        and isinstance(runset['runset'], cvac.RunSet):
        if classmap is None:
            classmap = runset['classmap']
        rnst = runset['runset']
    if rnst is None or not isinstance(rnst, cvac.RunSet):
        raise RuntimeError("No runset given - use createRunSet instead")
        
    # convert purpose if given, but not proper type
    if not purpose is None:
        if not type(purpose) is cvac.Purpose and not type(purpose) is str:
            raise RuntimeError("Purpose must be specified as str or cvac.Purpose")
        purpose = getPurpose( purpose )
        
    if type(samples) is dict and not purpose is None:
        # all categories get identical purposes
        for key in samples.keys():
            addToClassmap( classmap, key, purpose )
            addPurposedLabelablesToRunSet( rnst, purpose, samples[key] )

    elif type(samples) is dict:
        # multiple categories, try to guess the purpose and
        # if not possible, fall back to MULTICLASS
        assert( purpose is None )
        pur_categories = []
        pur_categories_keys = sorted( samples.keys() )

        if len(samples) is 1:
            # single category - assume "unpurposed"
            purpose = _determineDefaultPurpose( samples.keys()[0], purpose, classmap )
            addPurposedLabelablesToRunSet( rnst, purpose, samples.values()[0] )
            return
        
        # if it's two classes, maybe one is called "pos" and the other "neg"?
        elif len(samples) is 2 \
              and not pur_categories_keys[0] in classmap \
              and not pur_categories_keys[1] in classmap:
            alow = pur_categories_keys[0].lower()
            blow = pur_categories_keys[1].lower()
            poskeyid = -1
            if "pos" in alow and "neg" in blow:
                # POSITIVES in keys[0]
                poskeyid = 0
            elif "neg" in alow and "pos" in blow:
                # POSITIVES in keys[1]
                poskeyid = 1
            if poskeyid != -1:
                pospur = cvac.Purpose( cvac.PurposeType.POSITIVE, 1 )
                negpur = cvac.Purpose( cvac.PurposeType.NEGATIVE, 0 )
                poskey = pur_categories_keys[poskeyid]
                negkey = pur_categories_keys[1-poskeyid]
                addPurposedLabelablesToRunSet( rnst, pospur, samples[poskey] )
                addPurposedLabelablesToRunSet( rnst, negpur, samples[negkey] )
                addToClassmap( classmap, poskey, pospur )
                addToClassmap( classmap, negkey, negpur )
                return

        # multi-class, assign purposes
        cnt = 0 # todo: obtain the first unused classID, as per classmap
        for key in pur_categories_keys:
            # honor Purpose assignments from a given classmap:
            if key in classmap:
                purpose = classmap[key]
            else:
                purpose = cvac.Purpose( cvac.PurposeType.MULTICLASS, cnt )
            addToClassmap( classmap, key, purpose )
            addPurposedLabelablesToRunSet( rnst, purpose, samples[key] )
            cnt = cnt+1

    elif type(samples) is list and len(samples)>0 and isinstance(samples[0], cvac.Labelable):
        # single category - assume "unpurposed"
        for lbl in samples:
            purpose = _determineDefaultPurpose( lbl, purpose, classmap )
            if lbl.lab.hasLabel:
                addToClassmap( classmap, lbl.lab.name, purpose )
            addPurposedLabelablesToRunSet( rnst, purpose, [lbl] )

    elif isinstance(samples, cvac.Labelable):
        # single sample - assume "unpurposed"
        if purpose is None:
            purpose = _determineDefaultPurpose( samples, purpose, classmap )
        else:
            if samples.lab.hasLabel:
                addToClassmap( classmap, samples.lab.name, purpose )
        addPurposedLabelablesToRunSet( rnst, purpose, [samples] )

    elif type(samples) is str and isLikelyDir( samples ):
        # single path to a directory.  Create a corpus, turn into RunSet, close corpus.
        corpus = openCorpus( samples, corpusServer=getDefaultCorpusServer() )
        categories, lablist = getDataSet( corpus, corpusServer=getDefaultCorpusServer() )
        addToRunSet( runset, categories, purpose=purpose, classmap=classmap )
        closeCorpus( corpus, corpusServer=getDefaultCorpusServer() )
        
    elif type(samples) is str and not isLikelyDir( samples ):
        # single file, create an unpurposed entry
        fpath = getCvacPath( samples )
        labelable = getLabelable( fpath )  # no label text, hence nothing for the classmap
        if purpose is None:
            purpose = cvac.Purpose( cvac.PurposeType.UNPURPOSED )
        addPurposedLabelablesToRunSet( rnst, purpose, [labelable] )
        
    else:
        raise RuntimeError( "don't know how to create a RunSet from ", type(samples) )

def createRunSet( samples, purpose=None, classmap=None ):
    '''Add all samples from the argument to a new RunSet.
    Determine whether this is a two-class (positive and negative)
    or a multiclass dataset and create the RunSet appropriately.
    Input argument can also be a string to a single file.
    Note that the positive and negative classes might not be
    determined correctly automatically.  Specifiy a single Purpose
    if all samples will have the same purpose.
    Return the mapping from Purpose (class ID) to label name.'''

    runset = cvac.RunSet()
    if classmap is None:
        classmap={}
    addToRunSet( runset, samples, purpose=purpose, classmap=classmap )
    return {'runset':runset, 'classmap':classmap}

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

def putFile( fileserver, filepath, testExistence=True ):
    '''Copy the file referenced by filepath to the specified fileserver.
    By default, test first if the file exists on the fileserver, and only
    putFile if it does not exist.
    '''
    if isinstance( filepath, str ):
        filepath = getCvacPath( filepath )
    if testExistence and fileserver.exists( filepath ):
        return

    origFS = getFSPath( filepath )
    if not os.path.exists( origFS ):
        raise RuntimeError("Cannot obtain FS path to local file:",origFS)
    forig = open( origFS, 'rb' )
    bytes = bytearray( forig.read() )
    
    # "put" the file's bytes to the FileServer
    fileserver.putFile( filepath, bytes );
    forig.close()

def getFile( fileserver, filepath ):
    if type(filepath) is str:
        filepath = getCvacPath( filepath )
    else:
        assert( type(filepath) is cvac.FilePath )
    localFS = getFSPath( filepath )
    if not os.path.exists( os.path.dirname(localFS) ):
        os.makedirs( os.path.dirname(localFS) )
    flocal = open( localFS, 'wb' )
    
    # "get" the file's bytes from the FileServer
    bytes = fileserver.getFile( filepath );
    flocal.write( bytes )
    flocal.close()
    if not os.path.exists( localFS ):
        raise RuntimeError("Cannot obtain FS path to local file:",origFS)

def collectSubstrates( runset ):
    '''obtain a set (a list without duplicates) of all
    substrates that occur in this runset or result set'''
    if not runset:
        return set()
    if type(runset) is list and isinstance(runset[0], cvac.Result):
        return _collectSubstratesFromResults( runset )
    elif type(runset) is dict:
        runset = runset['runset']
    substrates = set()
    for plist in runset.purposedLists:
        if isinstance(plist, cvac.PurposedDirectory):
            raise RuntimeError("cannot deal with PurposedDirectory yet")
        elif isinstance(plist, cvac.PurposedLabelableSeq):
            for lab in plist.labeledArtifacts:
                if not lab.sub in substrates:
                    substrates.add( lab.sub )
        else:
            raise RuntimeError("unexpected subclass of PurposedList: "+type(plist))
    return substrates

def _collectSubstratesFromResults( results ):
    '''
    Collect all image substrates of found labels;
    if the foundLabel does not have a path, use the path from
    the original
    '''
    substrates = {}
    for res in results:
        for lbl in res.foundLabels:
            if lbl.sub.isImage:
                if not lbl.sub.path.filename:
                    subpath = getFSPath( res.original )
                else:
                    subpath = getFSPath( lbl )
                if subpath in substrates:
                    substrates[subpath].append( lbl )
                else:
                    substrates[subpath] = [lbl]
    return substrates

def putAllFiles( fileserver, runset ):
    '''Make sure all files in the RunSet are available on the remote site;
    it is the client\'s responsibility to upload them if not.
    For reporting purposes, return what has and has not been uploaded.'''
    assert( fileserver and runset )

    # collect all "substrates"
    substrates = collectSubstrates( runset )
    
    # upload if not present
    uploadedFiles = []
    existingFiles = []
    for sub in substrates:
        if not isinstance(sub, cvac.Substrate):
            raise RuntimeError("Unexpected type found instead of cvac.Substrate:", type(sub))
        if not fileserver.exists( sub.path ):
            putFile( fileserver, sub.path, testExistence=False )
            uploadedFiles.append( sub.path )
        else:
            existingFiles.append( sub.path )

    return {'uploaded':uploadedFiles, 'existing':existingFiles}

def deleteAllFiles( fileserver, uploadedFiles ):
    '''Delete all files that were previously uploaded to the fileserver.
    For reporting purposes, return what has and has not been uploaded.'''
    assert( fileserver )

    # are there any files to delete?
    if not uploadedFiles:
        return

    # try top delete, ignore but log errors
    deletedFiles = []
    notDeletedFiles = []
    for path in uploadedFiles:
        if not isinstance(path, cvac.FilePath):
            raise RuntimeError("Unexpected type found instead of cvac.FilePath:", type(path))
        try:
            fileserver.deleteFile( path )
            deletedFiles.append( path )
        except cvac.FileServiceException:
            notDeletedFiles.append( path )

    return {'deleted':deletedFiles, 'notDeleted':notDeletedFiles}

def getTrainerProperties(trainer):
    ''' Get the trainer properties for this trainer'''
    trainerProps = trainer.getTrainerProperties()
    if not trainerProps:
        raise RuntimeError("Getting trainer properties failed")
    return trainerProps

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
    trainingFinished = False
    def message( self, level, messageString, current=None ):
        print("message (level " + str(level) + ") from trainer: "+messageString, end="")
        
    def createdDetector(self, detData, current=None):
        if not detData:
            raise RuntimeError("Finished training, but obtained no trained model")
        # print("Finished training, obtained a trained model (DetectorData)")
        self.detectorData = detData
        self.trainingFinished = True

def train( trainer, runset, callbackRecv=None, trainerProperties=None ):
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
    if not trainerProperties:
        trainerProperties = cvac.TrainerProperties()
        trainerProperties.verbosity = 3
    if type(runset) is dict:
        runset = runset['runset']
    trainer.process( cbID, runset, trainerProperties )

    # check results
    if not callbackRecv.detectorData:
        raise RuntimeError("no DetectorData received from trainer")    

    return callbackRecv.detectorData

def getDetectorProperties(detector):
    ''' Get the detector properties for this detector'''
    detectProps = detector.getDetectorProperties()
    if not detectProps:
        raise RuntimeError("Getting detector properties failed")
    return detectProps

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
    detectionFinished = False
    def foundNewResults(self, r2, current=None):
        # collect all results
        self.allResults.extend( r2.results )

def detect( detector, detectorData, runset, detectorProps=None, callbackRecv=None ):
    '''
    Synchronously run detection with the specified detector,
    trained model, and optional callback receiver.
    The detectorData has to be a filename of a pre-trained model, or
     empty if the detector is pre-configured with a model, or if it does
     not require a model.  Naturally, the model has to be
     compatible with the detector.
    The runset can be either a cvac.RunSet object or anything that
    createRunSet can turn into a RunSet.
    If a callback receiver is specified, this function returns nothing,
    otherwise, the obtained results are returned.
    '''

    # create a cvac.DetectorData object out of a filename
    if not detectorData:
        detectorData = getCvacPath( "" )
    elif type(detectorData) is str:
        detectorData = getCvacPath( detectorData )
    elif not type(detectorData) is cvac.FilePath:
        raise RuntimeError("detectorData must be either filename or cvac.FilePath")

    # if not given an actual cvac.RunSet, try to create a RunSet
    if isinstance(runset, cvac.RunSet):
        pass
    elif type(runset) is dict and not runset['runset'] is None\
        and isinstance(runset['runset'], cvac.RunSet):
        #classmap = runset['classmap']
        runset = runset['runset']
    else:
        res = createRunSet( runset )
        #classmap = res['classmap']
        runset = res['runset']

    # ICE functionality to enable bidirectional connection for callback
    adapter = ic.createObjectAdapter("")
    cbID = Ice.Identity()
    cbID.name = Ice.generateUUID()
    cbID.category = ""
    ourRecv = False  # will we use our own simple callback receiver?
    if not callbackRecv:
        ourRecv = True
        callbackRecv = DetectorCallbackReceiverI();
        callbackRecv.allResults = []
    adapter.add( callbackRecv, cbID )
    adapter.activate()
    detector.ice_getConnection().setAdapter(adapter)

    # connect to detector, initialize with a verbosity value
    # and the trained model, and run the detection on the runset
    if detectorProps == None:
        props = cvac.DetectorProperties()
    else:
        props = detectorProps
    detector.process( cbID, runset, detectorData, props )

    if ourRecv:
        return callbackRecv.allResults

def getPurposeName( purpose ):
    '''Returns a string to identify the purpose or an
    int to identify a multiclass class ID.'''
    if purpose.ptype is cvac.PurposeType.UNPURPOSED:
        return "unpurposed"
    elif purpose.ptype is cvac.PurposeType.POSITIVE:
        return "positive"
    elif purpose.ptype is cvac.PurposeType.NEGATIVE:
        return "negative"
    elif purpose.ptype is cvac.PurposeType.MULTICLASS:
        return purpose.classID
    elif purpose.ptype is cvac.PurposeType.ANY:
        return "any"
    else:
        raise RuntimeError("unexpected cvac.PurposeType")

def getLabelText( label, classmap=None, guess=False ):
    '''Return a label text for the label: either
    "unlabeled" or the name of the label or whatever
    Purpose this label maps to.'''
    if not label.hasLabel:
        return "unlabeled"
    if label.name is "":
        return "<empty>"
    text = label.name
    if classmap and text in classmap:
        mapped = classmap[text]
        if type(mapped) is cvac.Purpose:
            text = getPurposeName( mapped )
            if type(text) is int:
                if guess and text.isdigit():
                    text = 'class {0}'.format( text )
                else:
                    text = '{0}'.format( text )
        elif type(mapped) is str:
            text = mapped
        else:
            raise RuntimeError( "unexpected type for classmap elements: "+
                                type(mapped) )
    return text

def printResults( results, foundMap=None, origMap=None, inverseMap=False ):
    '''Print detection results as specified in a ResultSet.
    If classmaps are specified, the labels are mapped
    (replaced by) purposes: the foundMap maps found labels and
    the origMap maps the original labels.  The maps are Python
    dictionaries, mapping either a label to a Purpose or a label
    to a string.

    If inverseMap=True: Since detectors do not produce Purposes,
    but the foundMap maps labels to Purposes, it is assumed that
    the user wishes to replace a label that hints at the Purpose
    with the label that maps to that same Purpose.  For example,
    a result label of '12' is assumed to be a class ID.  The
    classmap might map 'face' to a Purpose(MULTICLASS, 12).
    Hence, we would replace '12' with 'face'.'''
    
    # create inverse map for found labels
    purposeLabelMap = {}
    if inverseMap:
        if foundMap:
            for key in foundMap.keys():
                pur = foundMap[key]
                if not type(pur) is cvac.Purpose:
                    break
                pid = getPurposeName( pur )
                if type(pid) is int:
                    pid = str(pid)
                    if pid in purposeLabelMap:
                        purposeLabelMap[pid] += ", " +key
                    else:
                        purposeLabelMap[pid] = key
    
    print('received a total of {0} results:'.format( len( results ) ))
    identical = 0
    for res in results:
        names = []
        for lbl in res.foundLabels:
            foundLabel = getLabelText( lbl.lab, foundMap, guess=True )
            if purposeLabelMap and foundLabel in purposeLabelMap:
                foundLabel = purposeLabelMap[foundLabel]
            names.append(foundLabel)
        numfound = len(res.foundLabels)
        origname = getLabelText( res.original.lab, origMap, guess=False )
        if origMap and purposeLabelMap:
            # also map the original label to purpose and back, which removes
            # ambiguities in case two labels map to the same purpose
            if res.original.lab in origMap and \
                    str(origMap[res.original.lab]) in purposeLabelMap:
                origname = purposeLabelMap[ str(origMap[res.original.lab]) ]
        print("result for {0} ({1}): found {2} label{3}: {4}".format(
            res.original.sub.path.filename, origname,
            numfound, ("s","")[numfound==1], ', '.join(names) ))
        if numfound==1 and origname.lower()==names[0].lower():
            identical += 1
    print('{0} out of {1} results had identical labels'.format( identical, len( results ) ))

def initGraphics():
    try:
        wnd = tk.Tk()
        wnd.title('results')
    except:
        wnd = None
        raise RuntimeError("cannot display images - do you have PIL installed?")
    return wnd

def showImage( img ):
    # open window, convert image into displayable photo
    wnd = initGraphics()
    photo = ImageTk.PhotoImage( img )
    
    # make the window the size of the image
    # position coordinates of wnd 'upper left corner'
    x = 0
    y = 0
    w = photo.width()
    h = photo.height()
    wnd.geometry("%dx%d+%d+%d" % (w, h, x, y))
    
    # Display the photo in a label (as wnd has no image argument)
    panel = tk.Label(wnd, image=photo)
    panel.pack(side='top', fill='both', expand='yes')    
    # save the panel's image from 'garbage collection'
    panel.image = photo

    # start the event loop
    wnd.mainloop()
    wnd = None

def drawResults( results ):
    if not results:
        print("no results, nothing to draw")
        return

    # first, collect all image substrates of found labels;
    # if the foundLabel doesn't have a path, use the path from
    # the original
    substrates = collectSubstrates( results )
    if not substrates:
        print("no labels and/or no substrates, nothing to draw");
        return

    # print out some summary information
    sys.stdout.softspace=False;
    for subpath in sorted( substrates.keys() ):
        numlabels = len( substrates[subpath] )
        print("{0} ({1} label{2})".format( subpath, numlabels, ("s","")[numlabels==1] ))

    # render the substrates with respective labels
    showImagesWithLabels( substrates )

def drawLabelables( lablist, maxsize=None ):
    # first, collect all image substrates of the labels
    substrates = {}
    for lbl in lablist:
        if lbl.sub.isImage:
            subpath = getFSPath( lbl.sub )
            if subpath in substrates:
                substrates[subpath].append( lbl )
            else:
                substrates[subpath] = [lbl]
    if not substrates:
        print("no labels and/or no substrates, nothing to draw");
        return
    showImagesWithLabels( substrates, maxsize )

def showImagesWithLabels( substrates, maxsize=None ):
    '''Takes a dictionary of type dict[file_system_path] = [Labelable] as
    input and renders every image with labels overlaid.  The size
    parameter can be used to display all images at the same size.'''
    # now draw
    for subpath in substrates:
        img = Image.open( subpath )
        scale = 1.0
        if not maxsize is None:
            scalex = float(img.size[0])/float(maxsize[0])
            scaley = float(img.size[1])/float(maxsize[1])
            scale = max( scalex, scaley )
            maxsize = int(img.size[0]/scale), int(img.size[1]/scale)
            img = img.resize( maxsize, Image.NEAREST ) # favor speed over quality
        for lbl in substrates[subpath]:
            # draw poly into the image
            if isinstance(lbl, cvac.LabeledLocation):
                if isinstance( lbl.loc, cvac.BBox):
                    a = lbl.loc.x/scale
                    b = lbl.loc.y/scale
                    c = a+lbl.loc.width/scale
                    d = b+lbl.loc.height/scale
                    draw = ImageDraw.Draw( img )
                    draw.line([(a,b), (c,b), (c,d), (a,d), (a,b)], fill=255, width=2)
                    del draw
                elif isinstance( lbl.loc, cvac.Silhouette):
                    if len(lbl.loc.points) is 0:
                        continue
                    if len(lbl.loc.points) is 1:
                        raise RuntimeError("Incorrect Labelable: a single point should not be a silhouette")
                    cpts = []
                    for point in lbl.loc.points:
                        cpts.append( (point.x/scale, point.y/scale) )
                    cpts.append( cpts[0] )  # close the loop
                    draw = ImageDraw.Draw( img )
                    draw.line( cpts, fill=255, width=2 )
                    del draw
                else:
                    print("warning: not rendering Label type {0}".format( type(lbl.loc) ))
        showImage( img )

def getConfusionMatrix( results, origMap, foundMap ):
    '''produce a confusion matrix'''
    import numpy
    catsize = len( origMap )
    if catsize>50:
        pass
    confmat = numpy.empty( (catsize+1, catsize+1) )
    return confmat

def getHighestConfidenceLabel( lablist ):
    '''
    Return the label in the list that has the highest confidence.
    If multiple labels have the same high confidence, the first of these
    is returned.
    '''
    highest = None
    maxconf = 0.0
    for lbl in lablist:
        if lbl.confidence>maxconf:
            highest = lbl
    return highest

def sortIntoFolders( results, outfolder, multi="highest", symlink=False ):
    '''
    Sort the substrates from the original labels into subfolders of
    "outfolder" corresponding to the found labels;
    if multiple labels were found per original, consider only
    the label with the highest confidence unless otherwise
    specified.  Create symlinks if desired and if possible (on Unix),
    copy files by default.  CVAC.DataDir must be absolute for symlinks to work.
    Note: this method is intended for images, not videos.
    '''
    if not results:
        print("No results, nothing to sort.")
        return
    if not multi.lower()=="highest":
        raise RuntimeError("Multi-match strategy "+multi+" unknown.")
    if symlink:
        if not os.name.lower() is "posix":
            raise RuntimeError("Please check if symlinks work on your platform "\
                               "and change this code accordingly. "\
                               "(os.name==" + os.name + ")")

    # Match strategy "highest" means that a substrate gets put into at most
    # one output folder;
    # First collect substrates and their labels
    substrates = collectSubstrates( results )
    for sub in substrates.keys():
        lbl = getHighestConfidenceLabel( substrates[sub] )
        if lbl:
            # create directory and symlink according to found label
            assert( lbl.lab.hasLabel )
            dirname = outfolder + "/" + lbl.lab.name
            if not os.path.isdir( dirname ):
                os.makedirs( dirname )
            path, filename = os.path.split( sub )
            fpath = dirname + "/" + filename
            if symlink:
                # create a symlink; note that "sub" must be an absolute path
                # or else this will not work well on the OS
                os.symlink( sub, fpath )
            else:
                # copy the file
                shutil.copy2( sub, fpath )

def testResultSyntax( results, runset=None ):
    '''
    More a unit testing routine than for user consumption, this function
    tests whether the result data structure returned from a detector is
    valid.  It checks the data structure composition as well as some
    required contents and reports errors accordingly.  A boolean return
    value signals success.
    If a runset is given, the results\' original labels are compared
    against the labels provided in the runset.
    '''
    # make sure the top-level data structure is ok
    if not type(results) is list:
        print("results must be a list, but is "+type(results))
        return False
    if not results:
        print("empty results, nothing to test")
        return True

    if runset:
        print("TODO: implement comparison against runset labels")
        # there must be at least one result for every runset label
        return False

    #
    for res in results:
        if not isinstance( res, cvac.Result ):
            print("Incorrect Result type: "+type(res) )
            return False
        for lbl in res.foundLabels:
            if not isinstance( lbl, cvac.Labelable ):
                print("Incorrect Labelable type: "+type(lbl))
                return False
            if not lbl.lab or not lbl.lab.hasLabel:
                print("Empty Labelable should not be part of results.")
                return False
            if not lbl.sub.isImage and not lbl.sub.isVideo:
                print("Label must be either video or image: "+lbl)
                return False
            if lbl.confidence==0.0:
                print("For found labels, the confidence cannot be zero.")
                return False
            if lbl.confidence<=0.0 or lbl.confidence>1.0:
                print("Label confidence out of (0..1] range: "+lbl.confidence)
                return False
    return True
