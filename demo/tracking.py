'''
Easy!  mini tutorial
Evaluate the accuracy of a pedestrian tracker
matz 9/2014
'''

#
# create a test set from Vatic-annotated videos
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
