PROJECT( JavaServices Java )

cmake_minimum_required(VERSION 2.8.6)
# Java support since CMake 2.8.6
FIND_PACKAGE( Java )
INCLUDE( UseJava )
#For compatability we want to use java sdk 1.6
SET(CMAKE_JAVA_COMPILE_FLAGS ${CMAKE_JAVA_COMPILE_FLAGS} -source 1.6 -target 1.6)

SET( SLICE_FILES
    ${CVAC_ROOT_DIR}/src/Files.ice
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
SET ( SLICE_ARGS -I${ICE_ROOT}/slice --output-dir ${SLICE_OUTPUT_PATH} )

#    COMMAND "${ICE_SLICE2JAVA_EXECUTABLE}"  ARGS "${SLICE_ARGS}"

ADD_CUSTOM_COMMAND(
    OUTPUT "${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java" 
    PRE_BUILD
    COMMAND "${ICE_SLICE2JAVA_EXECUTABLE}"  ${SLICE_FILES} ${SLICE_ARGS}
    DEPENDS ${SLICE_FILES}
    WORKING_DIRECTORY "${CVAC_ROOT_DIR}/src"
    COMMENT "Generating (slicing) .java files from .ice sources")
MESSAGE ( STATUS "Java source files will be created from Slice definitions using this command:" )

MESSAGE ( STATUS "${ICE_SLICE2JAVA_EXECUTABLE} ${SLICE_FILES} ${SLICE_ARGS}" )


SET( CORPUS_SOURCE_FILES
    cvac/corpus/CorpusI.java
    cvac/corpus/CorpusServer.java
    cvac/corpus/CorpusServiceI.java
    cvac/corpus/CorpusConfig.java
    cvac/corpus/LabelableListI.java
    #util/RunSetWrapper.java
    cvac/corpus/CommonDataSet.java
)

SET( FILESERVER_SOURCE_FILES
    cvac/fileserver/FileServer.java
    cvac/fileserver/FileServiceI.java
)

SET(HEADER_FILES
)

INCLUDE_DIRECTORIES(
)

#
# find and add all the dependencies to build the jar files.
# FIND_JAR current doesn't have a way to check for REQUIRED modules,
# so we need to check for success ourselves.
#
FIND_JAR( Ice.jar Ice 
	  PATHS 
	  	${CVAC_ROOT_DIR}/lib 
		${ICE_ROOT}/lib
		)
FIND_JAR( IceBox.jar IceBox 
	  PATHS 
	  	${CVAC_ROOT_DIR}/lib 
		${ICE_ROOT}/lib
		)
IF( ${BUILD_FILESERVER} )
ENDIF( ${BUILD_FILESERVER} )

IF( ${BUILD_CORPUS_SERVER} )
  FIND_JAR( javatar.jar NAMES javatar-2.5 PATHS ${CVAC_ROOT_DIR}/3rdparty/lib )
  IF( NOT javatar.jar )
    MESSAGE( SEND_ERROR "Need javatar.jar to build Corpus server")
  ENDIF( NOT javatar.jar )
  FIND_JAR( IO.jar NAMES commons-io-2.4 PATHS ${CVAC_ROOT_DIR}/3rdparty/lib )
  IF( NOT IO.jar )
    MESSAGE( SEND_ERROR "Need commons-io-2.4 to build Corpus server")
  ENDIF( NOT IO.jar )
ENDIF( ${BUILD_CORPUS_SERVER} )

IF( ${BUILD_CORPUS_LABELME} )
  SET( CORPUS_SOURCE_FILES ${CORPUS_SOURCE_FILES} cvac/corpus/LabelMeDataSet.java )
  FIND_JAR( javabuilder.jar NAMES javabuilder PATHS ${CVAC_ROOT_DIR}/3rdparty/lib )
  IF( NOT javabuilder.jar )
   MESSAGE( SEND_ERROR "Need Matlab javabuilder.jar to build LabelMe connection")
  ENDIF( NOT javabuilder.jar )
  FIND_JAR( labelme.jar NAMES labelme PATHS ${CVAC_ROOT_DIR}/3rdparty/lib )
  IF( NOT labelme.jar )
   MESSAGE( SEND_ERROR "Need labelme.jar to build LabelMe connection")
  ENDIF( NOT labelme.jar )
ENDIF( ${BUILD_CORPUS_LABELME} )

SET( CMAKE_JAVA_INCLUDE_PATH ${Ice.jar} ${IceBox.jar} ${javatar.jar} ${IO.jar} ${javabuilder.jar} ${labelme.jar})
SET( CMAKE_JAVA_TARGET_OUTPUT_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
IF (WIN32)
SET( CMAKE_JAVA_JAR_PATH ${Ice.jar};${IceBox.jar};${javatar.jar};${IO.jar};${javabuilder.jar};${labelme.jar})
ELSE (WIN32)
SET( CMAKE_JAVA_JAR_PATH ${Ice.jar}:${IceBox.jar}:${javatar.jar}:${IO.jar}:${javabuilder.jar}:${labelme.jar})
ENDIF (WIN32)


FUNCTION( copy_file target destdir )
    ADD_CUSTOM_COMMAND( TARGET ${target}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ARGS ${target}.jar ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
      COMMENT "copying ${target}.jar to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
    )
endfunction()

# Create a jar file for the sliced Corpus files only
# and then some for all, including the hardcoded "implementation" files;
# The former is handy to have if the hardcoded ones are built in another IDE
#IF (MSVC)
FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cvac)
#add_custom_target(CvacSliced
add_custom_command(TARGET ${target}
                   OUTPUT ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.class
                   COMMAND ${Java_JAVAC_EXECUTABLE} -d ${CMAKE_CURRENT_BINARY_DIR}/cvac -cp "${CMAKE_JAVA_JAR_PATH}" ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java ${SLICE_OUTPUT_PATH}/cvac/*.java
                   DEPENDS ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java
                   COMMENT "compiling CvacSliced"
                   WORKING_DIRECTORY ${CVAC_ROOT_DIR}/src )
#make a jar
add_custom_command(TARGET ${target}
                   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CvacSliced.jar
                   COMMAND ${Java_JAR_EXECUTABLE}  -cvf ${CMAKE_CURRENT_BINARY_DIR}/CvacSliced.jar -C ${CMAKE_CURRENT_BINARY_DIR}/cvac . > list
                   DEPENDS "${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.class"
                   COMMENT "jaring CvacSliced"
                   )

add_custom_command(TARGET ${target}
                   OUTPUT ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/CvacSliced.jar
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/CvacSliced.jar ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
                   COMMENT "copying CvacSliced"
                   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CvacSliced.jar)

add_custom_target(CvacSliced ALL
                  DEPENDS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/CvacSliced.jar
                  )

#ELSE (MSVC)
#ADD_JAR( CvacSliced ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java
#        ${SLICE_OUTPUT_PATH}/cvac/*.java
#        DEPENDS
#            ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java
#)
#ENDIF (MSVC)


IF( ${BUILD_CORPUS_SERVER} )

#IF (MSVC)
FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/corpus)
#ALL causes all corpus.jar to be build always
add_custom_command(TARGET ${target}
                   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/corpus/cvac/corpus/CorpusServer.class
                   COMMAND ${Java_JAVAC_EXECUTABLE}  -d ${CMAKE_CURRENT_BINARY_DIR}/corpus -cp "${CMAKE_JAVA_JAR_PATH}" ${CORPUS_SOURCE_FILES} util/*java ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java ${SLICE_OUTPUT_PATH}/cvac/*.java
                   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CvacSliced.jar
                   COMMENT "compiling Corpus"
                   WORKING_DIRECTORY ${CVAC_ROOT_DIR}/src )

#make the jar
add_custom_command(TARGET ${target}
                   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Corpus.jar
                   COMMAND ${Java_JAR_EXECUTABLE}  -cvf ${CMAKE_CURRENT_BINARY_DIR}/Corpus.jar -C ${CMAKE_CURRENT_BINARY_DIR}/corpus . > list
                   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/corpus/cvac/corpus/CorpusServer.class
                   COMMENT "jaring Corpus"
                   )

#copy the jar to the libaray dir
add_custom_command(TARGET ${target}
                   OUTPUT ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Corpus.jar
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/Corpus.jar ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
                   COMMENT "copying Corpus"
                   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Corpus.jar)

add_custom_target(Corpus ALL
                  DEPENDS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Corpus.jar
                  )
#Add target level dependancy so CvacSliced gets done first
add_dependencies(Corpus CvacSliced)
  #copy_file( Corpus ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
#ELSE (MSVC)
#  ADD_JAR( Corpus ${CORPUS_SOURCE_FILES} ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java
#        ${SLICE_OUTPUT_PATH}/cvac/*.java
#        DEPENDS
#            ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java
#  )
#IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
#  INSTALL_JAR( Corpus ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
#ENDIF( NOT ${BUILD_3RDPARTY_PACKAGE} )
#ENDIF(MSVC)

ENDIF( ${BUILD_CORPUS_SERVER} )

IF( ${BUILD_FILESERVER} )
#IF (MSVC)
FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/fileserver)
#ALL causes all FileServer.jar to be build always
add_custom_command(TARGET ${target}
                   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/fileserver/cvac/fileserver/FileServer.class
                   COMMAND ${Java_JAVAC_EXECUTABLE}  -d ${CMAKE_CURRENT_BINARY_DIR}/fileserver -cp "${CMAKE_JAVA_JAR_PATH}" ${FILESERVER_SOURCE_FILES} ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java ${SLICE_OUTPUT_PATH}/cvac/*.java
                   COMMENT "compiling FileServer"
                   WORKING_DIRECTORY ${CVAC_ROOT_DIR}/src )

#make the jar
add_custom_command(TARGET ${target}
                   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/FileServer.jar
                   COMMAND ${Java_JAR_EXECUTABLE}  -cvf ${CMAKE_CURRENT_BINARY_DIR}/FileServer.jar -C ${CMAKE_CURRENT_BINARY_DIR}/fileserver . > list
                   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/fileserver/cvac/fileserver/FileServer.class
                   COMMENT "jaring FileServer"
                   )

#copy the jar to the libaray dir
add_custom_command(TARGET ${target}
                   OUTPUT ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/FileServer.jar
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/FileServer.jar ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
                   COMMENT "copying FileServer"
                   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/FileServer.jar)

add_custom_target(FileServer ALL
                  DEPENDS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/FileServer.jar
                  )
#Add target level dependancy so Corpus gets done first
add_dependencies(FileServer Corpus)
 #copy_file( FileServer ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
#ELSE (MSVC)
# ADD_JAR( FileServer ${FILESERVER_SOURCE_FILES} ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java
#        ${SLICE_OUTPUT_PATH}/cvac/*.java
#        DEPENDS
#            ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java
#  )
#IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
#  INSTALL_JAR( FileServer ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
#ENDIF( NOT ${BUILD_3RDPARTY_PACKAGE} )
#ENDIF (MSVC)


ENDIF( ${BUILD_FILESERVER} )

#INSTALL( TARGETS Corpus
#               DESTINATION lib
#             )
            
IF (BUILD_3RDPARTY_PACKAGE OR BUILD_BINARY_PACKAGE)
  INSTALL(FILES ${javatar.jar} ${IO.jar} ${javabuilder.jar} ${labelme.jar}
    DESTINATION 3rdparty/lib
    COMPONENT Runtime 
  )
ENDIF (BUILD_3RDPARTY_PACKAGE OR BUILD_BINARY_PACKAGE)

IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
  INSTALL(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Corpus.jar 
    DESTINATION bin
    COMPONENT Runtime
  )
  INSTALL(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/FileServer.jar 
    DESTINATION bin
    COMPONENT Runtime
  ) 
ENDIF ( NOT ${BUILD_3RDPARTY_PACKAGE} )

INCLUDE (UserCMakeLists.txt OPTIONAL)

