'''
Easy!  mini tutorial
Obtain labeled data from a LabelMe server.
See http://new-labelme.csail.mit.edu/Release3.0
matz 6/19/2013
'''

import easy

# The properties file for a LabelMe Corpus contains all pertinent information.
# Take a look at corpus/LabelMeCarsTest.properties and pay particular
# attention to the following properties:
# LMFolders and LMObjectNames
corpus = easy.openCorpus( "corpus/LabelMeCarsTest.properties" )
categories, lablist = easy.getDataSet( corpus, createMirror=True )
if (len(lablist)==0):
    print("No labelable objects found for the data set.  Note that\n"
          "the Python implementation of the FileServer currently requires\n"
          "all LabelMe files to be local, rather than on a server.\n"
          "Please either download all image and annotation files first,\n"
          "or run the Java implementation of the FileServer.")
    exit(-1)
print('Obtained {0} labeled artifact{1} from corpus "{2}":'.format(
    len(lablist), ("s","")[len(lablist)==1], corpus.name ));
easy.printCategoryInfo( categories )

# draw the images and their annotations, one image at a time,
# at a given maximum size (width, height)
easy.drawLabelables( lablist, (512, 512) )
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

# Note that Labels are cached in the CorpusServer, but the corpus currently
# needs to re-mirror if the CorpusServer is restarted because Labels are
# not stored to disk.  Images are stored to disk.

quit()  # done for now

# Train a detector on license plates
trainer = easy.getTrainer( "BOW_Trainer:default -p 10103 ")
trainset = easy.createRunSet( license_plates, "pos" )
easy.printRunSetInfo( trainset )
licenseplateModel = easy.train( trainer, trainset )

# test the license plate detector on the known locations of cars;
# this will only try to detect within the boundaries of cars.
testset = easy.createRunSet( cars, "unpurposed" )
detector = easy.getDetector( "BOW_Detector:default -p 10104" )
results = easy.detect( detector, licenseplateModel, testset )

printResults( results )
