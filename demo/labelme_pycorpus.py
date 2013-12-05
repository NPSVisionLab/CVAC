'''
Easy!  test for corpus_service.py
Obtain labeled data from a LabelMe server.
See http://new-labelme.csail.mit.edu/Release3.0
matz 6/19/2013
'''

import easy

# The properties file for a LabelMe Corpus contains all pertinent information.
# Take a look at corpus/LabelMeCarsTest.properties and pay particular
# attention to the following properties:
# LMFolders and LMObjectNames
cs = easy.getCorpusServer( "PythonCorpusService:default -p 10021")
corpus = easy.openCorpus( "corpus/LabelMeExample.properties", corpusServer=cs )
categories, lablist = easy.getDataSet( corpus, corpusServer=cs, 
                                       createMirror=True )
print('Obtained {0} labeled artifact{1} from corpus "{2}":'.format(
    len(lablist), ("s","")[len(lablist)==1], corpus.name ));
easy.printCategoryInfo( categories )

# draw the images and their annotations, one image at a time,
# at a given maximum size (width, height)
easy.drawLabelables( lablist, (512, 512) )
print("-----------")

