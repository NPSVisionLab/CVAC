cmake_minimum_required(VERSION 2.6)

PROJECT(CVAC)

INCLUDE (UserCMakeLists.txt OPTIONAL)

OPTION(BUILD_WITH_BOW "Enables the projects that depend on the bow library" OFF)
OPTION(BUILD_WITH_OPENCVPERFORMANCE "Enables the projects that depend on the OpencvPerformance library" OFF)
OPTION(BUILD_WITH_TESTS "Build the tests, which depend on UnitTest++" OFF)
OPTION(BUILD_DOCUMENTATION "Build the CVAC API documentation with Doxygen (use 'make doc')" ON)
OPTION(BUILD_3RDPARTY_PACKAGE "Build the 3rd-Party development distribution package" OFF)
OPTION(BUILD_BINARY_PACKAGE "Build the binary distribution package" OFF)

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

FIND_PACKAGE(Ice REQUIRED)
FIND_PACKAGE(LibArchive REQUIRED)

IF( BUILD_WITH_OPENCVPERFORMANCE OR BUILD_WITH_BOW )
  FIND_PACKAGE( OpenCV 2.4 REQUIRED
                            core imgproc highgui ml features2d nonfree objdetect calib3d
             HINTS
                            ${CMAKE_SOURCE_DIR}/3rdparty/opencv
  )
  MESSAGE( STATUS "Found OpenCV ${OpenCV_VERSION} configuration at ${OpenCV_CONFIG_PATH}" )
ENDIF( BUILD_WITH_OPENCVPERFORMANCE OR BUILD_WITH_BOW )

# First see if there is a version file (included in release tarballs),
# then try to create the include/version.h file that specifies the CVAC version,
# as obtained from the version-control tag (major.minor) and the
# number of commits since that tag (patch)
if( EXISTS "${CMAKE_SOURCE_DIR}/.git" )
    EXECUTE_PROCESS( COMMAND git describe --long
              OUTPUT_VARIABLE CVAC_VERSION_GIT
              OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if ( CVAC_VERSION_GIT )
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
     	CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/include/version.h.in
                                   ${CMAKE_SOURCE_DIR}/include/version.h)
    endif( CVAC_VERSION_GIT )
endif( EXISTS "${CMAKE_SOURCE_DIR}/.git" )

ADD_SUBDIRECTORY( etc )
ADD_SUBDIRECTORY(src/util)
IF (BUILD_WITH_BOW OR BUILD_WITH_OPENCVPERFORMANCE)
  ADD_SUBDIRECTORY(algorithms)
ENDIF(BUILD_WITH_BOW OR BUILD_WITH_OPENCVPERFORMANCE)
IF( BUILD_WITH_TESTS)
  #Turn on CTest input files
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
