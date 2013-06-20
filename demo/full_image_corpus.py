#
# Easy!  mini tutorial
#
# Utilize a Corpus of images with full-image labels
#
# matz 6/19/2013

import easy

# uncomment the line below if you want the program to
# pause after large chunks of text output
def wait():
    # raw_input("Press Enter to continue...")
    pass

# Open a corpus that is specified via a "properties" file;
# note that the path is relative to CVAC.DataDir, as always.
# This doesn't do much yet, it just reads the properties file.
corpus1 = easy.openCorpus( "corpus/CvacCorpusTest.properties" )

# Now let's obtain the labels contained in this corpus.  With
# this particular corpus, the labels are only available if the
# image files are local to the CorpusServer, which in this case is
# the default, local server.  More on that in a moment.
# Images will be downloaded automatically if the createMirror flag
# is set, however, a network connection is required.
categories1, lablist1 = easy.getDataSet( corpus1, createMirror=True )
print("=== Corpus 1: ===");
print('Obtained {0} labeled artifact{1} from corpus1 "{2}":'.format(
    len(lablist1), ("s","")[len(lablist1)==1], corpus1.name ));
easy.printCategoryInfo( categories1 )

# Create a corpus that consists of files under a directory.  This
# is at first identical to creating a RunSet from a folder.
corpus2 = easy.openCorpus( "trainImg" )
categories2, lablist2 = easy.getDataSet( corpus2, createMirror=True )
print("\n=== Corpus 2: ===");
print('Obtained {0} labeled artifact{1} from corpus1 "{2}":'.format(
    len(lablist2), ("s","")[len(lablist2)==1], corpus2.name ));
easy.printCategoryInfo( categories2 )

# Note how both corpora contain flag images, but they have different
# labels.  To use them for evaluation, let's assign the same purpose
# to syntactically different but semantically identical labels.
# Because we don't specify it, this guesses the specific Purpose that
# is assigned to the labels.
# Also obtain this mapping from Purpose to label name, called "classmap."
rs1 = easy.createRunSet( categories1['US_flag']+categories2['us'], "1" )
easy.addToRunSet( rs1, categories1['KO_flag']+categories2['kr'], "2" )
easy.addToRunSet( rs1, categories1['CA_flag']+categories2['ca'], "3" )
print("\n=== The Corpora combined into one RunSet: ===");
easy.printRunSetInfo( rs1 )

# The runset can again be used for training and testing
print("------- Bag of Words results for corporate logos: -------")
detector = easy.getDetector( "bowTest:default -p 10104" )
modelfile = "detectors/bowUSKOCA.zip"
results1 = easy.detect( detector, modelfile, rs1 )
print("Note that both original and found labels are printed:")
easy.printResults( results1 )

# Print again, this time replacing the found labels with a double
# mapping from foundLabel -> guessed Purpose -> classmap label;
# Note that this fails if multiple original labels mapped to the same
# Purpose.
wait()
print("------- Same results, but found labels replaced with guessed original labels: -------")
easy.printResults( results1, foundMap=rs1['classmap'], inverseMap=True )

# Print again, this time replacing all labels with their assigned
# purposes, bot original and found labels.  Note the "identical label"
# matches.
wait()
print("------- Same results, but labels replaced with purposes: -------")
easy.printResults( results1, origMap=rs1['classmap'], foundMap=rs1['classmap'] )

# We were lucky though: the flags and the purposes we assigned is
# identical to the assignment when the Bag of Words model was trained.
# That cannot be guaranteed, so a custom mapping might be required.
# Let's create another runset, this time changing the purpose numbers:
rs2 = easy.createRunSet( categories1['US_flag']+categories2['us'], "2" )
easy.addToRunSet( rs2, categories1['KO_flag']+categories2['kr'], "3" )
easy.addToRunSet( rs2, categories1['CA_flag']+categories2['ca'], "1" )
results2 = easy.detect( detector, modelfile, rs2 )
wait()
print("------- Without appropriate classmaps for original and found labels: -------")
easy.printResults( results2, origMap=rs2['classmap'], foundMap=rs2['classmap'] )

wait()
print("------- With appropriate classmaps for original and found labels: -------")
foundmp = {'1':'2', '2':'3', '3':'1'}
easy.printResults( results2, origMap=rs2['classmap'], foundMap=foundmp )

print("------- Same, now mapped back from Purpose to original Label: -------")
foundmp = {'1':'US_flag', '2':'KO_flag', '3':'CA_flag'}
easy.printResults( results2, origMap=rs2['classmap'], foundMap=foundmp, inverseMap=True )


#import pdb
#pdb.set_trace()

quit()

cs = easy.getCorpusServer("CorpusServer:default -p 10011")
