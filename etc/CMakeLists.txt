
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

# start Python services if we can find the interpreter:
# CMakeLists in the root dir will set PYTHON_EXECUTABLE if found

IF (WIN32)
    SET( SCRIPT_EXT bat )
    SET (OPENCV_3RDPARTY_DIR "${CMAKE_SOURCE_DIR}/3rdparty/opencv/python/2.6" )
    SET (CV2_EXT "pyd")
ELSE (WIN32)
    SET( SCRIPT_EXT sh )
    SET (OPENCV_3RDPARTY_DIR "${CMAKE_SOURCE_DIR}/3rdparty/opencv/lib/python2.6/site-packages" )
    SET (CV2_EXT "so")
ENDIF (WIN32)

CONFIGURE_FILE( startIcebox.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/startIcebox.${SCRIPT_EXT} )
CONFIGURE_FILE( stopIcebox.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/stopIcebox.${SCRIPT_EXT} )

INCLUDE (UserCMakeLists.txt OPTIONAL)

IF( NOT BUILD_3RDPARTY_PACKAGE )
INSTALL(PROGRAMS
                     ${CMAKE_SOURCE_DIR}/bin/startIcebox.${SCRIPT_EXT}
                     ${CMAKE_SOURCE_DIR}/bin/stopIcebox.${SCRIPT_EXT}
               DESTINATION bin
               COMPONENT Runtime
             )
ENDIF( NOT BUILD_3RDPARTY_PACKAGE )


# We need to copy cvac, Ice, and easy to the build directory for packing
# python distribution.
# Set locations and variables
SET (SLICE_OUTPUT_PATH "${SLICE_OUTPUT_PYTHONDIR}")
SET (PYTHON_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/python")
FILE (MAKE_DIRECTORY "${PYTHON_BUILD_DIR}")
FILE (MAKE_DIRECTORY "${PYTHON_BUILD_DIR}/cvac")
FILE (MAKE_DIRECTORY "${PYTHON_BUILD_DIR}/ice")

# First copy over cvac.  Need to wait for buiid time since these files
# are created by slicing at build time
add_custom_target(copySlice ALL
    COMMAND ${CMAKE_COMMAND} -E copy "${SLICE_OUTPUT_PATH}/Corpus_ice.py" "${PYTHON_BUILD_DIR}/cvac"
    COMMAND ${CMAKE_COMMAND} -E copy "${SLICE_OUTPUT_PATH}/Data_ice.py" "${PYTHON_BUILD_DIR}/cvac"
    COMMAND ${CMAKE_COMMAND} -E copy "${SLICE_OUTPUT_PATH}/Files_ice.py" "${PYTHON_BUILD_DIR}/cvac"
    COMMAND ${CMAKE_COMMAND} -E copy "${SLICE_OUTPUT_PATH}/Services_ice.py" "${PYTHON_BUILD_DIR}/cvac"
    COMMAND ${CMAKE_COMMAND} -E copy "${SLICE_OUTPUT_PATH}/cvac/__init__.py" "${PYTHON_BUILD_DIR}/cvac"
    )

# Next copy over easy. Since these files are not sliced then do it now
FILE (COPY ${SLICE_OUTPUT_PATH}/easy DESTINATION ${PYTHON_BUILD_DIR}
      PATTERN *.py)
# Next copy over opencv files
FILE (COPY ${OPENCV_3RDPARTY_DIR}/cv.py DESTINATION ${PYTHON_BUILD_DIR} )
FILE (COPY ${OPENCV_3RDPARTY_DIR}/cv2.${CV2_EXT} DESTINATION ${PYTHON_BUILD_DIR} )

# Next copy over ice. Since we need to change directory name
# from python to ice copy over the files.  Not sliced so do it now
FILE (GLOB ICE_FILES ${ICE_ROOT}/python/*)
FILE (COPY ${ICE_FILES} DESTINATION ${PYTHON_BUILD_DIR}/ice)

CONFIGURE_FILE(setup.py.in "${PYTHON_BUILD_DIR}/setup.py")
CONFIGURE_FILE(easy.pth "${PYTHON_BUILD_DIR}/easy.pth")
CONFIGURE_FILE(ice.pth "${PYTHON_BUILD_DIR}/ice.pth")
CONFIGURE_FILE(PostInstall.cmake.in ${CMAKE_CURRENT_SOURCE_DIR}/PostInstall.cmake)

INSTALL(SCRIPT "PostInstall.cmake")

IF( NOT BUILD_3RDPARTY_PACKAGE AND NOT BUILD_BINARY_PACKAGE)
ENDIF ()
