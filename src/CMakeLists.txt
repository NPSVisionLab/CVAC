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
SET ( PROJECT_INTERFACE_JAR RunSet.jar )
SET ( SLICE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/sliced )
SET ( JAVA_OUTPUT_CLASSPATH ${CMAKE_CURRENT_BINARY_DIR}/cp )
MAKE_DIRECTORY ( ${SLICE_OUTPUT_PATH} )
MAKE_DIRECTORY ( ${JAVA_OUTPUT_CLASSPATH} )

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


# Command to create all .class files from the .java files
SET ( JAVAC_ARGS -cp ${ICE_ROOT}/lib/Ice.jar
        -d ${JAVA_OUTPUT_CLASSPATH} ${SLICE_OUTPUT_PATH}/cvac/*.java )
ADD_CUSTOM_COMMAND(
    OUTPUT ${JAVA_OUTPUT_CLASSPATH}/cvac/${SLICE_TOKEN_CLASS}.class
    COMMAND ${CMAKE_Java_COMPILER} 
    ARGS ${JAVAC_ARGS}
    DEPENDS ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java
    COMMENT "Building Java objects ${JAVA_OUTPUT_CLASSPATH}/cvac/*.class" )
MESSAGE ( STATUS "Java .class files will be compiled using this command:" )
MESSAGE ( STATUS "${CMAKE_Java_COMPILER} ${JAVAC_ARGS}" )


# the .jar file for the target
SET ( JAR_ARGS "-cf ${PROJECT_INTERFACE_JAR} ${JAVA_OUTPUT_CLASSPATH}/cvac/*.class" )
ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_INTERFACE_JAR}
    DEPENDS ${JAVA_OUTPUT_CLASSPATH}/${SLICE_TOKEN_CLASS}.class
    COMMAND ${CMAKE_COMMAND}
    ARGS -E chdir ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_Java_ARCHIVE} ${JAR_ARGS}
    COMMENT "Creating archive ${PROJECT_INTERFACE_JAR} from .class files." )
MESSAGE ( STATUS "Java archive will be built using this command:" )
MESSAGE ( STATUS "${CMAKE_Java_ARCHIVE} ${JAR_ARGS}" )

# the target
#ADD_CUSTOM_TARGET( ${PROJECT_INTERFACE_JAR}
#    DEPENDS ${JAVA_OUTPUT_CLASSPATH}/cvac/${SLICE_TOKEN_CLASS}.class
#)



SET(SOURCE_FILES
    cvac/corpus/Corpus.java
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
SET( CMAKE_JAVA_INCLUDE_PATH ${Ice.jar} ${JAVA_OUTPUT_CLASSPATH} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_INTERFACE_JAR})

ADD_JAR( Corpus ${SOURCE_FILES} ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java
         ${JAVA_OUTPUT_CLASSPATH}/cvac/${SLICE_TOKEN_CLASS}.class
         ${SLICE_OUTPUT_PATH}/cvac/*.java
         DEPENDS ${JAVA_OUTPUT_CLASSPATH}/cvac/${SLICE_TOKEN_CLASS}.class ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_INTERFACE_JAR})

