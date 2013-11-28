#
# high-level functionality to evaluate trainers and detectors
# Matz, July 2013
# last edit: November 2013
#
import easy

import random
random.seed()
import numpy as np

def evalResults( results, foundMap=None, origMap=None, inverseMap=False ):
    '''Determine true and false positives and negatives.
    Returns tp, fp, tn, fn'''
    
    tp = 0, fp = 0, tn = 0, fn = 0
    for res in results:
        names = []
        for lbl in res.foundLabels:
            foundLabel = getLabelText( lbl.lab, foundMap, guess=True )
            names.append(foundLabel)
        numfound = len(res.foundLabels)
        origname = getLabelText( res.original.lab, origMap, guess=False )
        if numfound==1 and origname.lower()==names[0].lower():
            # "true"
            if getPurpose( origname.lower() )==cvac.PurposeType.POSITIVE:
                tp += 1
            else:
                tn += 1
        else:
            # "false"
            if getPurpose( origname.lower() )==cvac.PurposeType.POSITIVE:
                fn += 1
            else:
                fp += 1
            
    return tp, fp, tn, fn

def splitRunSet( runset_pos, runset_neg, fold, chunksize, evalsize, rndidx ):
    '''Take parts of runset_pos and runset_neg and re-combine into
    a training set and an evaluation set.  For use by crossValidate().
    '''
    num_items = ( len(runset_pos), len(runset_neg) )
    evalidx(0) = range( fold*chunksize(0), fold*chunksize(0)+evalsize(0) )
    evalidx(1) = range( fold*chunksize(1), fold*chunksize(1)+evalsize(1) )
    trainidx(0) = range( 0, fold*chunksize(0) ) + range( fold*chunksize(0)+evalsize(0), num_items(0) )
    trainidx(1) = range( 0, fold*chunksize(1) ) + range( fold*chunksize(1)+evalsize(1), num_items(1) )
    evalset_pos  = runset_pos[ rndidx(0)[evalidx(0)] ]
    evalset_neg  = runset_neg[ rndidx(1)[evalidx(1)] ]
    trainset_pos = runset_pos[ rndidx(0)[trainidx(0)] ]
    trainset_neg = runset_neg[ rndidx(1)[trainidx(1)] ]
    evalset  = cvac.RunSet()
    evalset.purposedLists = (evalset_pos, evalset_neg)
    trainset = cvac.RunSet()
    trainset.purposedLists = (trainset_pos, trainset_neg)
    return trainset, evalset

def crossValidate( trainer, detector, runset, folds=10 ):
    '''Returns summary statistics tp, fp, tn, fn, recall, precision,
    and a detailed matrix of results with one row for
    each fold, and one column each for true positive, false
    positive, true negative, and false negative counts'''

    # sanity checks:
    # only positive and negative purposes,
    # count number of entries for each purpose
    # todo: warn that directories will be treated as one entity, if any
    runset_pos = asList( runset, purpose="pos" )
    runset_neg = asList( runset, purpose="neg" )
    num_items = ( len(runset_pos), len(runset_neg) )
    # check that there are no other purposes
    all_items = len( asList( runset ) )
    if sum(num_items)!=all_items:
        raise RuntimeError("crossValidate can only handle Positive and Negative purposes")
    if min(num_items)<2:
        raise RuntimeError("need more than 1 labeled item per purpose to cross validate")

    # make sure there are enough items for xval to make sense
    if folds>min(num_items):
        print("warning: cannot do "+folds+"-fold validation with only "+str(num_items)+" labeled items")
        folds = min(num_items)

    # calculate the size of the training and evaluation sets.
    # if the number of labeled items in the runset divided
    # by the number of folds isn't an even
    # division, use more items for the evaluation
    chunksize = (floor( num_items(0)/folds ), floor( num_items(1)/folds ))
    trainsize = (chunksize(0) * (folds-1), chunksize(1) * (folds-1))
    evalsize  = (num_items(0)-trainsize(0), num_items(1)-trainsize(1))

    # randomize the order of the elements in the runset, once and for all folds
    rndidx(0) = range( num_items(0) )
    rndidx(1) = range( num_items(1) )
    random.shuffle( rndidx(0) ) # shuffles items in place
    random.shuffle( rndidx(1) ) # shuffles items in place

    results = np.empty( (folds, 4), np.dtype=int )
    for fold in range( folds ):
        # split the runset
        trainset, evalset = splitRunSet( runset_pos, runset_neg, fold, chunksize, evalsize, rndidx )

        # training
        training( trainer, trainset )

        # detection
        res = detect( detector, evalset )
        results[fold,:] = evalResults( res )

    # calculate statistics
    sums = np.sum( results, 0 )  # sum over rows
    tp = sums[0], fp = sums[1], tn = sums[2], fn = sums[3]
    recall = tp/(tp+fn)
    precision = tp/(tp+fp)
        
    return tp, fp, tn, fn, recall, precision, results

