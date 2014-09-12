'''
Easy!  mini tutorial
Repeatedly train and evaluate for efficient label use; bootstrap.
matz 6/21/2013
'''

import os
import easy

#
# Create a training set from one sample each of 9 corporate logos
#
trainset1 = easy.createRunSet( "corporate_logos" )

# train, round 1
trainer = easy.getTrainer( "BOW_Trainer")
model1 = easy.train( trainer, trainset1 )

# evaluate the model on a separate test set, images and videos
# in DataDir/testImg
testset1 = easy.createRunSet( "testImg", "UNPURPOSED" )
easy.printRunSetInfo( testset1 )
detector = easy.getDetector( "BOW_Detector" )
result1 = easy.detect( detector, model1, testset1 )
easy.printResults(result1)

# sort the images from the testdata1 folder into subfolders of
# "testresults1" corresponding to the found labels;
# if multiple labels were found per original, consider only
# the label with the highest confidence
easy.sortIntoFolders( result1, outfolder="testresults1", multi="highest")

# Now manually sort through the created testresults1 and move
# _incorrectly_ classified samples into correctly labeled subfolders
# of a new folder "corporate_logos_round2".  Found labels on locations
# that are not one of the 9 logos have to be sorted into a 10th class
# (subfolder), generally called the "reject" class.
#
# We simulate this manual process here.  Note that the new folder needs to be
# accessible from the CorpusServer, hence located under the CVAC.DataDir.
print("Manual sorting will be simulated. Please read the file comments.")
reject_folder = easy.CVAC_DataDir + "/corporate_logos_round2/reject"
if not os.path.isdir( reject_folder ):
    os.makedirs( reject_folder )
nologos = ["TestKrFlag.jpg", "italia.jpg", "korean-american-flag.jpg", "TestUsFlag.jpg"]
for nologo in nologos:
    for root, dirnames, filenames in os.walk("testresults1"):
        for filename in filenames:
            if nologo == filename:
                fname = os.path.join(root, filename)
                newf =  reject_folder + "/" + nologo
                if (os.path.isfile(newf)):
                    os.unlink(newf) #If file exists delete it (required for some OS's)    
                os.rename( fname, newf)

# Create the new training set and combine it with the trainset1.
# (Alternatively, in the previous step, manually sort the wrong
# classifications into the original corporate_logos subfolders.)  Note
# that the original label->purpose assignment needs to be retained, or
# else the createRunSet method will assign arbitrary purposes.
# Also, some trainers treat the "reject" class differently.  Omit the line of
# code that assigns the NEGATIVE purpose in case the trainer does not
# distinguish the "reject" class.
mapwithreject = trainset1['classmap']
easy.addToClassmap( mapwithreject, 'reject', easy.getPurpose('neg') )
trainset2 = easy.createRunSet( "corporate_logos_round2", classmap=mapwithreject )
trainset2['runset'].purposedLists.extend( trainset1['runset'].purposedLists )
easy.printRunSetInfo( trainset2, printLabels=True )

# train, round 2
model2 = easy.train( trainer, trainset2 )

# repeat the evaluation (this time with model2), the manual sorting etc.
# with new testsets until the performance is satisfactory.
result1 = easy.detect( detector, model2, testset1 )
easy.printResults(result1)
