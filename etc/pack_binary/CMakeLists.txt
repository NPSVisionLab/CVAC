cmake_minimum_required(VERSION 2.8)

#
# CPack Package Properties
#
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
set(CPACK_PACKAGE_NAME "CVAC_binary")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CVAC binary install package")
set(CPACK_PACKAGE_VENDOR "NPS Vision Lab")

IF( NOT DEFINED CVAC_VERSION_MAJOR )
    MESSAGE( FATAL_ERROR "Could not determine CVAC version" )
ENDIF( NOT DEFINED CVAC_VERSION_MAJOR )
set(CPACK_PACKAGE_VERSION_MAJOR ${CVAC_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${CVAC_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${CVAC_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CVAC_VERSION_PATCH}")
set( CVAC_VERSION_DIR CVAC-${CVAC_VERSION} )

# If Windows 8 then add -W8 to version
IF ( ${CMAKE_SYSTEM} STREQUAL "Windows-6.2" )
MESSAGE ("windows 8")
set(CVAC_VERSION_DIR ${CVAC_VERSION_DIR}-Win8)
set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION}-Win8)
ELSE ( ${CMAKE_SYSTEM} STREQUAL "Windows-6.2" )
IF (WIN32)
set(CVAC_VERSION_DIR ${CVAC_VERSION_DIR}-Win7)
set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION}-Win7)
ENDIF (WIN32)
ENDIF ( ${CMAKE_SYSTEM} STREQUAL "Windows-6.2" )

set( CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.txt )
set(CPACK_COMPONENTS_ALL Runtime data ThirdParty)
set(CPACK_COMPONENT_CORE_DISPLAY_NAME "CVAC core runtime components")
set(CPACK_COMPONENT_3RDPARTY_DISPLAY_NAME "CVAC 3rd Party" )
set(CPACK_COMPONENT_DATA_DISPLAY_NAME "CVAC sample data" )

# change the install prefix to CVAC-version, since CPack will install into a zip-style archive
# set(CPACK_SET_DESTDIR true)
# set(CPACK_INSTALL_PREFIX ${CVAC_VERSION_DIR} )

# Binary distribution of CVAC: only built files, binary dependencies,
# python install distribution (containing ice, easy, and cvac, python client
# files), c++ client files (detectorClient and trainerClient).
# Everything required to run CVAC without building from source.

# We need to copy cvac, Ice, and easy to the build directory for packing
# python distribution.

