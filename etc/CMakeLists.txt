
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
ELSE (WIN32)
    SET( SCRIPT_EXT sh )
ENDIF (WIN32)

CONFIGURE_FILE( startIcebox.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/startIcebox.${SCRIPT_EXT} )
CONFIGURE_FILE( stopIcebox.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/stopIcebox.${SCRIPT_EXT} )
CONFIGURE_FILE( startServices.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/startServices.${SCRIPT_EXT} )

INCLUDE (UserCMakeLists.txt OPTIONAL)

IF( NOT BUILD_3RDPARTY_PACKAGE AND NOT BUILD_BINARY_PACKAGE)
        INSTALL(PROGRAMS
                     ${CMAKE_SOURCE_DIR}/bin/startIcebox.${SCRIPT_EXT}
                     ${CMAKE_SOURCE_DIR}/bin/stopIcebox.${SCRIPT_EXT}
               DESTINATION bin
               COMPONENT Runtime
         )
ENDIF( NOT BUILD_3RDPARTY_PACKAGE AND NOT BUILD_BINARY_PACKAGE)

IF( BUILD_CORPUS_SERVICE_PYTHON)
    # Add to the python config file since the python corpus server requires it.
    # If it already exists add to the end else create it
    IF (EXISTS "${CMAKE_SOURCE_DIR}/python.config")
        #If we don't have the string corpus_service add it
        SET (FILE_STRINGS "")
        FILE(STRINGS "${CMAKE_SOURCE_DIR}/python.config" FILE_STRINGS  
             REGEX "-m easy.corpus_service.py")
        IF (FILE_STRINGS STREQUAL "")
            FILE(APPEND "${CMAKE_SOURCE_DIR}/python.config" 
                 "-m easy.corpus_service.py")
        ENDIF (FILE_STRINGS STREQUAL "")
    ELSE (EXISTS "${CMAKE_SOURCE_DIR}/python.config")
        FILE(WRITE "${CMAKE_SOURCE_DIR}/python.config" 
             "-m easy.corpus_service.py")
        #Since we created it we have to install as well
        IF (NOT BUILD_3RDPARTY_PACKAGE)
            INSTALL(FILES ${CMAKE_SOURCE_DIR}/python.config
               DESTINATION .
               COMPONENT Runtime
               )
        ENDIF (NOT BUILD_3RDPARTY_PACKAGE)
    ENDIF (EXISTS "${CMAKE_SOURCE_DIR}/python.config")
ENDIF( BUILD_CORPUS_SERVICE_PYTHON)

# We need to copy cvac, Ice, and easy to the build directory for packing
# python distribution.
# Set locations and variables
SET (SLICE_OUTPUT_PATH "${SLICE_OUTPUT_PYTHONDIR}")
SET (PYTHON_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/python")
FILE (MAKE_DIRECTORY "${PYTHON_BUILD_DIR}")
FILE (MAKE_DIRECTORY "${PYTHON_BUILD_DIR}/cvac")
FILE (MAKE_DIRECTORY "${PYTHON_BUILD_DIR}/ice")


# Next copy over easy. Since these files are not sliced then do it now
FILE (COPY ${SLICE_OUTPUT_PATH}/easy DESTINATION ${PYTHON_BUILD_DIR}
      PATTERN *.py)
# Next copy over opencv files
IF( BUILD_WITH_OPENCVPERFORMANCE OR BUILD_WITH_BOW )
  FILE (COPY ${OPENCV_PY_LIB} DESTINATION ${PYTHON_BUILD_DIR} )
  FILE (COPY ${OPENCV_PY_PY} DESTINATION ${PYTHON_BUILD_DIR} )
ENDIF()

# Next copy over ice. Since we need to change directory name
# from python to ice copy over the files.  Not sliced so do it now
FILE (GLOB ICE_FILES ${ICE_ROOT}/python/*)
FILE (COPY ${ICE_FILES} DESTINATION ${PYTHON_BUILD_DIR}/ice)

CONFIGURE_FILE(setup.py.in "${PYTHON_BUILD_DIR}/setup.py")
CONFIGURE_FILE(easy.pth "${PYTHON_BUILD_DIR}/easy.pth")
CONFIGURE_FILE(ice.pth "${PYTHON_BUILD_DIR}/ice.pth")
# For unix set the install prefix for the python command to be the
# same as the cmake install prefix.  We do this since installing directly
# to the python site-packages require root access.
IF (WIN32)
    SET (PREFIX_CMD "")
ELSE (WIN32)
    SET (PREFIX_CMD "--prefix=${CMAKE_INSTALL_PREFIX")
ENDIF (WIN32)
CONFIGURE_FILE(PostInstall.cmake.in ${CMAKE_CURRENT_SOURCE_DIR}/PostInstall.cmake)

INSTALL(SCRIPT "PostInstall.cmake")

IF( NOT BUILD_3RDPARTY_PACKAGE AND NOT BUILD_BINARY_PACKAGE)
ENDIF ()
