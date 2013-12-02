#
# high-level functionality to evaluate trainers and detectors
# Matz, July 2013
# last edit: November 2013
#
import random
random.seed()
import numpy

import easy
import cvac
import math

def getRelativePath( label ):
    return label.sub.path.directory.relativePath + "/" + label.sub.path.filename

def evalResults( results, origMap, foundMap ):
    '''Determine true and false positives and negatives based on
    the purpose of original and found labels.
    Returns tp, fp, tn, fn'''
    
    tp = 0; fp = 0; tn = 0; fn = 0
    for res in results:
        foundPurposes = []
        for lbl in res.foundLabels:
            foundPurpose = foundMap[ easy.getLabelText( lbl.lab, guess=False ) ]
            foundPurposes.append(foundPurpose)
        numfound = len(res.foundLabels)
        origpur = origMap[ getRelativePath(res.original) ]
        # todo: should check the other found labels; need a strategy
        # to deal with more than one found label
        if numfound==1 and origpur==foundPurposes[0]:
            # "true"
            if origpur.ptype==cvac.PurposeType.POSITIVE:
                tp += 1
            else:
                tn += 1
        else:
            if numfound>1:
                print("warning: evalResults cannot deal with multiple found labels yet")
            # "false"
            if origpur.ptype==cvac.PurposeType.POSITIVE:
                fn += 1
            else:
                fp += 1

    print("found: {0}, {1}, {2}, {3}".format(tp,fp,tn,fn))
            
    return tp, fp, tn, fn

def splitRunSet( runset_pos, runset_neg, fold, chunksize, evalsize, rndidx ):
    '''Take parts of runset_pos and runset_neg and re-combine into
    a training set and an evaluation set.  For use by crossValidate().
    '''
    num_items = ( len(runset_pos), len(runset_neg) )
    evalidx_pos  = range( fold*chunksize[0], fold*chunksize[0]+evalsize[0] )
    evalidx_neg  = range( fold*chunksize[1], fold*chunksize[1]+evalsize[1] )
    trainidx_pos = range( 0, fold*chunksize[0] ) + range( fold*chunksize[0]+evalsize[0], num_items[0] )
    trainidx_neg = range( 0, fold*chunksize[1] ) + range( fold*chunksize[1]+evalsize[1], num_items[1] )
    # The following line selects those elements from runset_pos
    # that correspond to the randomized indices for the current
    # evaluation chunk.  Think of this, conceptually:
    # evalset_pos  = runset_pos[ rndidx[0][evalidx_pos] ]
    # Subsequent lines: equivalently, for runset_neg, and trainset pos/neg
    evalset_pos  = list( runset_pos[i] for i in list( rndidx[0][j] for j in evalidx_pos) )
    evalset_neg  = list( runset_neg[i] for i in list( rndidx[1][j] for j in evalidx_neg) )
    trainset_pos = list( runset_pos[i] for i in list( rndidx[0][j] for j in trainidx_pos) )
    trainset_neg = list( runset_neg[i] for i in list( rndidx[1][j] for j in trainidx_neg) )

    # create a RunSet with proper purposes
    trainset = cvac.RunSet()
    trainset.purposedLists = (cvac.PurposedLabelableSeq(easy.getPurpose("pos"), trainset_pos),
                              cvac.PurposedLabelableSeq(easy.getPurpose("neg"), trainset_neg))
    evalset  = cvac.RunSet()
    evalset.purposedLists = (cvac.PurposedLabelableSeq(easy.getPurpose("pos"), evalset_pos),
                             cvac.PurposedLabelableSeq(easy.getPurpose("neg"), evalset_neg))
    return trainset, evalset

