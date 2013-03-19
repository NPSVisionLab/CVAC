PROJECT(MultiBoost)

# for now, install in subdirectory of source directory by default
SET( CMAKE_INSTALL_PREFIX .. )
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

# CMake-specifics
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
IF(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
#    cmake_policy(SET CMP0015 NEW)
ENDIF(COMMAND cmake_policy)

IF( UNIX )
#    ADD_DEFINITIONS( -DUNIX -O2 )
    ADD_DEFINITIONS( -D_LINUX -D_x86 )
ELSEIF( APPLE )
    ADD_DEFINITIONS( -DOSX )
ELSEIF( WIN32 )
    SET( WIN_LIBS ws2_32.lib )
ENDIF( UNIX )

SET( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules
                       ${CMAKE_MODULE_PATH}
                       ${CMAKE_CURRENT_SOURCE_DIR} )

# OpenCV library, GSL headers 
SET(OpenCV_DIR	../opencv/build CACHE PATH "Path to the OpenCVConfig.cmake file") 

FIND_PACKAGE( OpenCV)
IF( OpenCV_FOUND )
    ADD_DEFINITIONS( -DHAVE_OPENCV )
    include_directories( ${OpenCV_INCLUDE_DIRS} )
    SET( LIBS ${LIBS} ${OpenCV_LIBS} )
ENDIF( OpenCV_FOUND )


# user specified options
OPTION( HAVE_SCALE_FACTORS "OpenCV modified for min and max detector
scaling factors" 0 )
IF( HAVE_SCALE_FACTORS )
 ADD_DEFINITIONS( -DHAVE_SCALE_FACTORS )
ENDIF( HAVE_SCALE_FACTORS )

# print the most important configuration details
MESSAGE(STATUS "OpenCV includes: ${OpenCV_INCLUDE_DIRS}")
MESSAGE(STATUS "OpenCV library: ${OpenCV_LIBS}")

OPTION(BUILD_WITH_PROTOBUF "Build MultiBoost with ProtoBuffer support" OFF)

IF (BUILD_WITH_PROTOBUF)
    # Protobuf library
    #include(FindProtobuf) 
    FIND_PACKAGE( Protobuf )
    IF( ${PROTOBUF_FOUND} )
        ADD_DEFINITIONS( -DHAVE_PROTOBUF )
        INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIRS})
        SET( LIBS ${LIBS} ${PROTOBUF_LIBRARIES} )
    ENDIF( ${PROTOBUF_FOUND} )
ENDIF(BUILD_WITH_PROTOBUF)

IF (WIN32)
   SET(CMAKE_DEBUG_POSTFIX  "d")

   OPTION(BUILD_WITH_MP "Enables the /MP multi-processor compiler option for Visual Studio 2005 and above" ON)
   MARK_AS_ADVANCED(BUILD_WITH_MP)
   IF(BUILD_WITH_MP)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
   ENDIF(BUILD_WITH_MP)
ENDIF (WIN32)

# Declare the targets: The MultiBoost library, detector and trainer
#########################
SET( MULTIBOOST_LIB_FILES
	MultiBoost/Clasificador.cpp
	MultiBoost/CrossConvolution.cpp
	MultiBoost/dictionary.cpp
	MultiBoost/jointBoost.cpp
	MultiBoost/parameters.cpp
	MultiBoost/rundetector.cpp
	MultiBoost/utilities.cpp
)

IF (${PROTOBUF_FOUND})
    PROTOBUF_GENERATE_CPP(PROTOBUFF_SOURCES PROTOBUFF_HEADERS
                          MultiBoost/trainingData.proto)                   
ENDIF (${PROTOBUF_FOUND})

SET (MULTIBOOST_HEADER_FILES
        MultiBoost/ComputeFeatures.h
        MultiBoost/CrossConvolution.h
        MultiBoost/dictionary.h
        MultiBoost/fileManager.h
        MultiBoost/jointBoost.h
        MultiBoost/MultiBoost.h
        MultiBoost/parameters.h
        MultiBoost/runDetector.h
#        MultiBoost/stdafx.h
        MultiBoost/targetver.h
        MultiBoost/testDetector.h
        MultiBoost/trainDetector.h
        MultiBoost/utilities.h
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}) #for protobuf generated files
ADD_LIBRARY( mboost
             ${MULTIBOOST_LIB_FILES}
             ${MULTIBOOST_HEADER_FILES}
             ${PROTOBUFF_SOURCES} 
             ${PROTOBUFF_HEADERS} 
           )
           
SET_TARGET_PROPERTIES( mboost PROPERTIES Version 1.0 )


###########################
SET( MULTIBOOST_DETECT_FILES
	MultiBoost/MultiBoost_detect.cpp
	MultiBoost/fileManager.cpp
	MultiBoost/testDetector.cpp
)

ADD_EXECUTABLE( mboost_detect ${MULTIBOOST_DETECT_FILES} )
TARGET_LINK_LIBRARIES( mboost_detect mboost ${LIBS} ${WIN_LIBS} )
SET_TARGET_PROPERTIES( mboost_detect PROPERTIES Version 1.0 )

###########################
SET( MULTIBOOST_TRAIN_LIB_SOURCE
	MultiBoost/ComputeFeatures.cpp
	MultiBoost/fileManager.cpp
	MultiBoost/trainDetector.cpp
	MultiBoost/dictionary_build.cpp
	MultiBoost/MultiBoostTrainerApp.cpp
)
SET (MULTIBOOST_TRAIN_LIB_HEADERS
     MultiBoost/ComputeFeatures.h
     MultiBoost/fileManager.h
     MultiBoost/MultiBoostTrainerApp.h
     MultiBoost/trainDetector.h
)

INCLUDE_DIRECTORIES(.)

ADD_LIBRARY(mboost_train ${MULTIBOOST_TRAIN_LIB_SOURCE} 
                         ${MULTIBOOST_TRAIN_LIB_HEADERS})
                         
TARGET_LINK_LIBRARIES( mboost_train mboost )
#SET_TARGET_PROPERTIES( mboost_train PROPERTIES Version 1.0 )

INSTALL_TARGETS( bin mboost_detect )

######################################
SET(MULTIBOOST_TRAIN_APP_SOURCE
	MultiBoost/MultiBoost_train.cpp
)

ADD_EXECUTABLE(mboost_train_app ${MULTIBOOST_TRAIN_APP_SOURCE})
TARGET_LINK_LIBRARIES(mboost_train_app mboost_train
                                       ${LIBS})
