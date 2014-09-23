'''
Easy!  mini tutorial
Evaluate the accuracy of a shot boundary detector
matz 8/2014
'''

#
# Example 1: create a test set from VOC-annotated videos
#
corpus = easy.openCorpus( "corpus/PascalVOC2012.properties" )
categories, lablist = easy.getDataSet( corpus )
shots_truth = easy.createRunSet( categories['shot boundary'], "pos" )
shots_test = easy.removeLabels( shots_truth )

#
# define a custom matching function that compares frame numbers
# and reports a match if the boundary was found within 5 frames
# of the known truth
#
def matchFun( truth, evaluate ):
    # assume type cvac.LabeledVideoSegment
    diff = truth.start.framecnt-evaluate.start.framecnt
    return (abs(diff) < 5)

#
# evaluate your shot boundary detection algorithm
#
sbd = easy.getDetector( "SBD_Detector:default -p 10198" )
shotModel = "detectors/SBDdefaultModel.zip"
shots_result = easy.detect( sbd, shotModel, shots_test )
confmat = easy.getConfusionMatrix( shots_truth, shots_result,\
                                   match=matchFun )
easy.printConfusionMatrix( confmat )

