
from __future__ import print_function

'''
    Misc utility functions. 
    Some for handling directories and files into labelables
'''

import os
import cvac


def getImageExtensions():
    return ["bmp","dib","jpeg","jpg","jpe","jp2", \
            "png","pbm","pgm","ppm","sr","ras", \
            "tiff","tif","gif","giff","emf","pct", \
            "pcx","pic","pix","vga","wmf"];

def getVideoExtensions():
    return ["mpg","mpeg","avi","wmv","wmp","wm", \
            "asf","mpe","m1v","m2v","mpv2","mp2v", \
            "dat","ts","tp","tpr","trp","vob","ifo", \
            "ogm","ogv","mp4","m4v","m4p","m4b","3gp", \
            "3gpp","3g2","3gp2","mkv","rm","ram", \
            "rmvb","rpm","flv","swf","mov","qt", \
            "amr","nsv","dpg","m2ts","m2t","mts", \
            "k3g","skm","evo","nsr","amv","divx","webm"];

'''
Return the directory name with the prefix (usually CVAC_DataDir) stripped off
if its appended to the front of the directory.
'''
def strip_path_prefix(mydir, prefix):
    # make both prefix (usually CVAC_DataDir) and mydir absolute and then strip off
    absprefix = os.path.abspath(prefix)
    absdir = os.path.abspath(mydir)
    labsdir = absdir.lower()
    if labsdir.startswith(absprefix.lower()):
        absdir = absdir[len(absprefix + '/'):]
        return absdir
    return mydir   # Noth9ing to strip so return original

'''
Return the FilePath with the prefix (usually CVAC_DataDir) stripped off
if its appended to the front of the directory.
'''
def strip_path_prefix_from_FilePath(mypath, prefix):
    mydir = os.path.join(mypath.directory.relativePath,mypath.filename);
    resdir = strip_path_prefix(mydir, prefix)
    if resdir != mydir:
        mypath.directory.relativePath = os.path.dirname(resdir)
    return mypath

'''
  Add a labelable to the set for the given file.  Use the last directory as
  the label name ane previous directories as labelproperties
'''
def addFileToLabelableSet(lset, ldir, lfile, relative_to_path, video=True, image=True):
    isVideo = False
    isImage = False
    if '.' not in lfile:
        return  # No extension so don't add it
    #see if we have a video or image file if not just skip it
    name, ext = lfile.rsplit('.',1)
    extLower = ext.lower()
    if extLower in getVideoExtensions():
        isVideo = True
    elif extLower in getImageExtensions():
        isImage = True
    else:
        return
    if isVideo and video == False:
        return
    if isImage and image == False:
        return
    # strip off relative_to_path (usually cvac data dir)
    ldir = strip_path_prefix(ldir, relative_to_path)
    
    props = {}
    # last directory is the label name
    ldir = os.path.normpath(ldir)
    ldir = ldir.rstrip('\\')
    ldir = ldir.rstrip('/')
    labelName = os.path.basename(ldir)
    # FIll in props with each directory
    nextDir = os.path.dirname(ldir)
    while nextDir != None and nextDir != "":
        nextProp = os.path.basename(nextDir)
        if nextProp != None and nextProp != "":
            props[nextProp] = ""
        nextd = os.path.dirname(nextDir)
        if nextd == nextDir:
            break
        else:
            nextDir = nextd
    dirpath = cvac.DirectoryPath(ldir)
    fpath = cvac.FilePath(dirpath,lfile)
    if isVideo == True:
        sub = cvac.VideoSubstrate()
        sub.width = 0
        sub.height = 0
        sub.videopath = fpath
    else:
        sub = cvac.ImageSubstrate()
        sub.width = 0
        sub.height = 0
        sub.path = fpath
    lab = cvac.Label(True, labelName, props, cvac.Semantics(""))
    lset.append(cvac.Labelable(0.0, lab, sub))
    
'''
    Search a directory optionally recursively adding the correct 
    file types to a labelable set
'''
def searchDir(lset, ldir, relative_to_path, recursive=True, video=True, image=True):
    flist = os.listdir(ldir)
    for f in flist:
        if os.path.isdir(ldir + '/' + f) and recursive:
            searchDir(lset, ldir + '/' + f, relative_to_path)
        else:
            addFileToLabelableSet(lset, ldir, f, relative_to_path, video, image)

def getLabelableFilePath(lab):
    #if lab.sub.ice_isA('::cvac::ImageSubstrate'):
    if isinstance(lab.sub, cvac.ImageSubstrate):
        path = lab.sub.path
    else:
        path = lab.sub.videopath
    return path
        