def asList( runset, purpose=None ):
    '''You can pass in an actual cvac.RunSet or a dictionary with
    the runset and a classmap, as returned by createRunSet.'''
    if type(runset) is dict and not runset['runset'] is None\
        and isinstance(runset['runset'], cvac.RunSet):
        runset = runset['runset']
    if not runset or not isinstance(runset, cvac.RunSet) or not runset.purposedLists:
        raise RuntimeError("no proper runset")
    if isinstance(purpose, str):
        purpose = easy.getPurpose( purpose )

    rsList = []
    for plist in runset.purposedLists:
        if purpose and not plist.pur==purpose:
            # not interested in this purpose
            continue
        if isinstance(plist, cvac.PurposedDirectory):
            print("warning: runset contains directory; will treat as one for folds")
        elif isinstance(plist, cvac.PurposedLabelableSeq):
            rsList = rsList + plist.labeledArtifacts
        else:
            raise RuntimeError("unexpected plist type "+type(plist))
    return rsList

class CrossValidationResult:
    # see below
    pass

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
    chunksize = (int(math.floor( num_items[0]/folds )), int(math.floor( num_items[1]/folds )))
    trainsize = (chunksize[0] * (folds-1), chunksize[1] * (folds-1))
    evalsize  = (num_items[0]-trainsize[0], num_items[1]-trainsize[1])
    print( "Will perform a {0}-fold cross-validation with {1} training samples and "
           "{2} evaluation samples".format( folds, trainsize, evalsize ) )

    # randomize the order of the elements in the runset, once and for all folds
    rndidx = ( range( num_items[0] ), range( num_items[1] ) )
    random.shuffle( rndidx[0] ) # shuffles items in place
    random.shuffle( rndidx[1] ) # shuffles items in place

    results = numpy.empty( [folds, 4], dtype=int )
    for fold in range( folds ):
        # split the runset
        trainset, evalset = splitRunSet( runset_pos, runset_neg, fold, chunksize, evalsize, rndidx )
        print( "-------- fold number {0} --------".format(fold) )

        # training
        print( "---- training:" )
        easy.printRunSetInfo( trainset )
        model = easy.train( trainer, trainset )

        # detection
        print( "---- evaluation:" )
        easy.printRunSetInfo( evalset )
        res = easy.detect( detector, model, evalset )

        # omap maps the relative file path of every label to the assigned purpose
        omap = {}
        for plist in evalset.purposedLists:
            assert( isinstance(plist, cvac.PurposedLabelableSeq) )
            for sample in plist.labeledArtifacts:
                omap[ getRelativePath(sample) ] = plist.pur
        # todo: what's -1?, also: move outside this function
        fmap = {'1':easy.getPurpose('pos'), '-1':easy.getPurpose('neg'), '0':easy.getPurpose('neg')}

        results[fold,:] = evalResults( res, origMap=omap, foundMap=fmap )

    # calculate statistics
    r = CrossValidationResult()
    r.folds = folds
    sums = numpy.sum( results, 0 )  # sum over rows
    r.tp = sums[0]; r.fp = sums[1]; r.tn = sums[2]; r.fn = sums[3]
    # (r.tp, r.fp, r.tn, r.fn) = numpy.sum( results, 0 )  # sum over rows
    num_pos = r.tp+r.fn
    if num_pos!=0:
        r.recall = r.tp/float(num_pos)
    else:
        r.recall = 0
    num_det = r.tp+r.fp
    if num_det!=0:
        r.precision = r.tp/float(r.tp+r.fp)
    else:
        r.precision = 0
    r.results = results
    
    return r

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
        print("======== evaluating contender '{0}' ========".format( c.name ) )
        # todo: use TrainerProps and DetectorProps
        d = c.getDetector()
        if c.hasTrainer():
            t = c.getTrainer()
            res = crossValidate( t, d, runset, folds )
        else:
            # todo: break out functionality from crossValidate
            res = easy.evaluate( d, c.detectorData, runset, trainsize )

        print("results: {0}% recall with {1}% precision "
              "({2} tp, {3} fp, {4} tn, {5} fn)"
              .format( res.recall*100.0, res.precision*100.0,
                       res.tp, res.fp, res.tn, res.fn ))
        print res.results

        # calculate combined recall/precision score:
        score = (res.recall + res.precision)/2.0
        results[cnt] = (score, c.name, res)
        cnt += 1

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
    easy.printRunSetInfo( runset, printLabels=True )
    
    res = joust( [c1, c2, c3], runset, folds=3 )
    print res
    
