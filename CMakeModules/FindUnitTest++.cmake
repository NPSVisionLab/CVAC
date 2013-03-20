# Locate UnitTest++
#

  
FIND_LIBRARY(UNITTEST++_LIBRARY
             NAMES UnitTest++ UnitTest++.vsnet2005
             PATHS
              ${CMAKE_SOURCE_DIR}/3rdparty/UnitTest++/Release
             )

FIND_LIBRARY(UNITTEST++_LIBRARY_DEBUG
             NAMES UnitTest++D UnitTest++.vsnet2005
             PATHS
              ${CMAKE_SOURCE_DIR}/3rdparty/UnitTest++/Debug
             )

IF (NOT UNITTEST++_LIBRARY_DEBUG)
	SET(UNITTEST++_LIBRARY_DEBUG ${UNITTEST++_LIBRARY}
            CACHE PATH "Debug library for UnitTest-cpp"
	    FORCE
)
ENDIF()

FIND_PATH(UNITTEST++_INCLUDE_DIR UnitTest++.h
          HINTS
           ${CMAKE_SOURCE_DIR}/3rdparty/UnitTest++/src
          )

# handle the QUIETLY and REQUIRED arguments and set UNITTEST++_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(UnitTest++ DEFAULT_MSG 
                                             UNITTEST++_LIBRARY
                                             UNITTEST++_LIBRARY_DEBUG
                                             UNITTEST++_INCLUDE_DIR
                                  )
