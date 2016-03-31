'''
Easy Computer Vision

easy.py is a high-level interface to the underlying metadata
description syntax and to the multi-language service-oriented
architecture.  Its goal is to hide the lower-leve complexities
and to provide, uhm, easy access functions to its functionality.
'''

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import
from future import standard_library
standard_library.install_aliases()
from builtins import str
from builtins import range
from past.utils import old_div
from builtins import object
import os
import re
import sys, traceback
import shutil
import tempfile
import datetime
import collections

# paths should setup the PYTHONPATH.  If you special requirements
# then use the following to set it up prior to running.
# export PYTHONPATH="/opt/Ice-3.4.2/python:./src/easy"
sys.path.append('''.''')
import Ice
import IcePy
import cvac
try:
    import queue
except Exception as ex:
    import queue as Queue

import util.misc as misc
import servsup.files
from servsup.ArchiveHandler import ArchiveHandler as ArchiveHandler
from servsup.ArchiveHandler import DetectorDataArchive as DetectorDataArchive

import unittest
import stat
import threading

import os


'''
  Class for handling gui drawing.  The drawing
  is handled by a separate GuiThread that
  allows callback threads to render.  Basiclly 
  the GueQueue puts in a queue messages for the
  GuiThread to draw.
'''
class GuiQueue(object):
    def __init__(self):
        self.queue = None
        self.guiThread = None
        self.queue = queue.Queue()
        self.windows = {}

    def startThread(self):
        if self.guiThread == None:
            self.guiThread = GuiThread(self.queue)
            # Python 3.0 requires the guithread to
            # be the main thread or we an intermittent crash on window close.
            self.guiThread.run()
            # For OSX use this thread for the guiThread
            #if sys.platform == 'darwin':
            #   self.guiThread.run()
            #else:
            #    self.guiThread.start()
    
    '''
     Render and Image into a window.  If its the first
     time the window is created (via creating the GuiThread).
     The window will stay open until the user closes it.  That
     will also end the GuiThread.
     NOTE: On OSX the GuiThead must be the main thread so this
     call will block until the user closes the window. This also
     means that for OSX the first imgWindow call must be made from
     the main thread of the program.  The other calls can then be
     made from anythread.
    '''    
    def imgWindow(self, img, window = 0):
        self.queue.put(("ImageWindow",img, window))
        # we wait to start the guiThread at the first imgWindow call
        # as it will create an icon.
        self.startThread();
        
    '''
    Create a window that is not the default window.  This allows
    for multiple windows to be displayed.
    '''
    def startWindow(self, img):
        wid = 1
        while wid in list(self.windows.keys()):
            wid = wid + 1
        self.windows[wid] = wid
        self.queue.put(("ImageWindow", img, wid))
        return wid                                   
        
    def closeAllWindows(self):
        for wid in list(self.windows.keys()):
            self.queue.put(("CloseWindow", wid))
        self.queue.put(("CloseWindow", 0))

guiqueue = None
# for drawing only:
try:

    import tkinter as tk

    from PIL import Image, ImageTk, ImageDraw
    from util.GuiThread import GuiThread
    guiqueue = GuiQueue()
except Exception as exc:
    # don't raise an error now
    print("No gui available " + str(exc))
    quiqueue = None

def cleanup():
    print("Cleanup called")
    ic.destroy()


#
# one-time initialization code, upon loading the module
#
args = sys.argv
args.append('--Ice.MessageSizeMax=100000')
# try to find the config.client that specifies what services
# might be running, their names, etc.
if os.path.isfile('config.client'):
    args.append('--Ice.Config=config.client')
else:
    # Search for config.client up the directory tree
    modulepath = os.path.dirname(__file__)
    config_client_path = os.path.abspath(modulepath)
    while config_client_path != None:
        if os.path.isfile(config_client_path + '/config.client'):
            args.append('--Ice.Config=' + config_client_path + '/config.client')
            break
        else:
            nextdir = os.path.dirname(config_client_path)
            if nextdir == config_client_path:
                break
            else:
                config_client_path = nextdir

ic = Ice.initialize(args)
import atexit
atexit.register(cleanup)

defaultCS = None
# Parse the client.config for CVAC_DataDir
CVAC_DataDir = ic.getProperties().getProperty("CVAC.DataDir")
if CVAC_DataDir == None:
    CVAC_DataDir = "data"
# IF the environment variable is set, then use that else use data
CVAC_DataDir = os.getenv("CVAC_DATADIR", CVAC_DataDir)
if CVAC_DataDir == None or CVAC_DataDir == "":
    CVAC_DataDir = "data"
CVAC_ClientVerbosity = os.getenv("CVAC_CLIENT_VERBOSITY", "info") # info is verbosity level 2


def getFSPath( cvacPath, abspath=False ):
    '''Turn a CVAC path into a file system path'''
    if isinstance(cvacPath, cvac.Labelable):
        cvacPath = servsup.files.getLabelableFilePath(cvacPath)
    elif isinstance(cvacPath, cvac.ImageSubstrate):
        cvacPath = cvacPath.path
    elif isinstance(cvacPath, cvac.VideoSubstrate):
        cvacPath = cvacPath.videopath
    if isinstance( cvacPath, str ):
        # if cvacPath already has CVAC_DataDir then
        # don't add it again
        dirabs = os.path.abspath(cvacPath)
        cvacabs = os.path.abspath(CVAC_DataDir)
        if dirabs.startswith(cvacabs):
            path = cvacPath
        else:
            path = CVAC_DataDir+"/"+cvacPath
    elif isinstance(cvacPath, cvac.FilePath):
        # if cvacPath already has CVAC_DataDir then
        # don't add it again
        dirabs = os.path.abspath(cvacPath.directory.relativePath)
        cvacabs = os.path.abspath(CVAC_DataDir)
        if dirabs.startswith(cvacabs):
            path = cvacPath.directory.relativePath + '/' + cvacPath.filename
        else:
            path = CVAC_DataDir+"/"+cvacPath.directory.relativePath+"/"+cvacPath.filename
    elif isinstance(cvacPath, cvac.DirectoryPath):
        dirabs = os.path.abspath(cvacPath.directory.relativePath)
        cvacabs = os.path.abspath(CVAC_DataDir)
        if dirabs.startswith(cvacabs):
            path = cvacPath.relativePath
        else:
            path = CVAC_DataDir+"/"+cvacPath.relativePath
    else:
        dirabs = os.path.abspath(cvacPath.filename)
        cvacabs = os.path.abspath(CVAC_DataDir)
        if dirabs.startswith(cvacabs):
            path = cvacPath.filename
        else:
            path = CVAC_DataDir+"/"+cvacPath.filename
    if abspath == True:
        if os.path.isabs(path) == True:
            return path
        else:
            rootdir = os.getcwd()
            path = os.path.join(rootdir, path)
    return path

