#CPACK_PACKAGE_FILE_NAME requires this
cmake_minimum_required(VERSION 2.8.6)

cmake_policy( SET CMP0012 NEW)

IF(WIN32)
    # don't want the four default configuration types with MS Visual Studio
    SET (CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "")
ENDIF(WIN32)

PROJECT(CVAC)

SET (PYTHON_ONLY_CLIENT true)
SET(CVAC_FIRST_SERVICE_PORT "10000" CACHE STRING "First socket port to use")

INCLUDE (UserCMakeLists.txt OPTIONAL)

OPTION(BUILD_WITH_BOW "Build a Bag of Words trainer and detector (requires OpenCV)" ON)
OPTION(BUILD_WITH_OPENCVPERFORMANCE "Build the OpenCV Cascade trainer and detector" ON)
OPTION(BUILD_FILESERVER "Build a File server (generally for servers), requires Java" OFF) 
OPTION(BUILD_FILESERVER_PYTHON "Configure a Python File Server (generally for servers)" ON) 
OPTION(BUILD_CORPUS_SERVER "Build a Corpus server (generally for clients), requires Java" OFF) 
OPTION(BUILD_CORPUS_SERVICE_PYTHON "Configure a Python Corpus Server" ON) 
OPTION(BUILD_CORPUS_LABELME "Build an interface to LabelMe, requires Java and Matlab Compiler Runtime" OFF)
OPTION(BUILD_CPP_DETECTOR_CLIENT "Build a C++ client for detectors" ON)
OPTION(BUILD_CPP_TRAINER_CLIENT "Build a C++ client for trainers" OFF)
OPTION(BUILD_WITH_TESTS "Build the tests, which depend on UnitTest++" OFF)
OPTION(BUILD_DOCUMENTATION "Build the CVAC API documentation with Doxygen (use 'make doc')" OFF)
OPTION(DOWNLOAD_3RDPARTY_PACKAGE "Attempt to download a platform-specific 3rd-party software package" OFF)
OPTION(BUILD_3RDPARTY_PACKAGE "Build the 3rd-Party development distribution package" OFF)
OPTION(BUILD_BINARY_PACKAGE "Build the binary distribution package" OFF)
OPTION(BUILD_WITH_RSI "Build a test for the runset iterator" OFF)
IF (APPLE)
OPTION(USE_LEGACY_STDC++LIB "Don't use the new c++11 stdc++lib" ON)
ENDIF (APPLE)

IF (PYTHON_ONLY_CLIENT)
    IF (BUILD_WITH_BOW OR BUILD_WITH_OPENCVPERFORMANCE)
        SET (PYTHON_ONLY_CLIENT false)
    ENDIF (BUILD_WITH_BOW OR BUILD_WITH_OPENCVPERFORMANCE)
ENDIF (PYTHON_ONLY_CLIENT)

IF (PYTHON_ONLY_CLIENT)
    SET (START_ICE_SERVICES 0)
ELSE (PYTHON_ONLY_CLIENT)
    SET (START_ICE_SERVICES 1)
ENDIF (PYTHON_ONLY_CLIENT)


MARK_AS_ADVANCED( BUILD_3RDPARTY_PACKAGE BUILD_BINARY_PACKAGE )

# where the Find*.cmake files are
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules
                      ${CMAKE_MODULE_PATH})

SET(PROJECT_BINARY_DIR ${CMAKE_SOURCE_DIR})

SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
MAKE_DIRECTORY(${OUTPUT_BINDIR})

SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)
MAKE_DIRECTORY(${OUTPUT_LIBDIR})

SET(SLICE_OUTPUT_PYTHONDIR ${OUTPUT_LIBDIR}/python)
MAKE_DIRECTORY(${SLICE_OUTPUT_PYTHONDIR})

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

SET (ICE_ENCODE "#")
SET(HOST "")

