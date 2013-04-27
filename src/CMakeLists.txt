PROJECT( Corpus Java )

cmake_minimum_required(VERSION 2.8.6)
# Java support since CMake 2.8.6
FIND_PACKAGE( Java )
INCLUDE( UseJava )

SET( SLICE_FILES
    ${CVAC_ROOT_DIR}/src/Data.ice
    ${CVAC_ROOT_DIR}/src/Services.ice
    ${CVAC_ROOT_DIR}/src/Corpus.ice
)

# need to single out one class so we can address it explicitly instead of with *
SET ( SLICE_TOKEN_CLASS RunSet )
SET ( SLICE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/sliced )
MAKE_DIRECTORY ( ${SLICE_OUTPUT_PATH} )
# create a dummy .java file in the sliced directory, so that CMake
# doesn't complain about no *.java files being present;
# this cannot be the same as SLICE_TOKEN_CLASS or else CMake thinks
# that dependency exists already and won't slice it
IF ( NOT EXISTS  ${SLICE_OUTPUT_PATH}/cvac/Purpose.java )
  CONFIGURE_FILE( ${CVAC_ROOT_DIR}/src/Purpose.java.in ${SLICE_OUTPUT_PATH}/cvac/Purpose.java COPYONLY )
ENDIF()

#
# Command to run slice2java to produce all .java files
#
SET ( SLICE_ARGS ${SLICE_FILES} -I${ICE_ROOT}/slice --output-dir ${SLICE_OUTPUT_PATH} )
ADD_CUSTOM_COMMAND(
    OUTPUT ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java
    COMMAND ${ICE_SLICE2JAVA_EXECUTABLE}
    ARGS ${SLICE_ARGS}
    DEPENDS ${SLICE_FILES}
    COMMENT "Generating .java files from .ice sources." )
MESSAGE ( STATUS "Java source files will be created from Slice definitions using this command:" )
MESSAGE ( STATUS "${ICE_SLICE2JAVA_EXECUTABLE} ${SLICE_ARGS}" )


SET(SOURCE_FILES
    cvac/corpus/CorpusI.java
    cvac/corpus/CorpusServer.java
    cvac/corpus/LabelableListI.java
    #util/RunSetWrapper.java
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

ADD_JAR( Corpus ${SOURCE_FILES} ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java
         ${SLICE_OUTPUT_PATH}/cvac/*.java
        DEPENDS 
            ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java
)

#INSTALL( TARGETS Corpus
#               DESTINATION lib
#             )
INSTALL_JAR( Corpus ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
             
