
"""
Template for the CS4330 Recognition module.

"""

import easy
import cvac

# Replace the objname with your object's label.  Note that the
# corpus_fname file name is relative to the CVAC_DataDir which
# is set in config.icebox, config.client, and config.service, and
# which defaults to your installation directory /data.
objname = 'apple'
corpus_fname = 'corpus/labelme_'+objname+'.properties'


# The properties file for a LabelMe Corpus contains all pertinent information.
# Take a look at corpus/LabelMeCarsTest.properties and pay particular
# attention to the following properties:
# LMFolders and LMObjectNames
cs = easy.getCorpusServer( "PythonCorpusService:default -p 10021")
corpus = easy.openCorpus( corpus_fname, corpusServer=cs )
categories, lablist = easy.getDataSet( corpus, corpusServer=cs )
print('Obtained {0} labeled artifact{1} from corpus "{2}":'.format(
    len(lablist), ("s","")[len(lablist)==1], corpus.name ));
easy.printCategoryInfo( categories )

# if desired, you can draw the images and their annotations,
# one image at a time, at a given maximum size (width, height)
#easy.drawLabelables( lablist, (512, 512) )

print("==== Training runset: ====")
posPurpose = easy.getPurpose('pos')
negPurpose = easy.getPurpose('neg')
trainset = cvac.RunSet()
easy.addToRunSet(trainset, categories['apple'], posPurpose);
#trainset = easy.createRunSet( categories, purpose=posPurpose );
#easy.printRunSetInfo( trainset, printLabels=True )
easy.printRunSetInfo( trainset )

#
# Connect to the trainer for a Bag of Words algorithm, then
# train with the given runset
#
#trainer = easy.getTrainer( "BOW_Trainer")
trainer = easy.getTrainer( "BOW_Trainer:default -p 10103")
trainedModel = easy.train( trainer, trainset )
zipfname = easy.getFSPath( trainedModel )
print("Training model stored in file {0}".format( zipfname ))

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


