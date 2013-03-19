cmake_minimum_required(VERSION 2.6)

#Turn on CTest input files
ENABLE_TESTING()

PROJECT(CVAC)

#OPTION(BUILD_WITH_MULTIBOOST "Enables the projects that depend on the MultiBoost library" OFF)
OPTION(BUILD_WITH_BOW "Enables the projects that depend on the bow library" OFF)
#OPTION(BUILD_WITH_SBD "Enables the projects that depend on the SBD library" OFF)
OPTION(BUILD_WITH_OPENCVPERFORMANCE "Enables the projects that depend on the OpencvPerformance library" OFF)
#OPTION(BUILD_WITH_WIREDIAGRAM "Enables building a wiring diagram detector" OFF)
OPTION(BUILD_WITH_TESTS "Build the tests, which depend on UnitTest++" OFF)
#OPTION(BUILD_WITH_VIDEOVALIDATORFOROPENCV "Enables the projects that depend on the VideoValidatorForOpenCV library" OFF)

# where the Find*.cmake files are
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules
                      ${CMAKE_MODULE_PATH})

SET(PROJECT_BINARY_DIR ${CMAKE_SOURCE_DIR})

SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
MAKE_DIRECTORY(${OUTPUT_BINDIR})

SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)
MAKE_DIRECTORY(${OUTPUT_LIBDIR})

#Some CVAC "SDK" variables to be used in local projects
SET(CVAC_ROOT_DIR ${CMAKE_SOURCE_DIR} CACHE PATH "The root of the CVAC SDK")
SET(CVAC_INCLUDE_DIR ${CVAC_ROOT_DIR}/include CACHE PATH "The CVAC SDK include directory")

SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${OUTPUT_LIBDIR} CACHE PATH "build directory")
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${OUTPUT_BINDIR} CACHE PATH "build directory")
IF(WIN32)
  SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${OUTPUT_BINDIR} CACHE PATH "build directory")
ELSE(WIN32)
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR} CACHE PATH "build directory")
ENDIF(WIN32)

# For each configuration (Debug, Release, MinSizeRel... and/or anything the user chooses)
FOREACH(CONF ${CMAKE_CONFIGURATION_TYPES})
# Go uppercase (DEBUG, RELEASE...)
STRING(TOUPPER "${CONF}" CONF)
SET("CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_LIBDIR}")
SET("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_BINDIR}")
IF(WIN32)
  SET("CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_BINDIR}")
ELSE()
  SET("CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_LIBDIR}")
ENDIF()
ENDFOREACH()

CONFIGURE_FILE(runIcebox.sh.in runIcebox.sh)

IF (WIN32)
   SET(CMAKE_DEBUG_POSTFIX  "d")

   OPTION(BUILD_WITH_MP "Enables the /MP multi-processor compiler option for Visual Studio 2005 and above" ON)
   MARK_AS_ADVANCED(BUILD_WITH_MP)

   IF(BUILD_WITH_MP)
     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
   ENDIF(BUILD_WITH_MP)

   # Platform specific definitions
   ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
   ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
   ADD_DEFINITIONS(-DNOMINMAX)
ENDIF (WIN32)

# ADD -fPIC to g++ because it's hard for the script to figure out 
# if you are building a library or not, and -fPIC will work on everything.
IF(CMAKE_COMPILER_IS_GNUCXX)
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

FIND_PACKAGE(Ice REQUIRED)
FIND_PACKAGE(LibArchive)
FIND_PACKAGE(UnitTest++)

IF(BUILD_WITH_MULTIBOOST OR BUILD_WITH_OPENCVPERFORMANCE OR BUILD_WITH_BOW OR BUILD_WITH_SBD OR BUILD_WITH_VIDEOVALIDATORFOROPENCV)
  FIND_PACKAGE( OpenCV REQUIRED core imgproc highgui ml features2d nonfree objdetect calib3d PATHS ../CVAC_extras/OpenCV-2.4.2)
ENDIF (BUILD_WITH_MULTIBOOST OR BUILD_WITH_OPENCVPERFORMANCE OR BUILD_WITH_BOW OR BUILD_WITH_SBD OR BUILD_WITH_VIDEOVALIDATORFOROPENCV)

ADD_SUBDIRECTORY(src/util)
ADD_SUBDIRECTORY(../CVAC_algorithms bin)
IF( BUILD_WITH_TESTS)
  SET( BUILD_UNIT_TESTS ON )
  ADD_SUBDIRECTORY(test)
ENDIF( BUILD_WITH_TESTS )
