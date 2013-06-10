
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
    MESSAGE( SEND_ERROR "Could not find Ice.jar, but need it for starting the Java IceBox." )
  ENDIF()
ELSE( BUILD_CORPUS_SERVER OR BUILD_FILESERVER )
  SET( START_JAVA_SERVICES 0 )
ENDIF( BUILD_CORPUS_SERVER OR BUILD_FILESERVER )

# start Python services if we can find the interpreter:
# CMakeLists in the root dir will set PYTHON_EXECUTABLE if found

IF (WIN32)
    SET( SCRIPT_EXT bat )
ELSE (WIN32)
    SET( SCRIPT_EXT sh )
ENDIF (WIN32)

CONFIGURE_FILE( startIcebox.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/startIcebox.${SCRIPT_EXT} )
CONFIGURE_FILE( stopIcebox.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/stopIcebox.${SCRIPT_EXT} )

IF( NOT BUILD_3RDPARTY_PACKAGE )
INSTALL(PROGRAMS
                     ${CMAKE_SOURCE_DIR}/bin/startIcebox.${SCRIPT_EXT}
                     ${CMAKE_SOURCE_DIR}/bin/stopIcebox.${SCRIPT_EXT}
               DESTINATION bin
               COMPONENT Runtime
             )
ENDIF( NOT BUILD_3RDPARTY_PACKAGE )
