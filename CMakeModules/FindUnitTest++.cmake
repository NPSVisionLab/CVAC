# Locate UnitTest++
#

  
FIND_LIBRARY(UNITTEST++_LIBRARY
             NAMES UnitTest++ UnitTest++.vsnet
             HINTS
              ${CMAKE_SOURCE_DIR}/../CVAC_extras/lib
              ${CMAKE_SOURCE_DIR}/../UnitTest++/Release
             )

FIND_LIBRARY(UNITTEST++_LIBRARY_DEBUG
             NAMES UnitTest++D UnitTest++.vsnet 
             HINTS
              ${CMAKE_SOURCE_DIR}/../CVAC_extras/lib
              ${CMAKE_SOURCE_DIR}/../UnitTest++/Debug
             )

IF (NOT UNITTEST++_LIBRARY_DEBUG)
	SET(UNITTEST++_LIBRARY_DEBUG ${UNITTEST++_LIBRARY}
            CACHE PATH "Debug library for UnitTest-cpp"
	    FORCE
)
ENDIF()

FIND_PATH(UNITTEST++_INCLUDE_DIR UnitTest++.h
          HINTS
           ${CMAKE_SOURCE_DIR}/../CVAC_extras/inc
           ${CMAKE_SOURCE_DIR}/../UnitTest++/src
          )

# handle the QUIETLY and REQUIRED arguments and set UNITTEST++_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(UnitTest++ DEFAULT_MSG 
                                             UNITTEST++_LIBRARY
                                             UNITTEST++_LIBRARY_DEBUG
                                             UNITTEST++_INCLUDE_DIR
                                  )
