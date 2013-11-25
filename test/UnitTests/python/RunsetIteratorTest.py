#
# A demo for the Easy Computer Vision library.
#
from __future__ import print_function
import sys, os
thisPath = os.path.dirname(os.path.abspath(__file__))
srcPath = os.path.abspath(thisPath+"/../../lib/python")
sys.path.append(srcPath)

import cvac
import easy

runset = cvac.RunSet([])
 
singleImages = cvac.PurposedLabelableSeq(cvac.Purpose(cvac.PurposeType.MULTICLASS,1))
singleImages.labeledArtifacts = []
runset.purposedLists.append(singleImages)
 
tlab = cvac.Label(False,'')
tsub = cvac.Substrate(True,False,cvac.FilePath(cvac.DirectoryPath("trainImg/us"),"US001.jpg"),0,0)                        
singleImages.labeledArtifacts.append(cvac.Labelable(0,tlab,tsub))
 
tlab = cvac.Label(False,'')
tsub = cvac.Substrate(True,False,cvac.FilePath(cvac.DirectoryPath("trainImg/kr"),"Kr001.jpg"),0,0)                        
singleImages.labeledArtifacts.append(cvac.Labelable(0,tlab,tsub))
 
 

# Ex. for image files in a directory 
tFileSuffixes = ["jpg","bmp"]
directory_images = cvac.PurposedDirectory(cvac.Purpose(cvac.PurposeType.MULTICLASS,1),
                                          cvac.DirectoryPath("testImg"),
                                          tFileSuffixes,
                                          1)
runset.purposedLists.append(directory_images)
 

#===============================================================================
# # If you have video files, you may comment out this part for testing the video files
# # Ex. for video files in a directory 
# tFileSuffixes = ["mpg","mpeg","avi"]
# directory_videos = cvac.PurposedDirectory(cvac.Purpose(cvac.PurposeType.MULTICLASS,1),
#                                           cvac.DirectoryPath("trainVideo"),
#                                           tFileSuffixes,
#                                           1)
# runset.purposedLists.append(directory_videos)
#===============================================================================
 
host = "-h localhost"
detector = easy.getDetector( "RSItrTest_Detector:default -p 10109 " + host )
detectorData = cvac.DetectorData()  #empty detectorData only for testing
results = easy.detect( detector, detectorData, runset )
#results = easy.detect( detector, trainedModel, runset )
#easy.printResults( results, foundMap=classmap )
easy.printResults( results)

quit()
