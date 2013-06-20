#
# Easy!  mini tutorial
#
# Obtain labeled data from a LabelMe server.
# See http://new-labelme.csail.mit.edu/Release3.0
#
# matz 6/19/2013

import easy

# The properties file for a LabelMe Corpus contains all pertinent information.
# Take a look at corpus/LabelMeCarsTest.properties and pay particular
# attention to the following properties:
# LMFolders and LMObjectNames
corpus = easy.openCorpus( "corpus/LabelMeCarsTest.properties" )
categories, lablist = easy.getDataSet( corpus, createMirror=True )
print('Obtained {0} labeled artifact{1} from corpus "{2}":'.format(
    len(lablist), ("s","")[len(lablist)==1], corpus.name ));
easy.printCategoryInfo( categories )

# draw the images and their annotations, one image at a time,
# at a given maximum size (width, height)
#easy.drawLabelables( lablist, (512, 512) )
print("-----------")

# pick a subset: all license plates
license_plates = categories['license plate']
print("There are {0} license plate labels.".format( len(license_plates) ))

# another subset: all labels starting with "car"
cars = []
for key in categories.keys():
    if key.startswith("car"):
        cars.append( categories[key] )
print("There are {0} car-related labels.".format( len(cars) ))

# Train a detector on license plates
trainer = easy.getTrainer( "bowTrain:default -p 10103 ")
trainset = easy.createRunSet( license_plates, "pos" )
easy.printRunSetInfo( trainset )
licenseplateModel = easy.train( trainer, trainset )

# test the license plate detector on the known locations of cars;
# this will only try to detect within the boundaries of cars.
testset = easy.createRunSet( cars, "unpurposed" )
detector = easy.getDetector( "bowTest:default -p 10104" )
results = easy.detect( detector, licenseplateModel, testset )

printResults( results )

# Note that Labels are cached in the CorpusServer, but the corpus currently
# needs to re-mirror if the CorpusServer is restarted because Labels are
# not stored to disk.  Images are stored to disk.
