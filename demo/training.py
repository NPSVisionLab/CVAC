#
# Easy!  mini tutorial
#
# Build a model for object detection
#
# matz 6/18/2013

import easy
import zipfile

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
easy.printRunsetInfo( runset )

#
# Connect to the trainer for a Bag of Words algorithm, then
# train with the given runset
#
trainer = easy.getTrainer( "bowTrain:default -p 10103 ")
trainedModel = easy.train( trainer, runset )
zipfname = easy.getFSPath( trainedModel )
zipf = zipfile.ZipFile( zipfname )
print zipf.namelist()
print("Training model stored in file {0}; contents:\n{1}".\
      format( zipfname, zipf.namelist()))
