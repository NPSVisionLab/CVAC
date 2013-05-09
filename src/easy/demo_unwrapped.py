# before interpreting this file, make sure this is set:
# export PYTHONPATH="/opt/Ice-3.4.2/python:test/UnitTests/python"
import sys, traceback
#sys.path.append('''c:\Program Files (x86)\Zeroc\Ice-3.4.2\python''')
#sys.path.append('''C:\Users\tomb\Documents\nps\git\myCVAC\CVACvisualStudio\test\UnitTests\python''')
#sys.path.append('''C:\Users\tomb\Documents\nps\git\myCVAC\CVACvisualStudio\test\UnitTests\python\cvac''')
sys.path.append('''.''')
import Ice
if "C:\Program Files (x86)\ZeroC_Ice\python" not in sys.path:
    sys.path.append("C:\Program Files (x86)\ZeroC_Ice\python")
import Ice
import IcePy
import cvac
import unittest
import corpus
import service

#import corpus
#import service
#import cvac

#
#  open a Corpus of labeled data
#
ic = Ice.initialize(sys.argv)
cs_base = ic.stringToProxy("CorpusServer:default -p 10011")
cs = cvac.CorpusServicePrx.checkedCast(cs_base)
if not cs:
    raise RuntimeError("Invalid CorpusServer proxy")

dataRoot = cvac.DirectoryPath( "corpus" );
corpusConfigFile = cvac.FilePath( dataRoot, "CvacCorpusTest.properties" )
corpus = cs.openCorpus( corpusConfigFile )

corpusTestDir = cvac.DirectoryPath( "trainImg" );
# corpusTestDir = cvac.DirectoryPath( "corpusTestDir" );
corpus = cs.createCorpus( corpusTestDir )
if not corpus:
    raise RuntimeError("Could not create corpus from path '"
                       +dataRoot.relativePath+"/"+corpusTestDir+"'")

lablist = cs.getDataSet( corpus )
categories = {}
for lb in lablist:
    if lb.lab.name in categories:
        categories[lb.lab.name].append( lb )
    else:
        categories[lb.lab.name] = [lb]

# print information about this corpus
sys.stdout.softspace=False;
print 'Obtained', len(lablist), 'labeled artifacts from corpus', corpus.name, ':'
for key in sorted( categories.keys() ):
    klen = len( categories[key] )
    print "{0} ({1} artifact{2})".format( key, klen, ("s","")[klen==1] )

#
# add some samples to a RunSet
#
pur_categories = []
cnt = 0
for key in sorted( categories.keys() ):
    purpose = cvac.Purpose( cvac.PurposeType.MULTICLASS, cnt )
    pur_categories.append( cvac.PurposedLabelableSeq( purpose, categories[key] ) )
    cnt = cnt+1
runset = cvac.RunSet( pur_categories )

#
# Connect to a trainer service, train on the RunSet
#
trainer_base = ic.stringToProxy("bowTrain:default -p 10003")
#trainer_base = ic.stringToProxy("BOW_Trainer:default -p 10003")
trainer = cvac.DetectorTrainerPrx.checkedCast(trainer_base)
if not trainer:
    raise RuntimeError("Invalid DetectorTrainer proxy")

# this will get called once the training is done
class TrainerCallbackReceiverI(cvac.TrainerCallbackHandler):
    detectorData = None
    def createdDetector(self, detData, current=None):
        self.detectorData = detData
        print "Finished training, obtained DetectorData of type", self.detectorData.type

# ICE functionality to enable bidirectional connection for callback
adapter = ic.createObjectAdapter("")
callback = Ice.Identity()
callback.name = Ice.generateUUID()
callback.category = ""
tcbrec = TrainerCallbackReceiverI()
adapter.add( tcbrec, callback)
adapter.activate()
trainer.ice_getConnection().setAdapter(adapter)

# connect to trainer, initialize with a verbosity value, and train
trainer.initialize( 3 )
trainer.process( callback, runset )

if tcbrec.detectorData.type == cvac.DetectorDataType.FILE:
    print "received file: {0}/{1}".format(tcbrec.detectorData.file.directory.relativePath,
                                          tcbrec.detectorData.file.filename)
elif tcbrec.detectorData.type == cvac.DetectorDataType.BYTES:
    print "received bytes"
elif tcbrec.detectorData.type == cvac.DetectorDataType.PROVIDER:
    print "received a reference to a DetectorData provider"

#
# Connect to a detector service,
# test on the training RunSet for validation purposes
#
detector_base = ic.stringToProxy("bowTest:default -p 10004")
detector = cvac.DetectorPrx.checkedCast(detector_base)
if not detector:
    raise RuntimeError("Invalid Detector service proxy")

# this will get called when results have been found
class DetectorCallbackReceiverI(cvac.DetectorCallbackHandler):
    def foundNewResults(self, r2, current=None):
        for res in r2.results:
            numfound = len(res.foundLabels)
            origname = (res.original.lab.name, "unlabeled")[res.original.lab.hasLabel]
            print "result for {0} ({1}): found {2} label{3}: {4}".format(
                res.original.sub.path.filename, origname,
                #','.join(res.foundLabels) )
                numfound, ("s","")[numfound==1], res.foundLabels[0].lab.name )

# ICE functionality to enable bidirectional connection for callback
adapter = ic.createObjectAdapter("")
callback = Ice.Identity()
callback.name = Ice.generateUUID()
callback.category = ""
adapter.add(DetectorCallbackReceiverI(), callback)
adapter.activate()
detector.ice_getConnection().setAdapter(adapter)

# connect to detector, initialize with a verbosity value
# and the trained model, and run the detection on the runset
detector.initialize( 3, tcbrec.detectorData )
detector.process( callback, runset )

print 'returned from process call'


quit()


caltech101 = cs.openCorpus( cvac.FilePath( dataRoot, "Caltech101.properties" ) )
# c = corpus.Corpus( "testcorpus" )
print 'loaded corpus named ', caltech101.name
print caltech101.description

samples = c.categories[ 'sunflower' ]


quit()
#
# Train a detector on the RunSet
#
trainer = cvac.DetectorTrainer( 'Felzenszwalb' )
sunflower_detector = trainer.train( runset )

def detect( n ):
    a, b = 0, 1
    while b < n:
        print b,
        a, b = b, a+b
        
