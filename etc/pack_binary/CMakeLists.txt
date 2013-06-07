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
# and some sample data

# Select Generator by OS
IF(WIN32)
  SET(CPACK_GENERATOR "NSIS")
ELSE()
  SET(CPACK_GENERATOR "TGZ")
ENDIF(WIN32)

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

INSTALL(PROGRAMS ${CMAKE_SOURCE_DIR}/3rdparty/ICE/bin/icebox.exe
                 ${CMAKE_SOURCE_DIR}/3rdparty/ICE/bin/iceboxd.exe
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

INSTALL(DIRECTORY ${OpenCV_INSTALL_PATH}/python
      DESTINATION 3rdParty/opencv
       COMPONENT ThirdParty
      ) 

#If not windows need the libs for the shared libraries
IF (NOT WIN32)
INSTALL(DIRECTORY ${OpenCV_INSTALL_PATH}/lib
      DESTINATION 3rdParty/opencv
       COMPONENT ThirdParty
      ) 
ENDIF(NOT WIN32)

include(CPack)