IF (WIN32)
   SET(CMAKE_DEBUG_POSTFIX  "d")

   OPTION(BUILD_WITH_MP "Enables the /MP multi-processor compiler option for Visual Studio 2005 and above" ON)
   MARK_AS_ADVANCED(BUILD_WITH_MP)

   IF(BUILD_WITH_MP)
     #Windows 8 messes up multiprocessor compile and slicing
     IF ( ${CMAKE_SYSTEM} STREQUAL "Windows-6.2" )
         SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP1")
     ELSE ( ${CMAKE_SYSTEM} STREQUAL "Windows-6.2" )
         SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
     ENDIF ( ${CMAKE_SYSTEM} STREQUAL "Windows-6.2" )
   ENDIF(BUILD_WITH_MP)

   # Platform specific definitions
   ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
   ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
   ADD_DEFINITIONS(-DNOMINMAX)
   # The java services need local host specified to run on windows 8
   SET(HOST "-h localhost")
   #If windows 8 (ie ice 3.5) then uncomment the use ice 1.0 encoding (vs 1.1)
   IF ( ${CMAKE_SYSTEM} STREQUAL "Windows-6.2" )
       SET (ICE_ENCODE "")
   ELSE ( ${CMAKE_SYSTEM} STREQUAL "Windows-6.2" )
       SET (ICE_ENCODE "#")
   ENDIF ( ${CMAKE_SYSTEM} STREQUAL "Windows-6.2" )
ENDIF (WIN32)

IF (APPLE)
   SET (ICE_ENCODE "")
ENDIF (APPLE)

# ADD -fPIC to g++ because it's hard for the script to figure out 
# if you are building a library or not, and -fPIC will work on everything.
IF(CMAKE_COMPILER_IS_GNUCXX)
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

SET(OPENCV_DIR_EXT "")
SET(UNITTEST_DIR_EXT "")
SET(ICE_BIN_DIR "/bin")
IF (APPLE)
   IF (USE_LEGACY_STDC++LIB)
       SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98 -stdlib=libstdc++")
       SET(OPENCV_DIR_EXT "_stdc")
       SET(UNITTEST_DIR_EXT "_stdc")
   ELSE (USE_LEGACY_STDC++LIB)
       SET(ICE_BIN_DIR "bin/c++11")
   ENDIF (USE_LEGACY_STDC++LIB)
ENDIF (APPLE)

#
# What version of CVAC?
#
# Parse the include/version.h file for the current CVAC version.
FILE( STRINGS ${CMAKE_SOURCE_DIR}/include/version.h
  versionfile_contents
  REGEX "#define CVAC_VERSION*" )
STRING( REGEX REPLACE ".*MAJOR ([^ ]+)#.*" "\\1" CVAC_VERSION_MAJOR ${versionfile_contents} )
STRING( REGEX REPLACE ".*MINOR ([^ ]+)#.*" "\\1" CVAC_VERSION_MINOR ${versionfile_contents} )
STRING( REGEX REPLACE ".*PATCH ([^ ]+).*" "\\1" CVAC_VERSION_PATCH ${versionfile_contents} )
SET( CVAC_VERSION ${CVAC_VERSION_MAJOR}.${CVAC_VERSION_MINOR}.${CVAC_VERSION_PATCH} )
SET( CVAC_VERSION_MAJMIN ${CVAC_VERSION_MAJOR}.${CVAC_VERSION_MINOR} )
MESSAGE( STATUS "successfully parsed version from version.h: CVAC-${CVAC_VERSION}" )

IF( "x${CVAC_VERSION}x" STREQUAL "xx"
    OR "x${CVAC_VERSION_MAJOR}x" STREQUAL "xx"
    OR "x${CVAC_VERSION_MINOR}x" STREQUAL "xx"
    OR "x${CVAC_VERSION_PATCH}x" STREQUAL "xx" )
    # need version string to continue for many tasks
    MESSAGE( SEND_ERROR "Could not obtain CVAC version from git or include/version.h:" )
    MESSAGE( SEND_ERROR "all:${CVAC_VERSION} major:${CVAC_VERSION_MAJOR} minor:${CVAC_VERSION_MINOR}" )
ENDIF()