# Set locations and variables
SET (SLICE_OUTPUT_PATH "${SLICE_OUTPUT_PYTHONDIR}")
SET (PYTHON_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/pythonBuild")
SET (PYTHON_PACK_NAME "EasyComputerVision")
FILE (MAKE_DIRECTORY "${PYTHON_BUILD_DIR}")
IF (WIN32)
    SET (PYTHON_PACK_FILE "${PYTHON_PACK_NAME}-${CVAC_VERSION}.win32.zip")
ELSE (WIN32)
    #Fill in XXX with correct name
    SET (PYTHON_PACK_FILE "${PYTHON_PACK_NAME}-${CVAC_VERSION}.XXX.tar")
ENDIF (WIN32)

# First copy over cvac
FILE (GLOB ICE_FILES ${SLICE_OUTPUT_PATH}/*_ice.py)
FILE (COPY ${ICE_FILES} DESTINATION ${PYTHON_BUILD_DIR}/cvac)
FILE (COPY ${SLICE_OUTPUT_PATH}/cvac DESTINATION ${PYTHON_BUILD_DIR}
      PATTERN *.py)

# Next copy over easy. 
FILE (COPY ${SLICE_OUTPUT_PATH}/easy DESTINATION ${PYTHON_BUILD_DIR}
      PATTERN *.py)


# Next copy over ice. Since we need to change directory name
# from python to ice copy over the files
FILE (GLOB ICE_FILES ${ICE_ROOT}/python/*)
FILE (COPY ${ICE_FILES} DESTINATION ${PYTHON_BUILD_DIR}/ice)

# Run setup.py to build the python distribution file
CONFIGURE_FILE(setup.py.in "${PYTHON_BUILD_DIR}/setup.py")
ADD_CUSTOM_COMMAND(OUTPUT ${PYTHON_BUILD_DIR}/timestamp
                  COMMAND ${PYTHON_EXECUTABLE} setup.py bdist
                  COMMAND ${CMAKE_COMMAND} -E touch ${PYTHON_BUILD_DIR}/timestamp
                  DEPENDS ${SLICE_OUTPUT_PATH}/cvac/__init__.py
                  WORKING_DIRECTORY ${PYTHON_BUILD_DIR}
                  )
ADD_CUSTOM_TARGET (TARGET ALL DEPENDS ${PYTHON_BUILD_DIR}/timestamp)
# Select Generator by OS
IF(WIN32)
    SET( SCRIPT_EXT bat )
  SET(CPACK_GENERATOR "NSIS")
ELSE(WIN32)
    SET( SCRIPT_EXT sh )
  SET(CPACK_GENERATOR "TGZ")
ENDIF(WIN32)

INSTALL(FILES ${PYTHON_BUILD_DIR}/dist/${PYTHON_PACK_FILE}
       DESTINATION python
       COMPONENT Runtime 
      )

INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/data/corpus
       DESTINATION data
       COMPONENT data
      )
      
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/data/detectors
       DESTINATION data
       COMPONENT data
      )
      
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/data/trainImg
       DESTINATION data
       COMPONENT data
      )
      
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/data/testImg
       DESTINATION data
       COMPONENT data
      )
      
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/data/corporate_logos
       DESTINATION data
       COMPONENT data
      )
      
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/python
       DESTINATION lib
       COMPONENT Runtime
      )

INSTALL(FILES ${CMAKE_SOURCE_DIR}/include/version.h
       DESTINATION include
       COMPONENT Runtime
      )

INSTALL(FILES ${CMAKE_SOURCE_DIR}/config.admin ${CMAKE_SOURCE_DIR}/config.icebox ${CMAKE_SOURCE_DIR}/config.service ${CMAKE_SOURCE_DIR}/config.java_admin ${CMAKE_SOURCE_DIR}/config.java_icebox ${CMAKE_SOURCE_DIR}/config.client 
       DESTINATION .
       COMPONENT Runtime
      )

INSTALL(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/startServices.${SCRIPT_EXT} ${CMAKE_SOURCE_DIR}/etc/stopServices.${SCRIPT_EXT}
       DESTINATION bin
       COMPONENT Runtime
      )

# pre-written versions of startIcebox and stopIcebox for binary distribution, so it's easier to edit:
INSTALL(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/startServices.${SCRIPT_EXT} ${CMAKE_SOURCE_DIR}/etc/stopServices.${SCRIPT_EXT}
       DESTINATION bin
       COMPONENT Runtime
      )

# ThirdParty dependencies for building CVAC

IF (WIN32)

INSTALL(DIRECTORY   ${ICE_ROOT}/bin
        DESTINATION 3rdParty/ICE
        USE_SOURCE_PERMISSIONS
        COMPONENT ThirdParty
        PATTERN icestorm* EXCLUDE
        PATTERN icegrid* EXCLUDE
        PATTERN *d.dll EXCLUDE
        PATTERN *.exe EXCLUDE
        PATTERN *.pdb EXCLUDE
        )
INSTALL(DIRECTORY   ${ICE_ROOT}/lib
        DESTINATION 3rdParty/ICE
        USE_SOURCE_PERMISSIONS
        COMPONENT ThirdParty
        PATTERN *.pdb EXCLUDE
        PATTERN *.lib EXCLUDE
       )


ELSE (WIN32)

INSTALL(DIRECTORY   ${ICE_ROOT}/lib
        DESTINATION 3rdParty/ICE
        USE_SOURCE_PERMISSIONS
        COMPONENT ThirdParty
        PATTERN *.pdb EXCLUDE
       )

ENDIF (WIN32)

INSTALL(PROGRAMS ${CMAKE_SOURCE_DIR}/3rdparty/ICE/bin/icebox${CMAKE_EXECUTABLE_SUFFIX}
                 ${ICE_BOX_ADMIN}
        DESTINATION 3rdParty/ICE/bin
        COMPONENT ThirdParty
       )

INSTALL(DIRECTORY   ${ICE_ROOT}/python
        DESTINATION 3rdParty/ICE
        USE_SOURCE_PERMISSIONS
        COMPONENT ThirdParty
        PATTERN *.pyc EXCLUDE
       )

# libarchive is a symlink, but we want the file:
# CONFIGURE_FILE( ${LIBARCHIVE_LIBRARY} ${CMAKE_CURRENT_BINARY_DIR}/tmp/ COPYONLY)
GET_FILENAME_COMPONENT( LIBARCHIVE_LIBRARY_REALPATH ${LIBARCHIVE_LIBRARY} REALPATH )
IF (WIN32)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty/libarchive/bin
       DESTINATION 3rdParty/libarchive
       COMPONENT ThirdParty
      )
ELSE (WIN32)
INSTALL(FILES ${LIBARCHIVE_LIBRARY} ${LIBARCHIVE_LIBRARY_REALPATH}
       DESTINATION 3rdParty/libarchive/lib
       COMPONENT ThirdParty
      )
ENDIF (WIN32)

# OpenCV needs to be isolated in some staging area.  You need
# to configure this CMake to pull OpenCV from there.
# OpenCV_DIR points to the shared/OpenCV subdirectory.
IF( WIN32 )
  GET_FILENAME_COMPONENT( OpenCV_INSTALL_PATH "${OpenCV_DIR}" REALPATH)
ELSE( WIN32 )
  GET_FILENAME_COMPONENT( OpenCV_INSTALL_PATH "${OpenCV_DIR}/../.." REALPATH)
ENDIF( WIN32 )
MESSAGE( STATUS "OpenCV will be 3rd-party archived from this path: ${OpenCV_INSTALL_PATH}" )
INSTALL(DIRECTORY ${OpenCV_INSTALL_PATH}/bin
      DESTINATION 3rdParty/opencv
       COMPONENT ThirdParty
      ) 

IF (WIN32)
INSTALL(DIRECTORY ${OpenCV_INSTALL_PATH}/python
      DESTINATION 3rdParty/opencv
       COMPONENT ThirdParty
      ) 
# Add script commands to execute after the install
INSTALL(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/doWinInstall.${SCRIPT_EXT} 
       DESTINATION bin
       COMPONENT Runtime
      )
SET (CPACK_NSIS_EXTRA_INSTALL_COMMANDS "ExecWait '\\\"$INSTDIR\\\\bin\\\\doWinInstall $INSTDIR\\\"'")

ELSE (WIN32)
#If not windows need the libs for the shared libraries
INSTALL(DIRECTORY ${OpenCV_INSTALL_PATH}/lib
      DESTINATION 3rdParty/opencv
       COMPONENT ThirdParty
      ) 
ENDIF(WIN32)

include(CPack)
