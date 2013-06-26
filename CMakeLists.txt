cmake_minimum_required(VERSION 2.6)

cmake_policy( SET CMP0012 NEW)

IF(WIN32)
    # don't want the four default configuration types with MS Visual Studio
    SET (CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "")
ENDIF(WIN32)

PROJECT(CVAC)

INCLUDE (UserCMakeLists.txt OPTIONAL)

OPTION(BUILD_WITH_BOW "Build a Bag of Words trainer and detector (requires OpenCV)" ON)
OPTION(BUILD_WITH_OPENCVPERFORMANCE "Build the OpenCV Cascade trainer and detector" ON)
OPTION(BUILD_FILESERVER "Build a File server (generally for servers), requires Java" ON) 
OPTION(BUILD_CORPUS_SERVER "Build a Corpus server (generally for clients), requires Java" ON) 
OPTION(BUILD_CORPUS_LABELME "Build an interface to LabelMe, requires Java and Matlab Compiler Runtime" OFF)
OPTION(BUILD_WITH_TESTS "Build the tests, which depend on UnitTest++" OFF)
OPTION(BUILD_DOCUMENTATION "Build the CVAC API documentation with Doxygen (use 'make doc')" OFF)
OPTION(DOWNLOAD_3RDPARTY_PACKAGE "Attempt to download a platform-specific 3rd-party software package" OFF)
OPTION(BUILD_3RDPARTY_PACKAGE "Build the 3rd-Party development distribution package" OFF)
OPTION(BUILD_BINARY_PACKAGE "Build the binary distribution package" OFF)

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

#
# What version of CVAC?
#
# Try to create the include/version.h file that specifies the CVAC version,
# as obtained from the version-control tag (major.minor) and the
# number of commits since that tag (patch)
if( EXISTS "${CMAKE_SOURCE_DIR}/.git" )
    EXECUTE_PROCESS( COMMAND git describe --long --match "v[0-9]*.[0-9]*"
              OUTPUT_VARIABLE CVAC_VERSION_GIT
              OUTPUT_STRIP_TRAILING_WHITESPACE
              WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    IF ( CVAC_VERSION_GIT )
        # parse this kind of string v1.2-3-gkd3kdj4:
	# CVAC_VERSION_MAJMIN is 1.2
    	# CVAC_VERSION_MAJOR is 1
    	# CVAC_VERSION_MINOR is 2
    	# CVAC_VERSION_PATCH is 3
    	# CVAC_VERSION is 1.2.3
     	STRING( REGEX REPLACE "^v(.*)-.*-.*" "\\1" CVAC_VERSION_MAJMIN ${CVAC_VERSION_GIT} ) 
     	STRING( REGEX REPLACE "([^.]).*" "\\1" CVAC_VERSION_MAJOR ${CVAC_VERSION_MAJMIN} )
     	STRING( REGEX REPLACE ".*([^.])-*" "\\1" CVAC_VERSION_MINOR ${CVAC_VERSION_MAJMIN} )
     	STRING( REGEX REPLACE "^v.*-(.*)-.*" "\\1" CVAC_VERSION_PATCH ${CVAC_VERSION_GIT} )
     	SET( CVAC_VERSION ${CVAC_VERSION_MAJOR}.${CVAC_VERSION_MINOR}.${CVAC_VERSION_PATCH} )
        MESSAGE( STATUS "successfully parsed version from git: CVAC-${CVAC_VERSION}" )
     	CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/include/version.h.in
                                   ${CMAKE_SOURCE_DIR}/include/version.h)
    ENDIF( CVAC_VERSION_GIT )
endif( EXISTS "${CMAKE_SOURCE_DIR}/.git" )

if( NOT EXISTS "${CMAKE_SOURCE_DIR}/.git" OR NOT CVAC_VERSION_GIT )
    # .git directory doesn't exist: now make sure that
    # files are a complete download, not a fragmented collection.
    # In other words, there must be a include/version.h.repo file.
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/include/version.h.repo
          ${CMAKE_SOURCE_DIR}/include/version.h)
    FILE( STRINGS ${CMAKE_SOURCE_DIR}/include/version.h
          versionfile_contents
          REGEX "#define CVAC_VERSION*" )
    STRING( REGEX REPLACE ".*MAJOR ([^ ]+)#.*" "\\1" CVAC_VERSION_MAJOR ${versionfile_contents} )
    STRING( REGEX REPLACE ".*MINOR ([^ ]+)#.*" "\\1" CVAC_VERSION_MINOR ${versionfile_contents} )
    STRING( REGEX REPLACE ".*PATCH ([^ ]+).*" "\\1" CVAC_VERSION_PATCH ${versionfile_contents} )
    SET( CVAC_VERSION ${CVAC_VERSION_MAJOR}.${CVAC_VERSION_MINOR}.${CVAC_VERSION_PATCH} )
    SET( CVAC_VERSION_MAJMIN ${CVAC_VERSION_MAJOR}.${CVAC_VERSION_MINOR} )
    MESSAGE( STATUS "successfully parsed version from version.h: CVAC-${CVAC_VERSION}" )
