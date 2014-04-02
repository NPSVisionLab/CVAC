'''
Easy!  mini tutorial
Apply a pre-trained detector to an image
matz 6/17/2013
'''

import easy

# obtain a reference to a Bag of Words (BOW) detector
detector = easy.getDetector( "BOW_Detector" )

# a model for distinguishing Canadian, Korean, and US flags,
# trained previously with a BOW-specific trainer and stored in a file
modelfile = "detectors/bowUSKOCA.zip"

# a test image; the location is relative to the "CVAC.DataDir"
imgfile = "testImg/TestKrFlag.jpg"

# apply the detector type, using the model and testing the imgfile
results = easy.detect( detector, modelfile, imgfile )

# you can print the results with Easy!'s pretty-printer;
# we will explain the meaning of the "unlabeled" and the number
# of the found label later.
# or you can always print variables directly (uncomment the next line):
# print("{0}".format( results ))
print("------- Bag of Words results for flags: -------")
easy.printResults( results )

# let's try a different model, pre-trained not for
# flags but for various corporate logos
print("------- Bag of Words results for corporate logos: -------")
modelfile = "detectors/bowCorporateLogoModel.zip"
imgfile = "corporate_logos/shell/shell2.png"
results = easy.detect( detector, modelfile, imgfile )
easy.printResults( results )

# test the same image with a different detector type and model;
print("------- Deformable Parts Model results for Starbucks logo: -------")
try:
    detector = easy.getDetector( "DPM_Detector" )
    modelfile = "detectors/dpmStarbucksLogo.zip"
    imgfile = "corporate_logos/shell/shell2.png"
    results = easy.detect( detector, modelfile, imgfile )
    easy.printResults( results )
    imgfile = "corporate_logos/starbucks/starbucks-logo.png"
    results = easy.detect( detector, modelfile, imgfile )
    easy.printResults( results )
except:
    print("DPM detector not installed: if desired, please obtain it via\n"\
          "https://github.com/NPSVisionLab/PartsBasedDetector")
    
# yet another detector type
print("------- Viola-Jones cascades for face detection: -------")
detector = easy.getDetector( "OpenCVCascadeDetector" )
modelfile = "detectors/haarcascade_frontalface_alt.xml"
imgfile = "testImg/italia.jpg"
results = easy.detect( detector, modelfile, imgfile )
easy.printResults( results )

# face corpus
# corpus = easy.openCorpus( "corpus/Faces94.properties" )
# results = easy.detect( detector, modelfile, corpus )
