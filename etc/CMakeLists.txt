
#
# produce run scripts, put them in the binary folder
#

IF( BUILD_CORPUS_SERVER OR BUILD_FILESERVER )
  cmake_minimum_required(VERSION 2.8.6)
  # Java support since CMake 2.8.6
  FIND_PACKAGE( Java )
  INCLUDE( UseJava )
  SET( START_JAVA_SERVICES 1 )
  IF( NOT Java_JAVA_EXECUTABLE )
    MESSAGE( SEND_ERROR "Need Java Runtime to start java services" )
  ENDIF( NOT Java_JAVA_EXECUTABLE )
  IF( NOT Ice.jar )
    # Ice.jar is found in the src/CMakeLists.txt file, so make
    # sure that the src subdirectory is included before the etc subdir
    MESSAGE( SEND_ERROR "Could not find Ice.jar, but need it for starting the Java IceBox." )
  ENDIF()
ELSE( BUILD_CORPUS_SERVER OR BUILD_FILESERVER )
  SET( START_JAVA_SERVICES 0 )
ENDIF( BUILD_CORPUS_SERVER OR BUILD_FILESERVER )


IF (WIN32)
    SET( SCRIPT_EXT bat )
    SET( PYTHON_EXT pyd )
ELSE (WIN32)
    IF (APPLE)
        Set (ICEPLIBS "${ICEDIR}/lib")
        IF (USE_LEGACY_STDC++LIBS)
            SET (ICELIBS "${ICEDIR}/lib")
        ELSE (USE_LEGACY_STDC++LIBS)
            SET (ICELIBS "${ICEDIR}/lib/c++11")
        ENDIF (USE_LEGACY_STDC++LIBS)
    ENDIF (APPLE)
    SET( SCRIPT_EXT sh )
    SET( PYTHON_EXT so )
ENDIF (WIN32)

CONFIGURE_FILE( startIcebox.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/startIcebox.${SCRIPT_EXT} )
CONFIGURE_FILE( stopIcebox.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/stopIcebox.${SCRIPT_EXT} )
CONFIGURE_FILE( startServices.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/startServices.${SCRIPT_EXT} @ONLY )
CONFIGURE_FILE( stopServices.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/stopServices.${SCRIPT_EXT} @ONLY )


IF( NOT BUILD_3RDPARTY_PACKAGE AND NOT BUILD_BINARY_PACKAGE)
        INSTALL(PROGRAMS
                     ${CMAKE_SOURCE_DIR}/bin/startIcebox.${SCRIPT_EXT}
                     ${CMAKE_SOURCE_DIR}/bin/stopIcebox.${SCRIPT_EXT}
               DESTINATION bin
               COMPONENT Runtime
         )
ENDIF( NOT BUILD_3RDPARTY_PACKAGE AND NOT BUILD_BINARY_PACKAGE)

INCLUDE (UserCMakeLists.txt OPTIONAL)

# This code has to be after the UserCMakeLists.txt include as it modifies
# python.config that the include creates.
IF( BUILD_CORPUS_SERVICE_PYTHON)
    # Add to the python config file since the python corpus server requires it.
    # If it already exists add to the end else create it
    IF (EXISTS "${CMAKE_SOURCE_DIR}/python.config")
        #If we don't have the string corpus_service add it
        SET (FILE_STRINGS "")
        FILE(STRINGS "${CMAKE_SOURCE_DIR}/python.config" FILE_STRINGS  
             REGEX "src/easy/corpus_service.py")
        IF (FILE_STRINGS STREQUAL "")
            FILE(APPEND "${CMAKE_SOURCE_DIR}/python.config" 
                 "src/easy/corpus_service.py\n")
        ENDIF (FILE_STRINGS STREQUAL "")
    ELSE (EXISTS "${CMAKE_SOURCE_DIR}/python.config")
        FILE(WRITE "${CMAKE_SOURCE_DIR}/python.config" 
             "src/easy/corpus_service.py\n")
    ENDIF (EXISTS "${CMAKE_SOURCE_DIR}/python.config")
ENDIF( BUILD_CORPUS_SERVICE_PYTHON)