# Try to re-create the include/version.h file that specifies the CVAC version,
# as obtained from the version-control tag (major.minor) and the
# number of completed builds in the devel branch since that tag (patch).
# Note that the number of builds is set on the build server.
# The build process increases the PATCH number found in the
# version.h file by one before building, and it commits it back into
# the repository.
if( INCREASE_VERSION_PATCH )
  if( NOT EXISTS "${CMAKE_SOURCE_DIR}/.git" )
    MESSAGE( SEND_ERROR "Cannot INCREASE_VERSION_PATCH without git repository" )
  endif()
  EXECUTE_PROCESS( COMMAND git describe --long --match "v[0-9]*.[0-9]*"
    OUTPUT_VARIABLE CVAC_VERSION_GIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
  IF ( NOT CVAC_VERSION_GIT )
    MESSAGE( SEND_ERROR "Error obtaining git version; cannot INCREASE_VERSION_PATCH" )
  ENDIF()
  # parse this kind of string v1.2-3-gkd3kdj4:
  # CVAC_VERSION_MAJMIN is 1.2
  # CVAC_VERSION_MAJOR is 1
  # CVAC_VERSION_MINOR is 2
  # CVAC_VERSION_PATCH is obtained from the previous version file.
  # CVAC_VERSION is 1.2.x
  STRING( REGEX REPLACE "^v(.*)-.*-.*" "\\1" CVAC_VERSION_MAJMIN ${CVAC_VERSION_GIT} ) 
  STRING( REGEX REPLACE "([^.]).*" "\\1" CVAC_VERSION_MAJOR ${CVAC_VERSION_MAJMIN} )
  STRING( REGEX REPLACE ".*([^.])-*" "\\1" CVAC_VERSION_MINOR ${CVAC_VERSION_MAJMIN} )
  MATH( EXPR CVAC_VERSION_PATCH "${CVAC_VERSION_PATCH}+1" )
  SET( CVAC_VERSION ${CVAC_VERSION_MAJOR}.${CVAC_VERSION_MINOR}.${CVAC_VERSION_PATCH} )
  MESSAGE( STATUS "successfully did INCREASE_VERSION_PATCH via git: CVAC-${CVAC_VERSION}" )
  CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/include/version.h.in
    ${CMAKE_SOURCE_DIR}/include/version.h)
  # don't store this variable in the cache or else the version will be increased
  # open every invocation of CMake in the same build directory
  UNSET( INCREASE_VERSION_PATCH CACHE )
endif( INCREASE_VERSION_PATCH )