endif( NOT EXISTS "${CMAKE_SOURCE_DIR}/.git" OR NOT CVAC_VERSION_GIT )

IF( "x${CVAC_VERSION}x" STREQUAL "xx"
    OR "x${CVAC_VERSION_MAJOR}x" STREQUAL "xx"
    OR "x${CVAC_VERSION_MINOR}x" STREQUAL "xx" )
    # need version string to continue for many tasks
    MESSAGE( SEND_ERROR "Could not obtain CVAC version from git or include/version.h:" )
    MESSAGE( SEND_ERROR "all:${CVAC_VERSION} major:${CVAC_VERSION_MAJOR} minor:${CVAC_VERSION_MINOR}" )
ENDIF()


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
        SET( 3RDPARTY_SERVER http://www.movesinstitute.org/~kolsch/CVAC )
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
  FIND_PACKAGE( Doxygen )
  IF( DOXYGEN_FOUND )
      # this is the list of directories that doxygen will search for input files;
      # see Doxygen.in for a list of file types (basically, C, Java, and ICE files)
      set( CMAKE_DOXYGEN_INPUT_LIST
                ${CMAKE_SOURCE_DIR}/README.md
                ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/util
                ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/util
                ${CMAKE_SOURCE_DIR}/src/easy
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

FIND_PACKAGE( PythonInterp )
FIND_PACKAGE(Ice REQUIRED)
FIND_PACKAGE(LibArchive REQUIRED)

# Make sure packages are compatible
# ICE 3.4.2 is pre-built against Python 2.6
IF( PYTHONINTERP_FOUND )
  # make sure it's version 2.6 or else error
  IF( NOT PYTHON_VERSION_MAJOR EQUAL 2 OR NOT PYTHON_VERSION_MINOR EQUAL 6 )
    MESSAGE( SEND_ERROR "Found Python version ${PYTHON_VERSION_STRING} but need 2.6" )
    MESSAGE( SEND_ERROR "Please specify PYTHON_EXECUTABLE manually." )
  ENDIF()
ENDIF( PYTHONINTERP_FOUND )

IF( BUILD_WITH_OPENCVPERFORMANCE OR BUILD_WITH_BOW )
  FIND_PACKAGE( OpenCV 2.4 REQUIRED
                            core imgproc highgui ml features2d nonfree objdetect calib3d
             HINTS
                            ${CMAKE_SOURCE_DIR}/3rdparty/opencv
  )
  MESSAGE( STATUS "Found OpenCV ${OpenCV_VERSION} configuration at ${OpenCV_CONFIG_PATH}" )
ENDIF( BUILD_WITH_OPENCVPERFORMANCE OR BUILD_WITH_BOW )

## Setup the config.service and config.icebox files based upon detectors
## selected by the user.  These CF_ variables are defined in the config files
## and they get turned into comments or not.
SET( CF_MBOST "#" )

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

CONFIGURE_FILE("etc/config.icebox.in" "${CMAKE_CURRENT_SOURCE_DIR}/config.icebox")
CONFIGURE_FILE("etc/config.service.in" "${CMAKE_CURRENT_SOURCE_DIR}/config.service")
IF( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/user.config.service")
    FILE( READ "${CMAKE_CURRENT_SOURCE_DIR}/user.config.service" FILE_DATA )
    FILE( APPEND "${CMAKE_CURRENT_SOURCE_DIR}/config.service" ${FILE_DATA} )
ENDIF( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/user.config.service")
IF( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/user.config.icebox")
    FILE( READ "${CMAKE_CURRENT_SOURCE_DIR}/user.config.icebox" FILE_DATA )
    FILE( APPEND "${CMAKE_CURRENT_SOURCE_DIR}/config.icebox" ${FILE_DATA} )
ENDIF( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/user.config.icebox")

IF( BUILD_CORPUS_SERVER OR BUILD_FILESERVER )
    # This requires CMake 2.8 for Java support
    ADD_SUBDIRECTORY( src )
ENDIF( BUILD_CORPUS_SERVER OR BUILD_FILESERVER )
ADD_SUBDIRECTORY( etc )
ADD_SUBDIRECTORY(src/util)
ADD_SUBDIRECTORY(src/easy)
ADD_SUBDIRECTORY(algorithms)
IF( BUILD_WITH_TESTS)
  #Turn on CTest input files
  # Next include for outputing ctest results to the dash board
  #INCLUDE(CTest) add this to send stuff to dash board
  # Next include for setting up pre and post ctests execution
  #CONFIGURE_FILE(CTestCustom.cmake.in ${CMAKE_BINARY_DIR}/CTestCustom.cmake)
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
