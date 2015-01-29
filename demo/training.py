'''
Easy!  mini tutorial
Build a model for object detection
matz 6/18/2013
'''
import cvac
import easy
import zipfile

#
# create a RunSet from corporate logo images
#
print("==== Training runset: ====")
#trainset = easy.createRunSet( "trainImg" )
trainset = cvac.RunSet()
caset = easy.createRunSet("trainImg/ca")
krset = easy.createRunSet("trainImg/kr")
usset = easy.createRunSet("trainImg/us")
easy.addToRunSet(trainset, caset, cvac.Purpose(cvac.PurposeType.MULTICLASS,0))
easy.addToRunSet(trainset, krset, cvac.Purpose(cvac.PurposeType.MULTICLASS,1))
easy.addToRunSet(trainset, usset, cvac.Purpose(cvac.PurposeType.MULTICLASS,2))
easy.printRunSetInfo( trainset, printLabels=True )

#
# Connect to the trainer for a Bag of Words algorithm, then
# train with the given runset
#
print("starting training, this might take a few minutes...")
trainer = easy.getTrainer( "BOW_Trainer")
trainedModel = easy.train( trainer, trainset )

#
# Display information about the file in which the model is stored;
# this is generally no concern to algorithm users and only of
# interest to algorithm developers since it is algorithm-specific
#
zipfname = easy.getFSPath( trainedModel )
print("{0}".format( zipfname ))
zipf = zipfile.ZipFile( zipfname )
print("Training model stored in file {0}".format( zipfname))
# print("file contents:\n{0}".format(zipf.namelist()))

#
# test the trained model on a separate set of images
#
print("==== Test runset: ====")
testset = easy.createRunSet( "testImg" )
easy.printRunSetInfo( testset, printLabels=True )
detector = easy.getDetector( "BOW_Detector" )
results = easy.detect( detector, trainedModel, testset )
print("==== Results: ====")
easy.printResults( results )

