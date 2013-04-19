cmake_minimum_required(VERSION 2.8)

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
INSTALL(DIRECTORY   ${ICE_ROOT}  # Deep copy
        DESTINATION 3rdparty
        USE_SOURCE_PERMISSIONS
        COMPONENT IceExt
       )
INSTALL(DIRECTORY   ${UNITTEST++_INCLUDE_DIR}
        DESTINATION 3rdparty/UnitTest++/include
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

# OpenCV needs to be isolated in some staging area.  You need
# to configure this CMake to pull OpenCV from there.
# OpenCV_DIR points to the shared/OpenCV subdirectory.
GET_FILENAME_COMPONENT( OpenCV_INSTALL_PATH "${OpenCV_DIR}" REALPATH)
INSTALL(DIRECTORY ${OpenCV_INSTALL_PATH}
      DESTINATION 3rdparty
       COMPONENT OpenCVExt
      ) 

INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/include
       DESTINATION ${CVAC_VERSION_DIR}
       COMPONENT IncludeExt
       PATTERN version.h.in EXCLUDE
      )
      
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
