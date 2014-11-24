'''
Easy!  mini tutorial
Create a RunSet in several ways, use it for detection and evaluation
matz 6/18/2013
'''

import easy
import cvac

# a simple RunSet with just one unlabeled image;
# remember that paths are relative to CVAC.DataDir
rs1 = easy.createRunSet( "testImg/italia.jpg" )
print("=== RunSet 1: ===");
easy.printRunSetInfo( rs1, printLabels=True )

# to give samples a purpose, state the purpose:
rs2 = easy.createRunSet( "testImg/italia.jpg", "POSITIVE" )
print("\n=== RunSet 2: ===");
easy.printRunSetInfo( rs2, printLabels=True )

# add more samples to a runset; anything starting with "pos"
# will be added into the POSITIVE sequence of labeled items
easy.addToRunSet( rs2, "testImg/TestKrFlag.jpg", "POS" )
easy.addToRunSet( rs2, "testImg/TestCaFlag.jpg", "neg" )
easy.addToRunSet( rs2, "testImg/TestUsFlag.jpg", "0" )
print("\n=== RunSet 2, after appending: ===");
easy.printRunSetInfo( rs2, printLabels=True )

# create a runset from a folder with sub-folders
rs3 = easy.createRunSet( "trainImg" )
print("\n=== RunSet 3: ===");
easy.printRunSetInfo( rs3, printLabels=True )

# you can check wether the runset has been properly constructed:
if easy.isProperRunSet( rs3 ):
    print("correctly constructed runset")
else:
    print("improperly constructed runset")
