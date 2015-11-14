'''
Run the BOW asyncronously via icegrid
'''

import easy
import os
import cvac
import threading

strDetector = "BOW_Detector"

'''
   This detector callback will be called by all the detector callbacks
   in a multitreaded fashion.  To handle this we add a threading lock
   to insure that only one thread is writing the results at a time.
'''
class MyDetectorCallbackReceiverI(easy.DetectorCallbackReceiverI):
    def __init__(self):
        import easy
        easy.DetectorCallbackReceiverI.__init__(self)
        self.mylock = threading.Lock()

    def foundNewResults(self, r2, current=None):
        with self.mylock:
            easy.DetectorCallbackReceiverI.foundNewResults(self, r2, current)

#===============================================================================

def getImageExtensions():
    return ["bmp","dib","jpeg","jpg","jpe","jp2", \
            "png","pbm","pgm","ppm","sr","ras", \
            "tiff","tif","gif","giff","emf","pct", \
            "pcx","pic","pix","vga","wmf"];
'''
   Provide a list of all the image files in the provided directory
'''
def searchDir(fset, dir):
    flist = os.listdir(dir)
    for f in flist:
        if os.path.isdir(dir + '/' + f):
            searchDir(fset, dir +'/' + f)
        else:
            if '.' not in f:
                continue
            name, ext = f.rsplit('.',1)
            extLower = ext.lower();
            if extLower in getImageExtensions():
                fset.append(dir +'/' + f)
            

'''
Since we running asynchronously, we will pass a single
file at a time.  This allows us to run multiple detections at the same time.
'''
subset = []
searchDir(subset,'data/testImg')

'''
Testing with a detector model
'''
mycallback = MyDetectorCallbackReceiverI()
trainedModel = "detectors/bowUSKOCA.zip"
detector = easy.getDetector(strDetector)
print("----- testing with pre-trained detector model with icegrid-----")
asyncRes =  []

'''
  We send each detection process a file to detect.  Depending on how the
  IceGrid is configured, these detection processes could possibly run on 
  different machines.  The default configuration just runs on the local 
  machine in different processes so the speed up will be limited to the
  number of cores the machine has.
'''
for file in subset:
    # We pass async=True. This causes the detect call to not wait for the
    # server to finish. Instead of returning an actual result detect will
    # return an asyncRes which we can monitor for completion.
    ares = easy.detect( detector, trainedModel, file, async=True, callbackRecv=mycallback )
    # Save all the async results returned so we know when they are complete.
    asyncRes.append(ares)

# Now wait for results.  We will wait in the order we requested them.
while len(asyncRes) > 0:
    ares = asyncRes[0]
    del asyncRes[0]
    ares.waitForCompleted()

# Now we can print all the results.
easy.printResults(mycallback.allResults)