def getCvacPath( fsPath ):
    '''Turn a file system path into a CVAC FilePath'''
    # Remove the CVAC.DataDir from the fspath if there is one.
    fsPath = misc.stripCVAC_DataDir(fsPath)
    drive, path = os.path.splitdrive( fsPath )
    path, filename = os.path.split( path )
    dataRoot = cvac.DirectoryPath( path )
    return cvac.FilePath( dataRoot, filename )

def isLikelyVideo( cvacPath ):
    videoExtensions = ['avi', 'mpg', 'wmv', 'mov']
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
    if( isVideo ):
        # check whether this video is a single file or a directory
        # of individual frames
        substrate = getVideoSubstrate( filepath )
    else:
        substrate = cvac.ImageSubstrate( width=0, height=0, path=filepath )

    labelable = cvac.Labelable( 0.0, label, substrate )
    return labelable

def getVideoSubstrate( filepath ):
    '''
        Create a Labelable from a CVAC filepath.  This requires actual
        file system access to check whether individual frame images exist.
        It distinguishes the following cases:
        * the filepath is an actual video file:
          ** insert this filepath into the substrate
          ** if a directory exists with a name that starts with the
             filepath, such as avideo.mpg_frames/, look for frames there.
        * the filepath is a directory, not a file:
          ** look for frames inside this directory
    '''
    # do we have file system access?  if so, is the path a file (vs. directory)?
    fsPath = getFSPath( filepath )
    vpath = cvac.FilePath()
    if (os.path.isfile( fsPath )):
        vpath = filepath

    # is the path a directory?
    frm_paths = None
    if (os.path.isdir( fsPath )):
        # add each frame to the framepaths dictionary <int,str>
        # this assumes that the frames in the folder are named
        # according to their frames location (starting a 0). For example,
        # a file named 99.jpg corresponds to frame 100 of the movie.
        folder_listing = os.listdir( fsPath )
        frm_paths = {}
        for frm in folder_listing:
            if any(char.isdigit() for char in frm):
                # ignore filenames without numbers
                frm_num = int( frm.split( '.' )[ 0 ] )
                frm_paths[ frm_num ] = getCvacPath( fsPath + '/' + frm )
                print(frm_paths[ frm_num ])

    # if we didn't have file system access:
    if not vpath and not frm_paths:
        vpath = filepath
        
    substrate = cvac.VideoSubstrate( width=0, height=0,
                                     videopath=vpath, framepaths=frm_paths )
    return substrate

def getLabelableList(dirpath, recursive=True, video=True, image=True):
    '''Return a list of Labelables contained within the directory (optionally recursively)'''
    if type(dirpath) is str:
        if dirpath.startswith(CVAC_DataDir +'/'):
            strpath = dirpath
        else:
            strpath = getFSPath(dirpath)
    elif type(dirpath) is cvac.DirectoryPath:
        strpath = dirpath.relativePath
    res = []
    misc.searchDir(res, strpath, recursive, video, image)
    return res
    
        

def getCorpusServer( configstr ):
    '''Connect to a Corpus server based on the given configuration string'''
    proxyStr = getProxyString(configstr)
    cs_base = ic.stringToProxy( proxyStr )
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
        #first try configured Corpus Server
        proxstr = getProxyString("PythonCorpusService")
        if not proxstr:
            defaultCS = getCorpusServer( "PythonCorpusService:default -p 10011" )
        else:
            defaultCS = getCorpusServer( proxstr)
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
                               + corpusPath)
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
    def __init__(self):
        self.corpus = None
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
    categories = getCategories(labelList)
    return (categories, labelList)

def isInBounds( labelable ):
    '''Return True if the LabeledLocation is within image bounds.
    '''
    minX = 0
    minY = 0
    maxX = sys.maxsize 
    maxY = sys.maxsize                
    if labelable.sub.width>0:
        maxX = labelable.sub.width
    if labelable.sub.height>0:
        maxY = labelable.sub.height
    fpath = servsup.files.getLabelableFilePath(labelable)
    if isinstance(labelable, cvac.LabeledLocation):
        for pt in labelable.loc.points:
            if (pt.x < minX) or (pt.y < minY) \
                or (pt.x >= maxX) or (pt.y >= maxY):
                print("Warning: label \"" \
                      + labelable.label.name + "\" is out of bounds in file \"" \
                      + fpath.filename + "\"" \
                    + " (X=" + str(pt.x) + ", Y=" + str(pt.y) + ")")
                return False
#     else:
#         print("File= " + labelable.sub.path.filename)
#         print("Label= " + labelable.lab.name)
#         if labelable.lab.name in categories:
#             categories[labelable.lab.name].append(labelable)
#         else:       
#             categories[labelable.lab.name] = [labelable]
# 
#         if isinstance(labelable, cvac.LabeledTrack):
#             framestart = -1
#             curFrame = -1
#             for frame in labelable.keyframesLocations:
#                 frameNo = frame.frame.framecnt
#                 if framestart == -1:
#                     framestart = frameNo
#                     curFrame = frameNo
#                     if frameNo == curFrame:
#                         curFrame = curFrame + 1
#                     else:
#                         print ("Track frames {0} to {1}".format(framestart, curFrame))
#                         framestart = frameNo
#                         curFrame = frameNo + 1  
#                     if framestart != curFrame:   
#                         print ("Track frames {0} to {1}".format(framestart, curFrame))
    return True