IF( BUILD_FILESERVER_PYTHON)
    # Add to the python config file since the python fileserver requires it.
    # If it already exists add to the end else create it
    IF (EXISTS "${CMAKE_SOURCE_DIR}/python.config")
        #If we don't have the string fileservice add it
        SET (FILE_STRINGS "")
        FILE(STRINGS "${CMAKE_SOURCE_DIR}/python.config" FILE_STRINGS  
             REGEX "src/easy/fileserver.py")
        IF (FILE_STRINGS STREQUAL "")
            FILE(APPEND "${CMAKE_SOURCE_DIR}/python.config" 
                 "src/easy/fileserver.py\n")
        ENDIF (FILE_STRINGS STREQUAL "")
    ELSE (EXISTS "${CMAKE_SOURCE_DIR}/python.config")
        FILE(WRITE "${CMAKE_SOURCE_DIR}/python.config" 
             "src/easy/fileserver.py\n")
    ENDIF (EXISTS "${CMAKE_SOURCE_DIR}/python.config")
ENDIF( BUILD_FILESERVER_PYTHON)

IF( BUILD_FILESERVICE_PYTHON OR BUILD_CORPUS_SERVICE_PYTHON)
    IF (NOT BUILD_3RDPARTY_PACKAGE)
        INSTALL(FILES ${CMAKE_SOURCE_DIR}/python.config
           DESTINATION .
           COMPONENT Runtime
           )
    ENDIF (NOT BUILD_3RDPARTY_PACKAGE)
ENDIF( BUILD_FILESERVICE_PYTHON OR BUILD_CORPUS_SERVICE_PYTHON)

# We need to copy cvac, Ice, and easy to the build directory for packing
# python distribution.
# Set locations and variables
SET (SLICE_OUTPUT_PATH "${SLICE_OUTPUT_PYTHONDIR}")
SET (PYTHON_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/python")
FILE (MAKE_DIRECTORY "${PYTHON_BUILD_DIR}")
FILE (MAKE_DIRECTORY "${PYTHON_BUILD_DIR}/easyPkg")
FILE (MAKE_DIRECTORY "${PYTHON_BUILD_DIR}/icePkg")
FILE (MAKE_DIRECTORY "${PYTHON_BUILD_DIR}/easyPkg/cvac")
FILE (MAKE_DIRECTORY "${PYTHON_BUILD_DIR}/easyPkg/easy")



# Next copy over easy. Since these files are not sliced then do it now
FILE (COPY ${SLICE_OUTPUT_PATH}/easy DESTINATION ${PYTHON_BUILD_DIR}/easyPkg
      PATTERN *.py)

SET (PYTHON_PACK_NAME "CVAC")
SET (PYTHON_PACK_EASY_NAME "EasyCV")
SET (PYTHON_PACK_ICE_NAME "EasyCVIce")
# Next copy over ice. Since we need to change directory name
# from python to icePkg, copy over the files.  Not sliced so do it now
FILE (GLOB ICE_FILES ${ICE_PYTHON_DIR}/*)
FILE (COPY ${ICE_FILES} DESTINATION ${PYTHON_BUILD_DIR}/icePkg)
# Copy over the opencv files required by easy to the easy package.
# One day these should be in there own package?  Even those these are
# only required to python trainer/detectors its easier to always include them
FILE (COPY ${OPENCV_PY_PY} DESTINATION ${PYTHON_BUILD_DIR}/easyPkg)
FILE (COPY ${OPENCV_PY_LIB} DESTINATION ${PYTHON_BUILD_DIR}/easyPkg)

CONFIGURE_FILE(setup.easy.py.in "${PYTHON_BUILD_DIR}/easyPkg/setup.py")
CONFIGURE_FILE(setup.ice.py.in "${PYTHON_BUILD_DIR}/icePkg/setup.py")

# For unix set the install prefix for the python command to be the
# same as the cmake install prefix.  We do this since installing directly
# to the python site-packages require root access.
IF (WIN32)
    SET (PREFIX_CMD "")
ELSE (WIN32)
    SET (PREFIX_CMD "--prefix=${CMAKE_INSTALL_PREFIX}")
ENDIF (WIN32)
CONFIGURE_FILE(PostInstall.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/PostInstall.cmake)

INSTALL(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/PostInstall.cmake")

IF( NOT BUILD_3RDPARTY_PACKAGE AND NOT BUILD_BINARY_PACKAGE)
ENDIF ()