class Contender:
    '''Ultimately a detector, this product can be built from
    a trainer or be a pre-defined detector.  Any combination that
    that can be trained into a detector is valid.
    Once a trainer is specified, it is assumed that this contender
    needs to be trained, irrespective of whether detectorData are
    present or not.
    '''
    def __init__( self, name ):
        self.name = name
        self.trainer = None
        self.trainerString = None
        self.trainerProps = None
        self.detector = None
        self.detectorString = None
        self.detectorData = None
        self.detectorProps = None

    def hasTrainer( self ):
        if self.trainer or self.trainerString:
            return True
        return False

    def hasDetector( self ):
        if self.detector or self.detectorString:
            return True
        return False

    def isSufficientlyConfigured( self ):
        if not self.hasDetector():
            return False
        if self.hasTrainer():
            # we can produce a trainedModel (detectorData)
            return True
        # warn if no trainer and no detectorData because this might
        # cause a failure after training only, but return true
        if not self.detectorData:
            print( "Warning: contender has no trainer and no detectorData" )
        return True

    def getTrainer( self ):
        if not self.trainer:
            self.trainer = easy.getTrainer( self.trainerString )
        return self.trainer

    def getDetector( self ):
        if not self.detector:
            self.detector = easy.getDetector( self.detectorString )
        return self.detector

def joust( contenders, runset, method='crossvalidate', folds=10 ):
    '''evaluate the contenders on the runset, possibly training
    and evaluating with n-fold cross-validation or another method.
    The contenders parameter is a list of detectors (or
    trainer-detector tuples) that are to be evaluated.'''

    # check arguments
    if not method=='crossvalidate':
        raise RuntimeError( 'method ' + method + ' unknown.' )
    for c in contenders:
        if not c.isSufficientlyConfigured():
            return 'needs more configuration: ' + str(c)

    results = {}
    cnt = 0
    for c in contenders:
        d = c.getDetector()
        if c.hasTrainer():
            t = c.getTrainer()
            res = easy.crossValidate( t, d, runset, folds )
        else:
            # todo: break out functionality from crossValidate
            res = easy.evaluate( d, c.detectorData, runset, trainsize )
        # calculate combined recall/precision score:
        score = (res.recall + res.precision)/2
        results[cnt++] = (score, c.name, res)

    return sort( results )

# for testing only:
if __name__ == '__main__' :
    # test the comparative detector evaluation including
    # training of a model

    # Bag of Words
    c1 = Contender("BOW")
    c1.trainerString = "BOW_Trainer:default -p 10103"
    c1.detectorString = "BOW_Detector:default -p 10104"

    # Histogram of Oriented Gradients
    c2 = Contender("HOG")
    c2.trainerString = "HOG_Trainer:default -p 10117"
    c2.detectorString = "HOGTest:default -p 10118"

    # Deformable Parts Model;
    # currently, no trainer interface is available
    c3 = Contender("DPM")
    c3.detectorString = "DPM_Detector:default -p 10116"
    c3.detectorData = "detectors/amodel.zip"

    runset = easy.createRunSet( "trainImg/kr/Kr001.jpg", "pos" )
    easy.addToRunSet( runset, "trainImg/kr/Kr002.jpg", "pos" )
    easy.addToRunSet( runset, "trainImg/kr/Kr003.jpg", "pos" )
    easy.addToRunSet( runset, "trainImg/ca/ca0003.jpg", "neg" )
    easy.addToRunSet( runset, "trainImg/ca/ca0004.jpg", "neg" )
    easy.addToRunSet( runset, "trainImg/ca/ca0005.jpg", "neg" )
    easy.printRunSetInfo( runset )
    
    res = joust( [c1, c2, c3], runset, folds=3 )
    print res
    
