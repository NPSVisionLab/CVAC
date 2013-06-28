'''
Easy!  mini tutorial
Repeatedly train and evaluate for efficient label use; bootstrap.
matz 6/21/2013
'''
# Note: some of these Easy! functions are in alpha and
# planned for beta release v0.5

#
# Create a training set from one sample each of 9 corporate logos
#
trainset1 = easy.createRunSet( "corporate_logos" )

# train, round 1
trainer = easy.getTrainer( "bowTrain:default -p 10103 ")
model1 = easy.train( trainer, trainset1 )

# evaluate the model on a separate test set, images and videos
# in DataDir/testdata1
testset1 = easy.createRunSet( "testdata1", "UNPURPOSED" )
detector = easy.getDetector( "bowTest:default -p 10104" )
result1 = easy.detect( detector, model1, testset1 )

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
wait()

# Create the new training set and combine it with the trainset1.
# (Alternatively, in the previous step, manually sort the wrong
# classifications into the original corporate_logos subfolders.)  Note
# that the original label->purpose assignment needs to be retained, or
# else the createRunSet method will assign arbitrary purposes.
trainset2 = easy.createRunSet( "corporate_logos_round2", trainset1['classmap'] )
trainset2['runset'].purposedLists.append( trainset1['runset'].porposedLists )
                                          
# Some trainers treat the "reject" class differently:
easy.setPurposeOfLabel( trainset2, "reject", "NEGATIVE" )

# train, round 2
model2 = easy.train( trainer, trainset2 )

# repeat the evaluation (this time with model2), the manual sorting etc.
# with new testsets until the performance is satisfactory.
