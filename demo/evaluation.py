'''
Easy!  mini tutorial
Compare the performance of several trainers and/or detectors
matz 11/26/2013
'''
import easy
import easy.evaluate as ev

# perform a comparative detector evaluation including
# training of a model
# easy.CVAC_ClientVerbosity = 4
posPurpose = easy.getPurpose('pos')
negPurpose = easy.getPurpose('neg')
contenders = []

# Bag of Words
if (easy.getTrainer("BOW_Trainer")==None):
    print("BOW service(s) are insufficiently configured, skipping.")
else:
    c = ev.Contender("BOW")
    c.trainerString = "BOW_Trainer"
    c.detectorString = "BOW_Detector"
    c.foundMap = {'1':posPurpose, '0':negPurpose}
    contenders.append( c );

# Histogram of Oriented Gradients
if (easy.getTrainer("HOG_Trainer")==None):
    print("HOG service(s) are insufficiently configured, skipping.")
else:
    c = ev.Contender("HOG")
    c.trainerString = "HOG_Trainer"
    c.detectorString = "HOGTest"
    c.foundMap = {'1':easy.getPurpose('pos')}
    contenders.append( c );

# Deformable Parts Model;
# currently, no trainer interface is available
if (easy.getTrainer("DPM_Detector")==None):
    print("DPM service(s) are insufficiently configured, skipping.")
else:
    c = ev.Contender("DPM")
    c.detectorString = "DPM_Detector"
    c.detectorData = "detectors/dpmStarbucksLogo.zip"
    c.foundMap = {'Positive':easy.getPurpose('pos'), 'Negative':easy.getPurpose('neg')}
    contenders.append( c );

# OpenCVCascade, with special settings for anticipated poor performance
if (easy.getTrainer("OpenCVCascadeTrainer")==None):
    print("Cascade service(s) are insufficiently configured, skipping.")
else:
    c = ev.Contender("cascade")
    c.trainerString = "OpenCVCascadeTrainer"
    c.detectorString = "OpenCVCascadeDetector"
    # c.foundMap = {'any':easy.getPurpose('pos')}
    c.foundMap = {'positive':posPurpose, 'negative':negPurpose}
    detector = easy.getDetector(c.detectorString)
    detectorProps = easy.getDetectorProperties(detector)
    c.detectorProps = detectorProps;
    c.detectorProps.props["maxRectangles"] = "200"
    c.detectorProps.minNeighbors = 0; # This prevents hang up in evaluator when training has too few samples
    contenders.append( c );

runset = easy.createRunSet( "trainImg/kr", "pos" )
easy.addToRunSet( runset, "trainImg/ca", "neg" )
easy.printRunSetInfo( runset, printArtifacts=False, printLabels=True )

perfdata = ev.joust( contenders, runset, folds=3 )
ev.printEvaluationResults(perfdata[0])

