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

# first some detections on a pre-built model
detector = easy.getDetector( "OpenCVCascadeDetector:default -p 10102" )
results = easy.detect( detector, "detectors/haarcascade_frontalface_alt.xml", "testImg/italia.jpg" )
#results = easy.detect( detector, "detectors/haarcascade_frontalface_alt.xml", runset )
easy.printResults( results )
easy.drawResults( results )

print("not training yet")
quit()

# face corpus
corpus = easy.openCorpus( "corpus/Faces94.properties" )

categories, lablist = easy.getDataSet( corpus, createMirror=True )
print('Obtained {0} labeled artifact{1} from corpus "{2}":'.format(
    len(lablist), ("s","")[len(lablist)==1], corpus.name ));
easy.printCategoryInfo( categories )
# easy.printSubstrateInfo( lablist )

res = easy.createRunSet( categories )
runset = res['runset']
classmap = res['classmap']

