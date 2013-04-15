cmake_minimum_required(VERSION 2.8)

#
# CPack Package Properties
#
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
set(CPACK_PACKAGE_NAME "CVAC_binary")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CVAC binary install package")

set(CPACK_PACKAGE_VENDOR "NPS Vision Lab") 
set(CPACK_PACKAGE_VERSION_MAJOR ${CVAC_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${CVAC_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${CVAC_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set( CVAC_VERSION_DIR CVAC-${CVAC_VERSION} )

# Binary distribution of CVAC: only built files and binary dependencies
#INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/src/Corpus.jar
#		DESTINATION Devel
#		COMPONENT ExportJars
#		)
#INSTALL(FILES lib/utilsd.lib
#		DESTINATION Devel
#		COMPONENT ExportLibs
#		)

# SampleData
#INSTALL(DIRECTORY data/testImg
#		 DESTINATION SampleData/data
#		 COMPONENT TestImagery
#		)
#INSTALL(DIRECTORY data/trainImg
#		 DESTINATION SampleData/data
#		 COMPONENT TrainImagery
#		)

# Select Generator by OS
IF(WIN32)
  SET(CPACK_GENERATOR "ZIP")
ELSE()
  SET(CPACK_GENERATOR "TGZ")
ENDIF(WIN32)

include(CPack)

INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/include
       DESTINATION ${CVAC_VERSION_DIR}
       COMPONENT IncludeExt
       PATTERN version.h.in EXCLUDE
      )
include(CPack)

# Declare Binary Group
cpack_add_component_group (
  BinaryGrp
  DISPLAY_NAME "binary"
  DESCRIPTION "Binary CVAC components"
)
 cpack_add_component (
  IncludeExt
  DISPLAY_NAME "include files"
  DESCRIPTION "Required to use the CVAC API."
  GROUP BinaryGrp
)
