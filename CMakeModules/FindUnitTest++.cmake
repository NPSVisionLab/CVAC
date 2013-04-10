# Locate UnitTest++
#

  
FIND_PATH(UNITTEST++_INCLUDE_DIR UnitTest++.h
          HINTS
           ${CMAKE_SOURCE_DIR}/3rdparty/UnitTest++/src
          PATHS
           /opt/local
          PATH_SUFFIXES UnitTest++
          )

# Needed for CPack INSTALL command, path to library containing-folder
FIND_PATH(UNITTEST++_LIBRARY_FOLDER
             HINTS
                ${CMAKE_SOURCE_DIR}/3rdparty/UnitTest++/Debug
             PATHS
                ${CMAKE_SOURCE_DIR}/3rdparty/UnitTest++/Release
          )

# Needed for CPack INSTALL command, path to library containing-folder
FIND_PATH(UNITTEST++_LIBRARY_FOLDER
             HINTS
                ${CMAKE_SOURCE_DIR}/3rdparty/UnitTest++/Debug
             PATHS
                ${CMAKE_SOURCE_DIR}/3rdparty/UnitTest++/Release
          )

FIND_LIBRARY(UNITTEST++_LIBRARY
             HINTS
                ${CMAKE_SOURCE_DIR}/3rdparty/UnitTest++/Release
             NAMES UnitTest++ UnitTest++.vsnet2005
             PATHS
                ${CMAKE_SOURCE_DIR}/3rdparty/UnitTest++/Release
                /opt/local/lib
             )

FIND_LIBRARY(UNITTEST++_LIBRARY_DEBUG
             NAMES UnitTest++D UnitTest++.vsnet2005
             HINTS
                ${CMAKE_SOURCE_DIR}/3rdparty/UnitTest++/Debug
             PATHS
              ${CMAKE_SOURCE_DIR}/3rdparty/UnitTest++/Debug
             )

IF (NOT UNITTEST++_LIBRARY_DEBUG)
	SET(UNITTEST++_LIBRARY_DEBUG ${UNITTEST++_LIBRARY}
            CACHE PATH "Debug library for UnitTest-cpp"
	    FORCE
)
ENDIF()

# handle the QUIETLY and REQUIRED arguments and set UNITTEST++_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(UnitTest++ DEFAULT_MSG 
                                             UNITTEST++_LIBRARY
                                             UNITTEST++_LIBRARY_DEBUG
                                             UNITTEST++_INCLUDE_DIR
                                  )
