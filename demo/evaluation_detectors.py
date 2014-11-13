'''
Easy!  mini tutorial
matz 11/13/2014

Determine the accuracy performance of a full-image detector.
To evaluate your data set, put it into subfolders of CVAC_DataDir,
with positive and negative data in separate folders.  The names
of these folders will be picked up as labels by the RunSet methods.
Assign purposes in the addToRunSet method calls.

Similarly, the detector will report a label, but leave it to you to
decide on the purpose of such labels. Hence, the "found" labels
need to be mapped to their purposes in the foundMap.
Note that different methods will return different labels, so the
foundMap needs to be specific to the method.

Note that the particular detectors do not match the data, so the
code is illustrative, but the results are not very interesting.
'''

import easy, cvac
import easy.evaluate as ev

runset = cvac.RunSet()
easy.addToRunSet( runset, "trainImg/ca", "neg" )
easy.addToRunSet( runset, "trainImg/kr", "pos" )
#easy.printRunSetInfo( runset, printArtifacts=False, printLabels=True )

contenders = []

# Bag of Words
if (easy.getDetector("BOW_Detector")==None):
    print("BOW detector service is insufficiently configured, skipping.")
else:
    c = ev.Contender("BOW")
    c.detectorString = "BOW_Detector"
    c.detectorData = "detectors/bowUSKOCA.zip"
    c.foundMap = { 'kr':easy.getPurpose('pos'),
                   'ca':easy.getPurpose('neg'),
                   'us':easy.getPurpose('neg'),
                   'unlabeled':easy.getPurpose('neg')}
    contenders.append( c );

# OpenCV Cascade detector
if (easy.getDetector("OpenCVCascadeDetector")==None):
    print("OpenCVCascadeDetector service is insufficiently configured, skipping.")
else:
    c = ev.Contender("Faces")
    c.detectorString = "OpenCVCascadeDetector"
    c.detectorData = "detectors/OpencvFaces.zip"
    c.foundMap = { 'positive':easy.getPurpose('pos'),
                   'negative':easy.getPurpose('neg')}
    contenders.append( c );

perfdata = ev.joust( contenders, runset )
ev.printEvaluationResults(perfdata[0])
