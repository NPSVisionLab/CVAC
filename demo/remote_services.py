'''
Easy!  mini tutorial
Invoke a remote service, send files, receive files, receive messages
matz 6/17/2013
'''

import easy

#
# Example 1: Test on a remote machine.
# specify the host name of the service
#
host = "-h vision.nps.edu"

#
# create a simple RunSet with just one unlabeled image
#
rs1 = easy.createRunSet( "testImg/italia.jpg" )

#
# Make sure all files in the RunSet are available on the remote site;
# it is the client's responsibility to upload them if not.
# The putResult contains information about which files were actually transferred.
#
print("------- Remote detection, local result display: -------")
fileserver = easy.getFileServer( "PythonFileService:default -p 10111 " + host )
putResult = easy.putAllFiles( fileserver, rs1 )
modelfile = "detectors/haarcascade_frontalface_alt.xml"
if not fileserver.exists( easy.getCvacPath(modelfile) ):
    easy.putFile( fileserver, easy.getCvacPath(modelfile) )

#
# detect remotely: note the host specification
#
detector = easy.getDetector( "OpenCVCascadeDetector:default -p 10102 "+host )
results = easy.detect( detector, modelfile, rs1 )
easy.printResults( results )

#
# Example 2:
# Train on a remote machine, obtain the model file, and test locally.
# Assume the files are on the remote machine, or transfer with putAllFiles.
# If no local services are installed, this will be skipped.
#
print("------- Remote training, local detection: -------")
try:
    detector = easy.getDetector( "BOW_Detector:default -p 10104" ) # local service
    trainer = easy.getTrainer( "BOW_Trainer:default -p 10103 "+ host) # remote
    trainset = easy.createRunSet( "trainImg" );
    trainedModel = easy.train( trainer, trainset )
    easy.getFile( fileserver, trainedModel )  # downloads the model from remote
    print("obtained trained detector, stored in file {0}"
          .format(easy.getFSPath(trainedModel)))
    testset = easy.createRunSet("testImg","UNPURPOSED"  )
    results = easy.detect( detector, trainedModel, testset )
    easy.printResults( results )
except:
    print("Cannot connect to local detector.  Have you started the services?\n"\
          "This part of the demo does not work with the client-only distribution.")
