#
# Easy!  mini tutorial
#
# Invoke a remote service, send files, receive files, receive messages
#
# matz 6/17/2013

import easy

#host = "-h localhost"
host = "-h vision.nps.edu"

# obtain a reference to a Bag of Words (BOW) detector,
# running on a remote machine
detector = easy.getDetector( "bowTest:default -p 10104 "+ host )

#
# create a corpus from corporate logo images
#
corpus = easy.openCorpus( "corporate_logos" );
categories, lablist = easy.getDataSet( corpus )
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


# a model for distinguishing Canadian, Korean, and US flags,
# trained previously with a BOW-specific trainer and stored
# in a file
modelfile = "detectors/bowUSKOCA.zip"

# apply the detector type, using the model and testing the imgfile
results = easy.detect( detector, modelfile, runset )

#
# Make sure all files in the RunSet are available on the remote site;
# it is the client's responsibility to upload them if not.
#
fileserver = easy.getFileServer( "FileService:default -p 10110 " + host )
putResult = easy.putAllFiles( fileserver, runset )

#
# Connect to a trainer service, train on the RunSet
#
trainer = easy.getTrainer( "bowTrain:default -p 10103 " + host)
trainedModel = easy.train( trainer, runset )
print("Training model stored in file: " + easy.getFSPath( trainedModel.file ))

# this will fail until BOW zips its results
easy.getFile( fileserver, trainedModel )