def isProperRunSet(runset, deleteInvalid=False):
    '''Return True if the RunSet has proper syntax, False otherwise.
    If deleteInvalid==True, labeled artifacts with bounding boxes out
    of image bounds will be removed from the RunSet. 
    '''

    if type(runset) is dict and not runset['runset'] is None\
        and isinstance(runset['runset'], cvac.RunSet):
        runset = runset['runset']
        
    if not runset or not isinstance(runset, cvac.RunSet) or not runset.purposedLists:
        print("Not a valid runset")
        return False
    
    #categories = {}
    for plist in runset.purposedLists:
        if isinstance(plist, cvac.PurposedLabelableSeq) != True: 
            print("unexpected plist type "+type(plist))
            return False
        else:
            # process list backwards since we might be removing entries
            for i in range(len(plist.labeledArtifacts)-1, -1, -1):
                lb = plist.labeledArtifacts[i]               
                labelname  = 'nolabel'
                fpath = servsup.files.getLabelableFilePath(lb)
                if lb.lab.hasLabel != True:
                    print("Warning: " + fpath.path.filename + " has no label.")
                else:
                    labelname = lb.lab.name

                inBounds = isInBounds( lb )
                if not inBounds:
                    if deleteInvalid == False:
                        return False
                    else:
                        del plist.labeledArtifacts[i]
                        
    return True    

def printCategoryInfo( categories ):
    '''Categories are a dictionary, key being the label and the
    value being all samples of that label.'''
    if not categories:
        print("no categories, nothing to print")
        return
    sys.stdout.softspace=False
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
        subpath = getPrintableFileName( lb.sub )
        if subpath in substrates:
            substrates[subpath] = substrates[subpath]+1
            if printLabels:
                labels[subpath].append( [getLabelText(lb.lab)] )
        else:
            substrates[subpath] = 1
            if printLabels:
                labels[subpath] = [getLabelText(lb.lab)]
    sys.stdout.softspace=False
    for subpath in sorted( substrates.keys() ):
        numlabels = substrates[subpath]
        if printLabels:
            print("{0}{1} ({2} label{3}): {4}".\
                  format( indent, subpath, numlabels, ("s","")[numlabels==1],
                          ', '.join(labels[subpath]) ))
        else:
            print("{0}{1} ({2} label{3})".\
                  format( indent, subpath, numlabels, ("s","")[numlabels==1] ))

def printRunSetInfo( runset, printLabels=False, printArtifacts=True ):
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
    sys.stdout.softspace=False
    for plist in runset.purposedLists:
        purposeText = plist.pur.ptype
        if purposeText is cvac.PurposeType.MULTICLASS:
            purposeText = "{0}, classID={1}".format( purposeText, plist.pur.classID)
        if isinstance(plist, cvac.PurposedDirectory):
            print("directory with Purpose '{0}'; not listing members"\
                  .format( purposeText ) )
        elif isinstance(plist, cvac.PurposedLabelableSeq):
            print("sequence with Purpose '{0}' and {1} labeled artifacts{2}"\
                  .format( purposeText, len(plist.labeledArtifacts),
                           (".",":")[printArtifacts]) )
            if printArtifacts:
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

