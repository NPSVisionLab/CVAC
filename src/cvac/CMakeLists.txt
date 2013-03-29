PROJECT( Corpus Java )

cmake_minimum_required(VERSION 2.8.6)
# Java support since CMake 2.8.6
FIND_PACKAGE( Java )
INCLUDE( UseJava )

#WRAP_SLICE2JAVA( SLICE_FILES src cvac
#           ${CVAC_ROOT_DIR}/src/Data.ice
#           ${CVAC_ROOT_DIR}/src/Services.ice
#)

SET(SOURCE_FILES
    corpus/Corpus.java
    corpus/LabelableListI.java
    #util/RunSetWrapper.java
    ${SLICE_FILES}
    ${CMAKE_CURRENT_BINARY_DIR}/*.java
)

SET(HEADER_FILES
)

INCLUDE_DIRECTORIES(
)

FIND_JAR( Ice.jar Ice 
	  PATHS 
	  	${CVAC_ROOT_DIR}/lib 
		${ICE_ROOT}/lib
		)
SET( CMAKE_JAVA_INCLUDE_PATH ${Ice.jar} )

ADD_JAR( Corpus ${SOURCE_FILES} )

