#
# Easy!  mini tutorial
#
# Obtain labeled data from a LabelMe server.
# See labelme.mit.edu
#
# matz 6/19/2013

import easy

corpus = easy.openCorpus( "corpus/AirCraftCarriers.properties" )
# corpus = easy.openCorpus( "corpus/ern_LabelMe.properties" )
categories, lablist = easy.getDataSet( corpus, createMirror=True )
print('Obtained {0} labeled artifact{1} from corpus "{2}":'.format(
    len(lablist), ("s","")[len(lablist)==1], corpus.name ));
easy.printCategoryInfo( categories )

#maxsize=1280, 1280
maxsize=512, 512
easy.drawLabelables( lablist, maxsize )


# caching of Labelables in CorpusServer, currently needs to re-mirror if CorpusServer is restarted
