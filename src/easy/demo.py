#
# A demo for the Easy Computer Vision library.
#
from __future__ import print_function
import sys, os

thisPath = os.path.dirname(os.path.abspath(__file__))
srcPath = os.path.abspath(thisPath+"/../../lib/python")
sys.path.append(srcPath)
import paths
import cvac

import easy


#
# First, a teaser for detection:
#
detector = easy.getDetector( "bowTest:default -p 10004" )
results = easy.detect( detector, "detectors/bowUSKOCA.zip", "testImg/TestCaFlag.jpg" )
easy.printResults( results )

#
# Second, a quick way to train a detector.  The resulting model
# can be used in place of the detector above.
#
# TODO: currently breaks because Caltech101 doesn't get extracted as expected
#categories, lablist = easy.getDataSet( "corpus/CvacCorpusTest" )
# categories, lablist = easy.getDataSet( "corpus/Caltech101.properties", createMirror=False )
#easy.printCategoryInfo( categories )
#runset = easy.createRunSet( categories["car_side"] )
#trainer = easy.getTrainer( "bowTrain:default -p 10003" )
#carSideModel = easy.train( trainer, runset )

#
# Third, a slower walk-through of functionality that digs a bit deeper.  All
# following steps are part of that.
# Obtain a set of labeled data from a Corpus,
# print dataset information about this corpus
#
cs = easy.getCorpusServer("CorpusServer:default -p 10011")
#corpus = easy.openCorpus( cs, "corpus/CvacCorpusTest.properties" )
#corpus = easy.openCorpus( cs, "corporate_logos" );
corpus = easy.openCorpus( cs, "trainImg" );
categories, lablist = easy.getDataSet( corpus, corpusServer=cs )
print('Obtained {0} labeled artifact{1} from corpus "{2}":'.format(
    len(lablist), ("s","")[len(lablist)==1], corpus.name ));
easy.printCategoryInfo( categories )

#
# add all samples from corpus to a RunSet,
# also obtain a mapping from class ID to label name
#
res = easy.createRunSet( categories )
runset = res['runset']
classmap = res['classmap']

#
# Make sure all files in the RunSet are available on the remote site;
# it is the client's responsibility to upload them if not.
#
host = "-h localhost"
#host = "-h vision.nps.edu"
fileserver = easy.getFileServer( "FileService:default -p 10110 " + host )
easy.putAllFiles( fileserver, runset )

#
# Connect to a trainer service, train on the RunSet
#
trainer = easy.getTrainer( "bowTrain:default -p 10103 " + host )
trainedModel = easy.train( trainer, runset )
print("Training model stored in file: " + easy.getFSPath( trainedModel.file ))

#
# Connect to a detector service,
# test on the training RunSet for validation purposes;
# The detect call takes the detector, the trained model, the
# runset, and a mapping from purpose to label name
#
detector = easy.getDetector( "bowTest:default -p 10104 " + host )
results = easy.detect( detector, trainedModel, runset )
easy.printResults( results, foundMap=classmap )

quit()
