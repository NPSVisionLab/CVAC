#
# Easy!  mini tutorial
#
# Create a RunSet in several ways, use it for detection and evaluation
#
# matz 6/18/2013

import easy
import cvac

# a simple RunSet with just one unlabeled image
rs1 = easy.createRunSet( "testImg/italia.jpg" )
easy.printRunsetInfo( rs1 )

# to give samples a purpose, state the purpose:
rs2 = easy.createRunSet( "testImg/italia.jpg", "POSITIVE" )
easy.printRunsetInfo( rs2 )

# add more samples to a runset; anything starting with "pos"
# will be added into the POSITIVE sequence of labeled items
easy.addToRunSet( rs2, "testImg/TestKrFlag.jpg", "POS" )
easy.addToRunSet( rs2, "testImg/TestCaFlag.jpg", "neg" )
easy.printRunsetInfo( rs2 )

quit()

#
# add all samples from corpus to a RunSet,
# also obtain a mapping from class ID to label name
#
res = easy.createRunSet( "data/testImg" )
runset = res['runset']
classmap = res['classmap']
easy.printRunsetInfo( runset )
