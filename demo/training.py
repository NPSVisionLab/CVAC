#
# Easy!  mini tutorial
#
# Build a model for object detection
#
# matz 6/18/2013

import easy
import zipfile

#
# create a RunSet from corporate logo images
#
print("==== Training runset: ====")
trainset = easy.createRunSet( "trainImg" );
easy.printRunSetInfo( trainset )

#
# Connect to the trainer for a Bag of Words algorithm, then
# train with the given runset
#
trainer = easy.getTrainer( "bowTrain:default -p 10103 ")
trainedModel = easy.train( trainer, trainset )
zipfname = easy.getFSPath( trainedModel )
print("Training algorithm produced a model in file {0}.".\
      format( zipfname ) )

#
# test the trained model on a separate set of images
#
print("==== Test runset: ====")
testset = easy.createRunSet( "testImg" )
easy.printRunSetInfo( testset )
detector = easy.getDetector( "bowTest:default -p 10104" )
results = easy.detect( detector, trainedModel, testset )
print("==== Results: ====")
easy.printResults( results )


quit()
# This fails because bow does not return a zip file
zipf = zipfile.ZipFile( zipfname )
print zipf.namelist()
print("Training model stored in file {0}; contents:\n{1}".\
      format( zipfname, zipf.namelist()))