def getCategories(lablist):
    categories = {}
    for lb in lablist:
        if lb.lab.name in categories:
            categories[lb.lab.name].append(lb)
        else:
            categories[lb.lab.name] = [lb]
    return categories
    
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
       
    
    if type(samples) is dict and 'runset' in list(samples.keys()) and not samples['runset'] is None\
                and isinstance(samples['runset'], cvac.RunSet)\
                and not purpose is None:
            for item in samples['runset'].purposedLists:
                addPurposedLabelablesToRunSet(rnst, purpose, item.labeledArtifacts)
        
    elif type(samples) is dict and not purpose is None:
        # all categories get identical purposes      
        for key in list(samples.keys()):
            addToClassmap( classmap, key, purpose )
            addPurposedLabelablesToRunSet( rnst, purpose, samples[key] )

    elif type(samples) is dict:
        # multiple categories, try to guess the purpose and
        # if not possible, fall back to MULTICLASS
        assert( purpose is None )
        pur_categories_keys = sorted( samples.keys() )

        if len(samples) is 1:
            # single category - assume "unpurposed"
            purpose = _determineDefaultPurpose( list(samples.keys())[0], purpose, classmap )
            addPurposedLabelablesToRunSet( rnst, purpose, list(samples.values())[0] )
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
        lablist = getLabelableList(samples)
        categories = getCategories(lablist)
        addToRunSet( runset, categories, purpose=purpose, classmap=classmap )
        
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
    proxyStr = getProxyString(configString)
    fileserver_base = ic.stringToProxy( proxyStr )
    if not fileserver_base:
        raise RuntimeError("no such FileService: "+proxyStr)
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

    proxyStr = getProxyString('PythonFileService')
    if not proxyStr:
        # get the FileServer at said host at the default port
        configString = "PythonFileService:default -h "+host+" -p 10110"
    else:
        configString = proxyStr
    try:
        fs = getFileServer( configString )
    except RuntimeError:
        raise RuntimeError( "No default Python FileServer at the detector's host",
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
    mbytes = bytearray( forig.read() )
    
    # "put" the file's bytes to the FileServer
    fileserver.putFile( filepath, mbytes )
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
    mbytes = fileserver.getFile( filepath )
    flocal.write( mbytes )
    flocal.close()
    if not os.path.exists( localFS ):
        raise RuntimeError("Cannot obtain FS path to local file:",localFS)

def collectSubstrates( runset ):
    '''obtain a set (a list without duplicates) of all
    substrates that occur in this runset or result set'''
    if not runset:
        return collections.OrderedDict()
    if type(runset) is list and isinstance(runset[0], cvac.Result):
        return _collectSubstratesFromResults( runset )
    elif type(runset) is dict:
        runset = runset['runset']
    substrates = collections.OrderedDict()
    for plist in runset.purposedLists:
        if isinstance(plist, cvac.PurposedDirectory):
            raise RuntimeError("cannot deal with PurposedDirectory yet")
        elif isinstance(plist, cvac.PurposedLabelableSeq):
            for lab in plist.labeledArtifacts:
                substrates = _addLabelToSubstrates(substrates, lab)
        else:
            raise RuntimeError("unexpected subclass of PurposedList: "+type(plist))
    return substrates

def _addLabelToSubstrates(substrates, lbl, orig = None):
    if lbl.sub is None:
        if orig is None:
            return substrates # Cant' add if no file name
        else:
            fpath = servsup.files.getLabelableFilePath(orig)
    else:
        fpath = servsup.files.getLabelableFilePath(lbl)
   
    if not fpath.filename:
        if orig is None:
            return substrates
        else:
            subpath = getFSPath( orig )
    else:
        subpath = getFSPath( fpath )
    if subpath in substrates:
        substrates[subpath].append( lbl )
    else:
        substrates[subpath] = [lbl]
    return substrates
                

def _collectSubstratesFromResults( results ):
    '''
    Collect all image substrates of found labels;
    if the foundLabel does not have a path, use the path from
    the original
    '''
    substrates = collections.OrderedDict()
    for res in results:
        for lbl in res.foundLabels:
            _addLabelToSubstrates(substrates, lbl, orig=res.original)
                
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
    # iterate over the substrates keys which are file paths
    for fpath in substrates:
        cvacpath = getCvacPath(fpath)
        if not fileserver.exists( cvacpath):
            putFile( fileserver, cvacpath, testExistence=False )
            uploadedFiles.append( cvacpath )
        else:
            existingFiles.append( cvacpath )

    return {'uploaded':uploadedFiles, 'existing':existingFiles}

def getAllFiles( fileserver, runset ):
    '''Make sure all files in the RunSet are available on the local site;
    if they are not then they will be downloaded via the remote fileserver
    to the local site.
    For reporting purposes, return what has and has not been uploaded.'''
    assert( fileserver and runset )

    # collect all "substrates"
    substrates = collectSubstrates( runset )
    
    # upload if not present
    downloadedFiles = []
    existingFiles = []
    # iterate over the substrates keys which are file paths
    for fpath in substrates:
        cvacpath = getCvacPath(fpath)
        filepath = getFSPath(cvacpath)
        if not os.path.exists( filepath ):
            getFile( fileserver, cvacpath)
            downloadedFiles.append( cvacpath )
        else:
            existingFiles.append( cvacpath )

    return {'downloaded':downloadedFiles, 'existing':existingFiles}


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
    if type(uploadedFiles) is dict and not uploadedFiles['uploaded'] is None:
        uploadedFiles = uploadedFiles['uploaded']
    if not uploadedFiles:
        return
    for path in uploadedFiles:
        if not isinstance(path, cvac.FilePath):
            raise RuntimeError("Unexpected type found instead of cvac.FilePath:", type(path))
        try:
            fileserver.deleteFile( path )
            deletedFiles.append( path )
        except cvac.FileServiceException:
            notDeletedFiles.append( path )

    return {'deleted':deletedFiles, 'notDeleted':notDeletedFiles}

def getProxyString(configString):
    '''If we have a proxy string already then just return it.  If we have
    a detector name look it up in the config.client file and return the
    proxy string.  If we don't have either then return empty string '''
    scanner = re.Scanner([
      (r"[_a-zA-Z0-9]+", lambda scanner, token:("IDENT", token)),
      (r"-tp", lambda scanner, token:("PORT", token)),
      (r"-p", lambda scanner, token:("PORT", token)),
      (r":", lambda scanner, token:("COLON", token)),
      (r"@", lambda scanner, token:("LOC", token)),
      (r"\s+", None), #skip white space
    ])
    parselist, remainder = scanner.scan(configString)
    for entry in parselist:
        if entry[0] == "PORT" or entry[0] == "COLON" or entry[0] == "LOC":
            return configString
    # We don't have a proxy so lets see if we have a match in client.config
    properties = ic.getProperties()
    prop = properties.getProperty(configString + ".Proxy")
    return prop

def getTrainerProperties(trainer):
    ''' Get the trainer properties for this trainer'''
    trainerProps = trainer.getTrainerProperties()
    if not trainerProps:
        raise RuntimeError("Getting trainer properties failed")
    return trainerProps

def getTrainer( configString ):
    '''Connect to a trainer service'''
    proxyStr = getProxyString(configString)
    if not proxyStr:
        raise RuntimeError("Invalid or unknown Trainer configString")
    trainer_base = ic.stringToProxy( proxyStr )
    try:
        trainer = cvac.DetectorTrainerPrx.checkedCast( trainer_base )
    except Ice.ConnectionRefusedException:
        raise RuntimeError("Cannot connect to trainer '{0}'"
                           .format(proxyStr) )
    if not trainer:
        raise RuntimeError("Invalid DetectorTrainer proxy")
    return trainer

def getVerbosityNumber( verbosityString ):
    '''Convert error, info, warning etc into 1, 2, 3 etc.
    '''
    try:
        # first see if it's a string digit ("0", "1", etc)
        return int( verbosityString )
    except ValueError:
        pass
    if verbosityString.lower().startswith("silent"):
        return 0
    if verbosityString.lower().startswith("err"):
        return 1
    if verbosityString.lower().startswith("warn"):
        return 2
    if verbosityString.lower().startswith("info"):
        return 3
    if verbosityString.lower().startswith("debug1") or verbosityString.lower()=="debug":
        return 4
    if verbosityString.lower().startswith("debug2"):
        return 5
    if verbosityString.lower().startswith("debug3"):
        return 6

# a default implementation for a TrainerCallbackHandler, in case
# the easy user doesn't specify one;
# this will get called once the training is done
class TrainerCallbackReceiverI(cvac.TrainerCallbackHandler):
    def __init__(self):
        self.detectorData = None
        self.trainingFinished = False
        
    def message( self, level, messageString, current=None ):
        global CVAC_ClientVerbosity
        if isinstance(CVAC_ClientVerbosity, str):
            CVAC_ClientVerbosity = getVerbosityNumber( CVAC_ClientVerbosity )
        if level<=CVAC_ClientVerbosity:
            print("message (level {0}) from trainer: {1}"
                  .format( str(level), messageString), end="")
        
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
        trainerProperties.verbosity = getVerbosityNumber( CVAC_ClientVerbosity )
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
    proxyStr = getProxyString(configString)
    if not proxyStr:
        raise RuntimeError("Invalid or unknown Detector configString")
    detector_base = ic.stringToProxy( proxyStr )
    try:
        detector = cvac.DetectorPrx.checkedCast(detector_base)
    except Ice.ConnectionRefusedException:
        raise RuntimeError("Cannot connect to detector '{0}'"
                           .format(proxyStr) )
    if not detector:
        raise RuntimeError("Invalid Detector service proxy")
    return detector

# a default implementation for a DetectorCallbackHandler, in case
# the easy user doesn't specify one;
# this will get called when results have been found;
# replace the multiclass-ID label with the string label
class DetectorCallbackReceiverI(cvac.DetectorCallbackHandler):
    def __init__(self):
        self.allResults = []
        self.detectionFinished = False
        self.id = 0
        self.adapter = None
        
    def message( self, level, messageString, current=None ):
        global CVAC_ClientVerbosity
        if isinstance(CVAC_ClientVerbosity, str):
            CVAC_ClientVerbosity = getVerbosityNumber( CVAC_ClientVerbosity )
        if level<=CVAC_ClientVerbosity:
            print("message (level {0}) from detector: {1}"
                  .format( str(level), messageString), end="")
        
    def foundNewResults(self, r2, current=None):
        # collect all results
        self.allResults.extend( r2.results )
        
        
def discardSuboptimal(perfdata,saveRelativeDir = None):
    '''
    This function returns
    1) all ROC operating points [x-axis: false alarm, y-axis:recall] 
    2) index of optimal ROC points among all ROC points.
    Input variables are 
    1) perfdata = performance data from jousting 
    2) saveRelativeDir = directory for saving a log file (CAUTION: it's just for DEBUGGING)         
    '''    
    ptsROC = []    
    for data in perfdata:
        tp = float(data.res.tp)
        fp = float(data.res.fp)
        tn = float(data.res.tn)
        fn = float(data.res.fn)
        xaxis = 1.0
        if (tp+fp)!=0:
            xaxis = old_div(fp,(tp+fp))
        else:
            print("Warning: " + "Invalid RoC data: (tp+fp) = 0")
            #raise RuntimeError("Invalid RoC data: (tp+fp) = 0")        
        yaxis = 0.0
        if (tp+fn)!=0:
            yaxis = old_div(tp,(tp+fn))
        else:
            print("Warning: " + "Invalid RoC data: (tp+fn) = 0")
            #raise RuntimeError("Invalid RoC data: (tp+fn) = 0")
        ptsROC.append([xaxis,yaxis,(1.0-xaxis)*yaxis,tp,fp,tn,fn])
        
    index_optimal = []
    for i in range(0,len(ptsROC)):
        flagOpt = True
        for j in range(0,len(ptsROC)):
            if j==i:
                continue
            elif ptsROC[j][1]<ptsROC[i][1]:                
                continue
            elif ptsROC[j][1]==ptsROC[i][1]:
                if ptsROC[j][0]<ptsROC[i][0]:
                    flagOpt = False
                    break
                else:
                    continue                                    
            elif ptsROC[j][0]<ptsROC[i][0]:
                flagOpt = False
                break
            elif ptsROC[j][0]==ptsROC[i][0]:
                if ptsROC[j][1]>ptsROC[i][1]:
                    flagOpt = False
                    break
                else:
                    continue                          
        
        if flagOpt==True:
            index_optimal.append(i)
            
    if saveRelativeDir is not None: 
        fpathROC = getFSPath(saveRelativeDir+"/"\
                             +"RocTable_Full_"\
                           +(datetime.datetime.now()).strftime("%m%d%y_%H%M%S") + ".txt")                 
        f = open(fpathROC,'w')        
        for pt in ptsROC:
            f.write(str(pt[0]) + '\t')
            f.write(str(pt[1]) + '\n')  
        f.close() 
            
    return ptsROC,index_optimal

        
def getBestDetectorData(listRocData,dFAR,dRec):
    '''
    This function returns the best detector data among detectors from a ROC file
    according to a criteria.
    Users can select only one criteria at a time; false alarm rate or recall rate
    When users set both criterias, it will return the best f-scored detector data. 
    But, it may cause unexpected results. 
    '''
    if len(listRocData)<1 | len(listRocData[0])<3:
        raise RuntimeError("RoC Data must include at least three elements in a row: detectorData, x-axis and y-axis")
    
    resMsg = "The best detectorData is "
    bestDetectorData = None
    if (dFAR<0):
        #when an user sets recall rate
        valueSmallest = 1.0
        for elem in listRocData:
            dist= elem[2]-dRec
            if (dist>=0) & (dist<valueSmallest):
                valueSmallest = dist
                bestDetectorData = elem[0]        
        if bestDetectorData == None:
            valueBiggest = -1.0
            for elem in listRocData:
                dist= elem[2]-dRec
                if (dist>valueBiggest):
                    valueBiggest = dist
                    bestDetectorData = elem[0]
            resMsg = "Most likely detectorData is "
    elif (dRec<0):
        #when an user sets false alarm rate
        valueBiggest = -1.0
        for elem in listRocData:
            dist= elem[1]-dFAR
            if (dist<=0) & (dist>valueBiggest):
                valueBiggest = dist
                bestDetectorData = elem[0]        
        if bestDetectorData == None:
            valueSmallest = 1.0
            for elem in listRocData:
                dist= elem[1]-dFAR
                if (dist<valueSmallest):
                    valueSmallest = dist
                    bestDetectorData = elem[0]
            resMsg = "Most likely detectorData is "
    else:
        #when an user sets both criteia
        #actually, this case is not allowed in pre-screen routine
        valueBiggest = -1.0
        for elem in listRocData:
            distX = elem[1]-dFAR
            distY = elem[2]-dRec
            if (distX<=0) & (distY>=0):
                fvalue = (1.0-elem[1])*elem[2]
                if (fvalue>valueBiggest):
                    valueBiggest = fvalue
                    bestDetectorData = elem[0]
        
        if bestDetectorData == None:
            valueBiggest = -1.0
            for elem in listRocData:
                fvalue = (1.0-elem[1])*elem[2]
                if (fvalue>valueBiggest):
                    valueBiggest = fvalue
                    bestDetectorData = elem[0]
            resMsg = "Most likely detectorData is "
            
    resMsg = resMsg + bestDetectorData.filename
    
    print(resMsg)
    #strip off any leading CVAC_DataDir in the detector data
    bestDetectorData = misc.stripCVAC_DataDir_from_FilePath(bestDetectorData)
    return bestDetectorData

#from easy.util.ArchiveHandler import *
def makeROCdata(rocData_optimal):
    '''
    This function makes a single ZIP file incluing mulitple detector data 
    and their performance values (false alarm and recall).
    Performance values are written in the file "roc.properties"
    Input format: a list of [detectordata, false alarm, recall]
    '''
    
    ###############################
    # Make a single zip file
    ###############################
    rocArch = ArchiveHandler(CVAC_DataDir)
    rocArch.mDDA.mPropertyFilename = "roc.properties"
    clientName = rocArch.createClientName('ROC', 'TBD')
    tempDir = rocArch.createTrainingDir(clientName)
    clientDir,relClientDir = rocArch.createClientDir(clientName)
    rocZip_fileName = rocArch.setArchivePath(clientDir,"ROCdata")
    
    for roc in rocData_optimal:
        valueStr = str(roc[1]) + ', ' + str(roc[2])
        rocArch.addFile(roc[0].filename,getFSPath(roc[0]),valueStr)                        
    
    rocArch.createArchive(tempDir)
    rocArch.deleteTrainingDir(clientName)
    
    ###############################
    # Move the file to its parent folder
    ###############################
    rocZiptemp = cvac.FilePath()
    rocZiptemp.directory.relativePath = relClientDir                      
    rocZiptemp.filename = rocZip_fileName
    
    rocZip = cvac.FilePath()
    rocZip.directory.relativePath, basedir = os.path.split(relClientDir)                  
    rocZip.filename = rocZip_fileName
    
    rocZiptempPath = getFSPath(rocZiptemp)  
    shutil.move(rocZiptempPath,getFSPath(rocZip))
    
    dirname, filename = os.path.split(rocZiptempPath)
    shutil.rmtree(dirname)
    
    return rocZip

def isROCdata(rocZip):
    '''
    This function checks whether the input zip file is a ROC zip file or 
    not (a regular detector file). Decision is based on existence 
    of the file "roc.properties".
    Return 1) is it a ROC zip file or not
    Return 2) model files and their false alarm rate and recall rate 
    (if it is a ROC zip file). 
    Return 3) a temp folder including model files (if it is a ROC zip file).     
    '''
    zipfilepath = getFSPath(rocZip)
    # Make a temp directory under the data directory in a directory
    # called "clientTemp"
    tempdirloc = getFSPath("clientTemp")
    if os.path.isdir(tempdirloc) == False:
        os.makedirs(tempdirloc)
    tempDir = tempfile.mkdtemp(dir=tempdirloc)

    rocArch = DetectorDataArchive()
    rocArch.mPropertyFilename = "roc.properties"    
    rocDict = rocArch.unArchive(zipfilepath,tempDir)
    rocData_optimal = []
    isROC = False
    if len(rocDict) > 0:
        isROC = True
        for filename in rocDict:
            detectorData = cvac.FilePath()
            detectorData.directory.relativePath = tempDir #relDir
            detectorData.filename = filename
            tperf = rocDict[filename].split(',')
            rocData_optimal.append([detectorData,\
                                    float(tperf[0]),float(tperf[1])])
    return isROC,rocData_optimal,tempDir

def getSensitivityOptions(detectorData):
    '''
    Return any False Alarm, and Recall rate options available
    in the model file.  This will return a list of
    <false alarm, recall> pairs that
    have been trained into the model or None if they are not any.
    detectorData is the model file that that might contain the
    different model files and sensitivity options.
    '''
    isRoc, rockList, tempDir = isROCdata(detectorData)
    if isRoc == False:
        return None
    else:
        if tempDir != None:
            if os.path.isdir(tempDir):
                shutil.rmtree(tempDir)
        return rockList
    

def detect( detector, detectorData, runset, detectorProperties=None, callbackRecv=None, async=False):
    '''
    Run detection with the specified detector,
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

    if async == True and callbackRecv == None:
        raise RuntimeError("async parameter requires callbackRecv to be provided")
    # create a cvac.DetectorData object out of a filename
    tempDir = None
    if not detectorData:
        detectorData = getCvacPath( "" )
    elif type(detectorData) is str:
        detectorData = getCvacPath( detectorData ) 
    if type(detectorData) is cvac.FilePath:
        fileext = os.path.splitext(detectorData.filename)[1]
        if fileext.lower()==".zip":
            #check whether the zip file is roc data or not
            isROC, rocData_optimal, tempDir = isROCdata(detectorData)
            if isROC == True:
                if detectorProperties == None:
                    raise RuntimeError("For selecting the best detectorData, " + \
                                       "detectorProperties including desired precision " + \
                                       "and recall must be entered")
                dFAR = detectorProperties.falseAlarmRate
                dRec = detectorProperties.recall
                if (dFAR<0) & (dRec<0):
                    raise RuntimeError("Inapporopriate values for desired recall and precision")
                elif (dFAR>1.0) & (dRec>1.0):
                    raise RuntimeError("Inapporopriate values for desired recall and precision")
                elif (dFAR>0.0) & (dRec>0.0):
                    raise RuntimeError("Users can set only one criteria not both")                
                detectorData = getBestDetectorData(rocData_optimal,dFAR,dRec)
    elif not type(detectorData) is cvac.FilePath:
        raise RuntimeError("detectorData must be filename, cvac.FilePath, or RoC data")
    
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
    if not callbackRecv or callbackRecv.id == 0:
        adapter = ic.createObjectAdapter("")
        cbID = Ice.Identity()
        cbID.name = Ice.generateUUID()
        cbID.category = ""
        ourRecv = False  # will we use our own simple callback receiver?
        if not callbackRecv:
            ourRecv = True
            callbackRecv = DetectorCallbackReceiverI()
            callbackRecv.allResults = []
        else:
            callbackRecv.id = cbID
            callbackRecv.adapter = adapter
        adapter.add( callbackRecv, cbID )
        adapter.activate()
        detector.ice_getConnection().setAdapter(adapter)
    else:
        cbID = callbackRecv.id
        ourRecv = False  # will we use our own simple callback receiver?
         

    # connect to detector, initialize with a verbosity value
    # and the trained model, and run the detection on the runset
    if detectorProperties == None:
        detectorProperties = cvac.DetectorProperties()
        detectorProperties.verbosity = getVerbosityNumber( CVAC_ClientVerbosity )
    misc.stripCVAC_DataDir_from_FilePath(detectorData)
    results = None
    if async == False:
        detector.process( cbID, runset, detectorData, detectorProperties )
        if ourRecv:
            results = callbackRecv.allResults
    else:
        asyncRes = detector.begin_process( cbID, runset, detectorData, 
                            detectorProperties)
        # Wait to make sure that the request is sent, otherwise we queue
        # might overflow.
        asyncRes.waitForSent()
        results = asyncRes

    
    if tempDir != None:
        shutil.rmtree(tempDir)    

    return results

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
                if guess:
                    text = 'class {0}'.format( text )
                else:
                    text = '{0}'.format( text )
        elif type(mapped) is str:
            text = mapped
        else:
            raise RuntimeError( "unexpected type for classmap elements: "+
                                type(mapped) )
    return text

def getPrintableFileName( substrate ):
    name = '??'
    if isinstance(substrate, cvac.ImageSubstrate):
        name = substrate.path.filename
    elif isinstance(substrate, cvac.VideoSubstrate):
        name = ''
        if substrate.videopath:
            name = substrate.videopath.filename
        if substrate.framepaths:
            minkey = min( substrate.framepaths.keys() )
            maxkey = max( substrate.framepaths.keys() )
            name = name + substrate.framepaths[minkey].filename \
                 + " [" + str(minkey) + ".." + str(maxkey) + "]"
        else:
            # raise RuntimeError("neither videopath nor framepaths specified")
            # todo: should it be policy that if there is a substrate it should be correct?
            name = '(incorrect substrate)'
    else:
        raise RuntimeError("unknown Substrate type: "+type(substrate))
    return name

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
            for key in list(foundMap.keys()):
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

    numres = len( results )
    print('received a total of {0} results{1}'\
          .format( numres, (":",".")[numres==1] ))
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
        printname = getPrintableFileName( res.original.sub )
        print("result for {0} ({1}): found {2} label{3}: {4}".format(
            printname, origname,
            numfound, ("s","")[numfound==1], ', '.join(names) ))
        if numfound==1 and origname.lower()==names[0].lower():
            identical += 1
    if foundMap and origMap:
        print('{0} out of {1} results had identical purposes'
              .format( identical, len( results ) ))
    else: 
        #print('(labels had unknown purposes, cannot determine result accuracy)')
        pass

def getLocationString( location ):
    if isinstance( location, cvac.Point2D ):
        return str(location.x) + ", " + str(location.y)
    elif isinstance( location, cvac.PreciseLocation ):
        return str(location.centerX) + ", " + str(location.centerY)
    else:
        raise RuntimeError("need to pretty-print " + type(location))
                
def printLabeledTrack( track ):
    '''
    print out the frame numbers and track locations.
    input argument can be a ResultSet ... LabeledTrack.
    '''
    if isinstance( track, cvac.ResultSet ):
        for res in track:
            printLabeledTrack( res.foundLabels )
        return
    if isinstance( track, cvac.Result ):
        printLabeledTrack( track.foundLabels )
        return
    if isinstance( track, list ):
        for lbl in track:
            printLabeledTrack( lbl )
        return
    if not isinstance( track, cvac.LabeledTrack ):
        # ignore silently
        return
    for floc in track.keyframesLocations:
        locstr = getLocationString( floc.loc )
        print( "{0} ({1}): {2}".format( floc.frame.framecnt,
                                        floc.frame.time,
                                        locstr ))
'''  
def initGraphics(title = "cvac results"):
    try:
      
        #wnd = tk.Tk()
        #wnd.title(title)
    except:
        wnd = None
        raise RuntimeError("cannot display images - do you have PIL installed?")
    return wnd
'''


def showROCPlot(ptList):
    '''
    Plot image is 300x200 with 10 pixel space around the plot so the
    plot area is 280x180. So each 1/10th is 18 pixels in height and 28 pixels in width.
    Plot 0,0 is pixel 10, 190
    '''
    xoffset = 10
    yoffset = 10
    #wnd = initGraphics(title="ROC Curve Plot")
    #wnd = guithread.getCanvas()
    #if wnd == None:
    #    return
    if guiqueue == None: 
         return
    try:
        im = Image.open(getFSPath('plot.jpg'))
    except:
        raise RuntimeError("Cannot open ROC plot background image plot.jpg")
    width, height = im.size
    #ImbImage = tk.Canvas(wnd, highlightthickness=0, bd=0, bg='red', width=width, height=height)
    #ImbImage.pack()
    draw = ImageDraw.Draw(im)
    for pt in ptList:
        # Scale over plot region assume plot is all but offset above and below
        # x value will be precision = TP / (TP + FP)
        # y value will be recall (hit rate) = TP ? (TP + FN)
        poscnt = pt.res.tp + pt.res.fp
        if poscnt == 0:
            precision = 0
        else:
            precision = old_div(pt.res.tp, float(poscnt))
        x = int(precision*(width - (2 * xoffset)))
        y = int(pt.recall*(height - (2 * yoffset)))
        x = x + xoffset
        y = y + yoffset
        #convert to origin in upper left
        y = height - y -1
        draw.ellipse((x-2, y-2, x+2, y+2), fill="black")

        
    del draw
    # Draw to a new window the imgWindow
    #Since we are not updating the window we don't 
    guiqueue.startWindow(im)
   
'''
Draw the results to the screen.  If you also want to see the results and the
labled data together then pass the origSet as the runset containing the labeled data.
'''
def drawResults( results, origSet=None, multiWindow=True ):
    if not results:
        print("no results, nothing to draw")
        return
    
    # first, collect all image substrates of found labels;
    # if the foundLabel doesn't have a path, use the path from
    # the original
    substrates = collectSubstrates( results )
    if not substrates:
        print("no labels and/or no substrates, nothing to draw")
        return
    # print out some summary information
    sys.stdout.softspace=False

    for subpath in iter(list(substrates.keys())):
        numlabels = len( substrates[subpath] )
        print("{0} ({1} label{2})".format( subpath, numlabels, ("s","")[numlabels==1] ))
        
    origSubs = {}    
    if origSet:
        for plist in origSet.purposedLists:
            assert( isinstance(plist, cvac.PurposedLabelableSeq) )
            for sample in plist.labeledArtifacts: 
                opath = getFSPath(sample)
                if opath not in origSubs:
                    origSubs[opath] = [sample]
                else:
                    origSubs[opath].append(sample)
    if origSubs:  
        # render the substrates with respective labels
        showImagesWithLabels( substrates, origSubs = origSubs, multiWindow=multiWindow )
    else:
        showImagesWithLabels( substrates, multiWindow=multiWindow )
        
def drawLabelables( lablist, maxsize=None, multiWindow=True ):
    # first, collect all image substrates of the labels
    substrates = {}
    num_videos = 0
    for lbl in lablist:
        if lbl.sub.isImage:
            subpath = getFSPath( lbl.sub )
            if subpath in substrates:
                substrates[subpath].append( lbl )
            else:
                substrates[subpath] = [lbl]
        elif lbl.sub.isVideo:
            num_videos += 1
        else:
            raise RuntimeError("Unknown substrate type")
    if num_videos>0:
        print("not drawing {0} video annotation{1}"
              .format(num_videos, ("s","")[num_videos==1]))
    if not substrates:
        print("no labels and/or no substrates, nothing to draw")
        return
    showImagesWithLabels( substrates, maxsize, multiWindow=multiWindow )
    
def showLocation(loc, img, scale, color=255):
    if isinstance( loc, cvac.BBox):
        a = old_div(loc.x,scale)
        b = old_div(loc.y,scale)
        c = a+old_div(loc.width,scale)
        d = b+old_div(loc.height,scale)
        draw = ImageDraw.Draw( img )
        draw.line([(a,b), (c,b), (c,d), (a,d), (a,b)], fill=color, width=0)
        del draw
    elif isinstance( loc, cvac.Silhouette):
        if len(loc.points) is 0:
            return
        if len(loc.points) is 1:
            raise RuntimeError("Incorrect Labelable: a single point should not be a silhouette")
        cpts = []
        for point in loc.points:
            cpts.append( (old_div(point.x,scale), old_div(point.y,scale)) )
        cpts.append( cpts[0] )  # close the loop
        draw = ImageDraw.Draw( img )
        draw.line( cpts, fill=color, width=0 )
        del draw
    else:
        print("warning: not rendering Label type {0}".format( type(loc) ))  
    

def showImagesWithLabels( substrates, origSubs = None, maxsize=None, multiWindow=True ):
    '''Takes a dictionary of type dict[file_system_path] = [Labelable] as
    input and renders every image with labels overlaid.  The size
    parameter can be used to display all images at the same size.
    If origSubs dictionary is not None the draw those labels too.
    They are the original labels and drawn in green.
    '''
    global guiqueue
    if guiqueue == None:
        return
    # now draw
    keycnt = len(substrates)
    cnt = 0
    for subpath in iter(list(substrates.keys())):
    #for subpath in sorted( substrates.keys() ):
        img = Image.open( subpath )
        scale = 1.0
        if not maxsize is None:
            scalex = old_div(float(img.size[0]),float(maxsize[0]))
            scaley = old_div(float(img.size[1]),float(maxsize[1]))
            scale = max( scalex, scaley )
            maxsize = int(old_div(img.size[0],scale)), int(old_div(img.size[1],scale))
            img = img.resize( maxsize, Image.NEAREST ) # favor speed over quality
        ''' If we have the original labels draw them first so they are on the buttom. '''
        if origSubs:
            if subpath in origSubs:
                for lbl in origSubs[subpath]:
                    if isinstance(lbl, cvac.LabeledLocation):
                        showLocation(lbl.loc, img, scale, color=0xff00)
                    elif isinstance(lbl, cvac.LabeledTrack):
                        for frame in lbl.keyframesLocations:
                            showLocation(frame.loc, img, scale, color=0xff00)
                    
        for lbl in substrates[subpath]:
            # draw poly into the image
            if isinstance(lbl, cvac.LabeledLocation):
                showLocation(lbl.loc, img, scale)
            elif isinstance(lbl, cvac.LabeledTrack):
                for frame in lbl.keyframesLocations:
                    showLocation(frame.loc, img, scale)
        cnt = cnt + 1
        if multiWindow:
            #The last window needs to be the main window otherwise we
            #end wihout displaying any windows
            if cnt < keycnt:
                guiqueue.startWindow(img)
            else:
                guiqueue.imgWindow(img)
        else:
            guiqueue.imgWindow(img)

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
    for sub in list(substrates.keys()):
        lbl = getHighestConfidenceLabel( substrates[sub] )
        if lbl:
            # create directory and symlink according to found label

            if not lbl.lab.hasLabel:
                print("No Label for " + sub)
                continue
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
