'''
Easy!  mini tutorial
Compare the performance of several algorithms
matz 6/16/2013
'''
# Note: some of these Easy! functions are in alpha and
# planned for beta revision v0.5

#
# Create a test set from a pre-configured Corpus (a subset of
# IMAGENET, some categories from the "natural object" sub-tree)
#
corpus = easy.openCorpus( "corpus/ImageNet_natural_object.properties" )
truth = easy.createRunSet( corpus )
testset = easy.removeLabels( truth )

#
# Some of the competing algorithms are installed at the researchers' sites
# and some are supplied on a virtual machine that you have started up locally.
# One of the algorithms has a hard-coded "model," most refer to a model, and
# one has two alternative models.
# Note that none of the algorithms have to be built locally, and that they
# can be implemented in any language and on any platform.
#
competitors = {}
det = easy.getDetector( "detector:default -p 10200 -h cvlab.uni1.edu" ) # remote, uni1
mod = "detectors/cvlabmodel.zip"  # this is stored at the remote site
competitors['uni1'] = (det, mod)

det = easy.getDetector( "detector:default -p 10123 -h ai.uni2.edu" ) # remote, uni2
mod = ""  # model hardcoded, no model required during initialization
competitors['uni2'] = (det, mod)

det = easy.getDetector( "detector:default -p 22223 -h localhost" )  # running on VM
mod = "detectors/outdoor_model.zip"  # the first of two models, same algorithm
competitors['vm_outdoor'] = (det, mod)

det = easy.getDetector( "detector:default -p 22223 -h localhost" )  # running on VM
mod = "detectors/indoor_model.zip"  # the second of two models, same algorithm
competitors['vm_indoor'] = (det, mod)

#
# run the algorithms.  Easy! permits asynchronous, parallel execution, but this is not shown here.
#
results  = {}
confmats = {}
scores   = {}
for comp in competitors.keys():
    try:
        det, mod = competitors[comp]
        results[comp]  = easy.detect( det, mod, testset )
        confmats[comp] = easy.getConfusionMatrix( results[comp], truth,\
                                                 match="highestscore" )
        # You could also implement a method that scores a hit if the
        # true label is within the n highest-ranked found labels
        scores[comp]   = numpy.trace( confmats[comp] )
    except:
        results[comp] = "did not finish"

# print the results
for comp in competitors.keys():
    if comp in scores:
        print( "competitor {0} number of correct classifications: {1}".\
               format( comp, scores[comp] ) )
    else:
        print( "competitor {0} did not finish.".format( comp ))
        
