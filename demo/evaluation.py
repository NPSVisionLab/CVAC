'''
Easy!  mini tutorial
Evaluate the accuracy of a shot boundary detector and a pedestrian tracker
matz 6/16/2013
'''
# Note: some of these Easy! functions are in alpha and
# planned for beta release v0.5

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

#
# Example 2: create a test set from Vatic-annotated videos
#
corpus = easy.openCorpus( "corpus/MallSurveillance.properties" )
categories, lablist = easy.getDataSet( corpus )
peds_truth = easy.createRunSet( categories['pedestrians'], "pos" )
peds_test = easy.removeLabels( peds_truth )

#
# evaluate your tracking algorithm with a common match scoring method
#
tracker = easy.getDetector( "myTracker:default -p 10199" )
trackerModel = "trackers/IndoorTrackingModel.zip"
peds_results = easy.detect( tracker, trackerModel, peds_test )
tracker_eval = easy.evaluate( truth=peds_truth, evaluate=peds_results, \
                              match="overlap" )
print("Summary tracking score:\n{0}".format( tracker_eval['statistics'] ) )