# offer pre-packaged 3rd-party software for developers; specific to Major.Minor version but not Patch
#
IF( DOWNLOAD_3RDPARTY_PACKAGE )
    IF( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
        IF ( ${CMAKE_SYSTEM} STREQUAL "Windows-6.2" )
            IF ( NOT ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" )
                MESSAGE( SEND_ERROR "Visual Studio 10 required for Windows 8" )
            ENDIF ( NOT ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" )
            SET( 3RDPARTY_FILENAME CVAC_3rdparty-${CVAC_VERSION_MAJMIN}-Win8-win32.zip )
        ELSE ( ${CMAKE_SYSTEM} STREQUAL "Windows-6.2" )
            SET( 3RDPARTY_FILENAME CVAC_3rdparty-${CVAC_VERSION_MAJMIN}-Win7-win32.zip )
        ENDIF ( ${CMAKE_SYSTEM} STREQUAL "Windows-6.2" )
    ELSE( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
        SET( 3RDPARTY_FILENAME CVAC_3rdparty-${CVAC_VERSION_MAJMIN}-${CMAKE_SYSTEM_NAME}.tar.gz )
    ENDIF( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    IF( EXISTS ${CMAKE_SOURCE_DIR}/${3RDPARTY_FILENAME} )
        MESSAGE( STATUS "3rd-party archive exists already, won't download again (${3RDPARTY_FILENAME})" )
    ELSE( EXISTS ${CMAKE_SOURCE_DIR}/${3RDPARTY_FILENAME} )
        SET( 3RDPARTY_SERVER http://www.movesinstitute.org/~kolsch/CVAC/downloads )
        SET( 3RDPARTY_URL ${3RDPARTY_SERVER}/${3RDPARTY_FILENAME} )
        MESSAGE( STATUS "Trying to download 3rd-party archive from ${3RDPARTY_URL}" )
        FILE( DOWNLOAD ${3RDPARTY_URL} ${CMAKE_SOURCE_DIR}/${3RDPARTY_FILENAME}
                INACTIVITY_TIMEOUT 30  # abort if n seconds of inactivity
                STATUS DOWNLOAD_RESULT
                SHOW_PROGRESS
#                EXPECTED_MD5;aaaab03d4f38d7fe0525c994e52c3aaa
               )
        list(GET ${DOWNLOAD_RESULT} 0 ${DOWNLOAD_CODE}) 
        list(GET ${DOWNLOAD_RESULT} 1 ${DOWNLOAD_STRING}) 
        IF( NOT ${DOWNLOAD_CODE} EQUAL 0)
            FILE( REMOVE ${CMAKE_SOURCE_DIR}/${3RDPARTY_FILENAME} )  # this doesn't seem to work.
            MESSAGE( SEND_ERROR "Could not download 3rd-party software package: ${DOWNLOAD_STRING}" )
        ENDIF( NOT ${DOWNLOAD_CODE} EQUAL 0) 
    ENDIF( EXISTS ${CMAKE_SOURCE_DIR}/${3RDPARTY_FILENAME} )

    # untar or unzip the file;
    # note: should use "jar xf" or 7zip on Windows
    SET( 3RDPARTY_DIR ${CMAKE_SOURCE_DIR}/3rdparty )
    IF( EXISTS ${3RDPARTY_DIR} )
        MESSAGE( STATUS "3rd-party directory exists, not unpacking (${3RDPARTY_DIR})" )    
    ELSE( EXISTS ${3RDPARTY_DIR} )
        MESSAGE( STATUS "Unpacking 3rd-party software package to ${CMAKE_SOURCE_DIR}/3rdparty" )    
        EXECUTE_PROCESS( 
             COMMAND ${CMAKE_COMMAND} -E tar xzf ${3RDPARTY_FILENAME}
             WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
             RESULT_VARIABLE unpack_result
             OUTPUT_VARIABLE unpack_output
         )
        IF( (${unpack_result} STREQUAL "0") AND EXISTS ${CMAKE_SOURCE_DIR}/3rdparty
            AND IS_DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty )
            # clear variables that will be different with the 3rdparty package in place
            MESSAGE( STATUS "Successfully unpacked the 3rdparty package. You should" )
            MESSAGE( STATUS "delete CMakeCache.txt and re-run CMake.  Or run" )
            MESSAGE( STATUS "cmake -U ICE_* -U OpenCV_* -U LIBARCHIVE_*" )
            MESSAGE( STATUS "so that a subsequent CMake call can pick up the new locations." )
        ELSE()
            MESSAGE( SEND_ERROR "Could not unpack 3rdparty package - subsequent errors may occur" )
        ENDIF()
    ENDIF( EXISTS ${3RDPARTY_DIR} )
    SET( DOWNLOAD_3RDPARTY_PACKAGE OFF CACHE BOOL "Download 3rd-party package had download or unpack errors, please obtain manually" FORCE )
ENDIF( DOWNLOAD_3RDPARTY_PACKAGE )

#
# add a Makefile target "doc" to generate API documentation with Doxygen
#
IF( BUILD_DOCUMENTATION )
  # if the user wants doxygen built, we need doxygen, else complain
  FIND_PACKAGE( Doxygen REQUIRED )
  IF( DOXYGEN_FOUND )
      # this is the list of directories that doxygen will search for input files;
      # see Doxygen.in for a list of file types (basically, C, Java, and ICE files)
      set( CMAKE_DOXYGEN_INPUT_LIST
                ${CMAKE_SOURCE_DIR}/README.md
                ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/util
                ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/util
                ${CMAKE_SOURCE_DIR}/src/easy
                ${CMAKE_SOURCE_DIR}/demo
           )		
      string(REGEX REPLACE ";" " " CMAKE_DOXYGEN_INPUT_LIST "${CMAKE_DOXYGEN_INPUT_LIST}")

      # customize the "Doxyfile", the configuration for doxygen
      set( CMAKE_DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/doc )
      CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

      # create the Makefile target
      ADD_CUSTOM_TARGET( doc
                         ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
                         COMMENT "Generating API documentation with Doxygen" VERBATIM
                        )
  ENDIF( DOXYGEN_FOUND )
ENDIF( BUILD_DOCUMENTATION )

FIND_PACKAGE( PythonInterp 2.7)
FIND_PACKAGE(Ice REQUIRED)
FIND_PACKAGE(LibArchive REQUIRED)

#Opencv includes now required for src/util library
FIND_PACKAGE( OpenCV 2.4 REQUIRED
                            core imgproc highgui ml features2d nonfree objdetect calib3d
             HINTS
                            ${CMAKE_SOURCE_DIR}/3rdparty/opencv${OPENCV_DIR_EXT}
                            ${CMAKE_SOURCE_DIR}/3rdparty/opencv_stdc
)
#If the find package did not set these lib dirs, then set ourselfs
IF (OpenCV_LIB_DIR_OPT STREQUAL "")
    SET(OpenCV_LIB_DIR_OPT "${OpenCV_DIR}/../../lib")
ENDIF (OpenCV_LIB_DIR_OPT STREQUAL "")
IF (OpenCV_LIB_DIR_DBG STREQUAL "")
    SET(OpenCV_LIB_DIR_DBG "${OpenCV_DIR}/../../lib")
ENDIF (OpenCV_LIB_DIR_DBG STREQUAL "")

IF( BUILD_WITH_OPENCVPERFORMANCE OR BUILD_WITH_BOW )
  IF( OpenCV_DIR )
    MESSAGE( STATUS "Found OpenCV ${OpenCV_VERSION} configuration at ${OpenCV_CONFIG_PATH}" )
  ENDIF()
ENDIF( BUILD_WITH_OPENCVPERFORMANCE OR BUILD_WITH_BOW )

# Make sure packages are compatible
# ICE 3.5.1 is pre-built against Python 2.7
IF( PYTHONINTERP_FOUND )
  IF( APPLE)
      # make sure it's version 2.7 or else error
      IF( NOT PYTHON_VERSION_MAJOR EQUAL 2 OR NOT PYTHON_VERSION_MINOR EQUAL 7 )
        MESSAGE( WARNING "Found Python version ${PYTHON_VERSION_STRING} but need 2.6" )
        MESSAGE( WARNING "Please specify PYTHON_EXECUTABLE manually." )
      ENDIF()
  ELSE (APPLE)
      # make sure it's version 2.6 or else error
      IF( NOT PYTHON_VERSION_MAJOR EQUAL 2 OR NOT PYTHON_VERSION_MINOR EQUAL 6 )
        MESSAGE( WARNING "Found Python version ${PYTHON_VERSION_STRING} but need 2.6" )
        MESSAGE( WARNING "Please specify PYTHON_EXECUTABLE manually." )
      ENDIF()
  ENDIF (APPLE)

  #For Python set a variable for cv2 dynamic library
  #
  IF (WIN32)
    FIND_FILE(OPENCV_PY_LIB cv2.pyd
      HINTS ${OpenCV_DIR}/python/${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR})
  ELSE (WIN32)
    FIND_LIBRARY(OPENCV_PY_LIB cv2.so
      HINTS ${OpenCV_LIB_DIR_OPT}/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages
            ${OpenCV_LIB_DIR_OPT}/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/dist-packages)

  ENDIF (WIN32)
ENDIF()


## Setup the config.service and config.icebox files based upon detectors
## selected by the user.  These CF_ variables are defined in the config files
## and they get turned into comments or not.
IF( BUILD_WITH_BOW )
SET(CF_BOW "" )
SET( CF_BOW_T "" )
ELSE( BUILD_WITH_BOW )
SET( CF_BOW "#" )
SET( CF_BOW_T "#" )
ENDIF( BUILD_WITH_BOW )

IF( BUILD_WITH_OPENCVPERFORMANCE )
SET( CF_CAS "" )
SET( CF_CAS_T "" )
ELSE( BUILD_WITH_OPENCVPERFORMANCE )
SET( CF_CAS "#" )
SET( CF_CAS_T "#" )
ENDIF( BUILD_WITH_OPENCVPERFORMANCE )

IF( BUILD_CORPUS_SERVER )
SET( CF_CORPUS_S "")
ELSE( BUILD_CORPUS_SERVER )
SET( CF_CORPUS_S "#")
ENDIF( BUILD_CORPUS_SERVER )

IF( BUILD_FILESERVER )
SET( CF_FILE_S "")
ELSE( BUILD_FILESERVER )
SET( CF_FILE_S "#")
ENDIF( BUILD_FILESERVER )

IF( BUILD_WITH_RSI )
SET(CF_RSI "" )
ELSE( BUILD_WITH_RSI )
SET( CF_RSI "#" )
ENDIF( BUILD_WITH_RSI )

# if cmake install prefix not set the set it to default else add 
# cvac to the end
IF( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
    SET( CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/installed" CACHE PATH "default install path" FORCE )
ELSE( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
    SET( CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/cvac" )
ENDIF( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )

#Need to have this at top level since multiple directories need it
#SET( SLICE_FILES
#    ${CVAC_ROOT_DIR}/src/Files.ice
#    ${CVAC_ROOT_DIR}/src/Data.ice
#    ${CVAC_ROOT_DIR}/src/Services.ice
#    ${CVAC_ROOT_DIR}/src/Corpus.ice
#)
IF( BUILD_CORPUS_SERVER OR BUILD_FILESERVER )
    # This requires CMake 2.8 for Java support
    ADD_SUBDIRECTORY( src )
ENDIF( BUILD_CORPUS_SERVER OR BUILD_FILESERVER )

ADD_SUBDIRECTORY(src/util)
ADD_SUBDIRECTORY(src/easy) 
ADD_SUBDIRECTORY(src/clients) 
ADD_SUBDIRECTORY(algorithms)
ADD_SUBDIRECTORY(demo)
ADD_SUBDIRECTORY( etc )

#Get all the port numbers based upon the CVAC_FIRST_SERVICE_PORT
MATH(EXPR ADMIN_PORT "${CVAC_FIRST_SERVICE_PORT}-3")
MATH(EXPR JAVA_ADMIN_PORT "${CVAC_FIRST_SERVICE_PORT}-5")
MATH(EXPR OPENCV_CASCADE_DETECTOR_PORT "${CVAC_FIRST_SERVICE_PORT}+102")
MATH(EXPR OPENCV_CASCADE_TRAINER_PORT "${CVAC_FIRST_SERVICE_PORT}+107")
MATH(EXPR BOW_TRAINER_PORT "${CVAC_FIRST_SERVICE_PORT}+103")
MATH(EXPR BOW_DETECTOR_PORT "${CVAC_FIRST_SERVICE_PORT}+104")
MATH(EXPR CORPUS_PORT "${CVAC_FIRST_SERVICE_PORT}+11")
MATH(EXPR PYTHON_CORPUS_PORT "${CVAC_FIRST_SERVICE_PORT}+21")
MATH(EXPR FILE_SERVER_PORT "${CVAC_FIRST_SERVICE_PORT}+110")
MATH(EXPR PYTHON_FILE_SERVER_PORT "${CVAC_FIRST_SERVICE_PORT}+111")
MATH(EXPR RSITR_TEST_PORT "${CVAC_FIRST_SERVICE_PORT}+109")

# the configuration of config.{icebox,service} must come after including the
# etc directory, because that's where the user.config.{..} get configured.
CONFIGURE_FILE("etc/config.icebox.in" "${CMAKE_CURRENT_SOURCE_DIR}/config.icebox")
CONFIGURE_FILE("etc/config.service.in" "${CMAKE_CURRENT_SOURCE_DIR}/config.service")
CONFIGURE_FILE("etc/config.client.in" "${CMAKE_CURRENT_SOURCE_DIR}/config.client")
CONFIGURE_FILE("etc/config.admin.in" "${CMAKE_CURRENT_SOURCE_DIR}/config.admin")
CONFIGURE_FILE("etc/config.java_admin.in" "${CMAKE_CURRENT_SOURCE_DIR}/config.java_admin")
CONFIGURE_FILE("etc/config.java_icebox.in" "${CMAKE_CURRENT_SOURCE_DIR}/config.java_icebox")
IF( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/user.config.service")
    FILE( READ "${CMAKE_CURRENT_SOURCE_DIR}/user.config.service" FILE_DATA )
    FILE( APPEND "${CMAKE_CURRENT_SOURCE_DIR}/config.service" ${FILE_DATA} )
ENDIF( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/user.config.service")
IF( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/user.config.client")
    FILE( READ "${CMAKE_CURRENT_SOURCE_DIR}/user.config.client" FILE_DATA )
    FILE( APPEND "${CMAKE_CURRENT_SOURCE_DIR}/config.client" ${FILE_DATA} )
ENDIF( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/user.config.client")
IF( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/user.config.icebox")
    FILE( READ "${CMAKE_CURRENT_SOURCE_DIR}/user.config.icebox" FILE_DATA )
    FILE( APPEND "${CMAKE_CURRENT_SOURCE_DIR}/config.icebox" ${FILE_DATA} )
ENDIF( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/user.config.icebox")

IF( BUILD_WITH_TESTS)
  ENABLE_TESTING()
  SET( BUILD_UNIT_TESTS ON )
  ADD_SUBDIRECTORY(test)
ENDIF( BUILD_WITH_TESTS )

IF( BUILD_UNIT_TESTS)
  FIND_PACKAGE(UnitTest++)
ENDIF( BUILD_UNIT_TESTS )

#
# Packaging for binary or 3rdparty distribution
#
IF ( BUILD_3RDPARTY_PACKAGE )
    ADD_SUBDIRECTORY( etc/pack_3rdparty )
ENDIF( BUILD_3RDPARTY_PACKAGE )
IF ( BUILD_BINARY_PACKAGE )
    ADD_SUBDIRECTORY( etc/pack_binary )
ENDIF ( BUILD_BINARY_PACKAGE )

