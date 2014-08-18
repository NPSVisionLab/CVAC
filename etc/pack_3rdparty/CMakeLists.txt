cmake_minimum_required(VERSION 2.6)

#
# CPack Package Properties
#
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
set(CPACK_PACKAGE_NAME "CVAC_3rdparty")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CVAC 3rdparty install package")

set(CPACK_PACKAGE_VENDOR "NPS Vision Lab") 
set(CPACK_PACKAGE_VERSION_MAJOR ${CVAC_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${CVAC_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${CVAC_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
set( CVAC_VERSION_DIR CVAC-${CVAC_VERSION} )

set( CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.txt )

# 3rdparty dependencies for building CVAC
INSTALL(DIRECTORY   ${ICE_ROOT}/include ${ICE_ROOT}/lib ${ICE_ROOT}/slice
        DESTINATION 3rdparty/ICE
        USE_SOURCE_PERMISSIONS
        COMPONENT IceExt
        PATTERN *.pdb EXCLUDE
       )
INSTALL(DIRECTORY   ${ICE_ROOT}/python
        DESTINATION 3rdparty/ICE
        USE_SOURCE_PERMISSIONS
        COMPONENT IceExt
        PATTERN *.pyc EXCLUDE
       )
#Windows needs all the dll files
IF(WIN32)
INSTALL(DIRECTORY   ${ICE_ROOT}/bin
        DESTINATION 3rdparty/ICE
        USE_SOURCE_PERMISSIONS
        COMPONENT IceExt
        PATTERN *.dll
       )
#Windows also needs numpy install exe
INSTALL(FILES    ${CMAKE_SOURCE_DIR}/3rdparty/numpy-1.8.1-win32-superpack-python2.7.exe
        DESTINATION 3rdparty
        COMPONENT NumpyExt
       )
#Windows also needs python install exe
INSTALL(FILES    ${CMAKE_SOURCE_DIR}/3rdparty/python-2.7.8.msi
        DESTINATION 3rdparty
        COMPONENT NumpyExt
       )
ENDIF(WIN32)

IF(APPLE)
# OSX also needs numpy package we will install the whl into the virtual env
INSTALL(FILES    ${CMAKE_SOURCE_DIR}/3rdparty/numpy-1.8.1-cp27-none-any.whl
        DESTINATION 3rdparty
        COMPONENT NumpyExt
       )
INSTALL(DIRECTORY    ${CMAKE_SOURCE_DIR}/3rdparty/virtualenv-1.11.6
        DESTINATION 3rdparty
        COMPONENT NumpyExt
        USE_SOURCE_PERMISSIONS
       )
#Need to install the stdc++ binaries if that option gets turned on
INSTALL(DIRECTORY  ${ICE_ROOT}/bin
        DESTINATION 3rdparty/ICE
        COMPONENT IceExt
        USE_SOURCE_PERMISSIONS
       )
# need to install the opencv stdc++ library files
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty/opencv_stdc
      DESTINATION 3rdparty
       COMPONENT OpenCVExt
        USE_SOURCE_PERMISSIONS
      ) 
# need to install the opencv ++11 library files
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty/opencv
      DESTINATION 3rdparty
       COMPONENT OpenCVExt
        USE_SOURCE_PERMISSIONS
      ) 
INSTALL(DIRECTORY   ${CMAKE_SOURCE_DIR}/3rdparty/UnitTest++
        DESTINATION 3rdparty
        COMPONENT UnitTestPPExt
        USE_SOURCE_PERMISSIONS
      )
INSTALL(DIRECTORY   ${CMAKE_SOURCE_DIR}/3rdparty/UnitTest++_stdc
        DESTINATION 3rdparty
        COMPONENT UnitTestPPExt
        USE_SOURCE_PERMISSIONS
      )

ELSE (APPLE)

INSTALL(PROGRAMS ${ICE_SLICE_EXECUTABLE}
                 ${ICE_SLICE2JAVA_EXECUTABLE} 
                 ${ICE_SLICE2PY_EXECUTABLE}
                 ${ICE_BOX_EXECUTABLE}
                 ${ICE_BOX_ADMIN}
        DESTINATION 3rdparty/ICE/bin
        COMPONENT IceExt
       )
# this gets us into trouble with symlinks
#INSTALL(FILES ${ICE_LIBRARY} ${ICE_LIBRARY_DEBUG}
#              ${ICE_UTIL_LIBRARY} ${ICE_UTIL_LIBRARY_DEBUG}
#              ${ICE_BOX_LIBRARY} ${ICE_BOX_LIBRARY_DEBUG}
#        DESTINATION 3rdparty/ICE/lib
#        COMPONENT IceExt
#       )
INSTALL(DIRECTORY   ${UNITTEST++_INCLUDE_DIR}
        DESTINATION 3rdparty/UnitTest++
        COMPONENT UnitTestPPExt
       )
INSTALL(FILES    ${UNITTEST++_LIBRARY}
		 ${UNITTEST++_LIBRARY_DEBUG}
        DESTINATION 3rdparty/UnitTest++/lib
        COMPONENT UnitTestPPExt
       )
INSTALL(FILES ${LIBARCHIVE_INCLUDE}/archive.h
              ${LIBARCHIVE_INCLUDE}/archive_entry.h
        DESTINATION 3rdparty/libarchive/include
        COMPONENT LibArchiveExt
       )

# libarchive is a symlink, but we want the file:
# CONFIGURE_FILE( ${LIBARCHIVE_LIBRARY} ${CMAKE_CURRENT_BINARY_DIR}/tmp/ COPYONLY)
GET_FILENAME_COMPONENT( LIBARCHIVE_LIBRARY_REALPATH ${LIBARCHIVE_LIBRARY} REALPATH )
INSTALL(FILES ${LIBARCHIVE_LIBRARY} ${LIBARCHIVE_LIBRARY_REALPATH}
       DESTINATION 3rdparty/libarchive/lib
       COMPONENT LibArchiveExt
      )

IF( WIN32 )
INSTALL(DIRECTORY ${LIBARCHIVE_BIN_DIR}
       DESTINATION 3rdparty/libarchive
       COMPONENT LibArchiveExt
      )
#Windows also needs python virtual env package so we don't need root access
INSTALL(DIRECTORY    ${CMAKE_SOURCE_DIR}/3rdparty/virtualenv-1.11.6
        DESTINATION 3rdparty
        COMPONENT NumpyExt
        USE_SOURCE_PERMISSIONS
       )
INSTALL(FILES ${CMAKE_SOURCE_DIR}/3rdparty/numpy-1.8.1-sse3.exe
       DESTINATION 3rdparty
       COMPONENT  NumpyExt
      )

ENDIF( WIN32 )


# OpenCV needs to be isolated in some staging area.  You need
# to configure this CMake to pull OpenCV from there.
# OpenCV_DIR points to the shared/OpenCV subdirectory.
IF( WIN32 )
  GET_FILENAME_COMPONENT( OpenCV_INSTALL_PATH "${OpenCV_DIR}" REALPATH)
ELSE( WIN32 )
  GET_FILENAME_COMPONENT( OpenCV_INSTALL_PATH "${OpenCV_DIR}/../.." REALPATH)
ENDIF( WIN32 )
MESSAGE( STATUS "OpenCV will be 3rd-party archived from this path: ${OpenCV_INSTALL_PATH}" )
INSTALL(DIRECTORY ${OpenCV_INSTALL_PATH}
      DESTINATION 3rdparty
       COMPONENT OpenCVExt
      ) 

ENDIF(APPLE)

include(CPack)

# Declare Third Party Group
cpack_add_component_group (
  ThirdPartyGrp
  DISPLAY_NAME "3rd Party"
  DESCRIPTION "Third-party dependencies including ICE and OpenCV"
)
#Assign GROUP to COMPONENT
cpack_add_component (
  IceExt
  DISPLAY_NAME "ICE"
  DESCRIPTION "Internet Communication Engine"
  GROUP ThirdPartyGrp
)
cpack_add_component (
  NumpyExt
  DISPLAY_NAME "Numpy"
  DESCRIPTION "Python numeric processing package"
  GROUP ThirdPartyGrp
)
cpack_add_component (
  PythonExt
  DISPLAY_NAME "Python"
  DESCRIPTION "Python package"
  GROUP ThirdPartyGrp
)
cpack_add_component (
  UnitTestPPExt
  DISPLAY_NAME "UnitTest++"
  DESCRIPTION "Unit test for C++ framework"
  GROUP ThirdPartyGrp
)
cpack_add_component (
  LibArchiveExt
  DISPLAY_NAME "LibArchive"
  DESCRIPTION "LibArchive library for multi-format archival/unarchival"
  GROUP ThirdPartyGrp
)
cpack_add_component (
  OpenCVExt
  DISPLAY_NAME "OpenCV"
  DESCRIPTION "Open Computer Vision Library"
  GROUP ThirdPartyGrp
)
