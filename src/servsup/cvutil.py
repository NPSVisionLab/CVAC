'''
Created on Oct 6, 2015

@author: tomb
'''

import cv2
import sys
import subprocess
import tempfile
import shutil
import os

def loadImage(filepath):
    import pdb
    pdb.set_trace()
    #make sure all the backslashes are gone
    filepath = filepath.replace("\\", '/')
    print(filepath)
    img_src_rgb = None
    try:
        img_src_rgb = cv2.imread(filepath, cv2.IMREAD_COLOR)
    except Exception as e:
        print("Error: " + filepath + " is not readable. ")
    if img_src_rgb == None:
        ''' OpenCV can't read the file so try and convert it to png
        '''
        basename = os.path.basename(filepath)
        newbasename, ext = os.path.splitext(basename)
        ''' For some reason only converting to bmp really converts the other formats
            changes the suffix for identify still shows it as a gif.
        '''
        newbasename = newbasename + ".bmp"
        tempdir = tempfile.mkdtemp()
        filepath2 = os.path.join(tempdir, newbasename)
        #print("calling convert on {0}".format(filepath))    
        ''' Windows fails on a relative call to convert so renamed program to iconvert
        '''
        try:
            res = subprocess.call("iconvert {0} {1}".format(filepath, filepath2, shell = True))    
            #print ("Result is {0}".format(res))
            if res != 0:
                print("Error: " + filepath + " could not be converted from .gif.")    
            try:
                img_src_rgb = cv2.imread(filepath2, cv2.CV_LOAD_IMAGE_COLOR)
                if img_src_rgb == None:
                    print("could not load file {0}".format(filepath2))
            except:
                print("Error: " + filepath + " is not readable.") 
        except:
            print("Could not convert with iconvert")
        shutil.rmtree(tempdir)
    return img_src_rgb    