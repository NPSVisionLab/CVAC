
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

IF(BUILD_BINARY_PACKAGE)
    IF (WIN32)
        SET(INSTALL_PATH "__INSTALL_PATH__")
        SET(PYTHON_PATH "__PYTHON_PATH__")
    ELSE (WIN32)
        SET(INSTALL_PATH "")
        SET(PYTHON_PATH "")
    ENDIF (WIN32)
ELSE(BUILD_BINARY_PACKAGE)
    SET(INSTALL_PATH "${CMAKE_INSTALL_PREFIX}")
    SET(PYTHON_PATH "${PYTHON_EXECUTABLE}")
ENDIF(BUILD_BINARY_PACKAGE)

CONFIGURE_FILE( startIcebox.${SCRIPT_EXT}.in  ${CMAKE_BINARY_DIR}/bin/startIcebox.${SCRIPT_EXT} )
CONFIGURE_FILE( stopIcebox.${SCRIPT_EXT}.in  ${CMAKE_BINARY_DIR}/bin/stopIcebox.${SCRIPT_EXT} )
CONFIGURE_FILE( startServices.${SCRIPT_EXT}.in  ${CMAKE_BINARY_DIR}/bin/startServices.${SCRIPT_EXT} @ONLY )
CONFIGURE_FILE( stopServices.${SCRIPT_EXT}.in  ${CMAKE_BINARY_DIR}/bin/stopServices.${SCRIPT_EXT} @ONLY )


IF( NOT BUILD_3RDPARTY_PACKAGE AND NOT BUILD_BINARY_PACKAGE)
        INSTALL(PROGRAMS
                     ${CMAKE_BINARY_DIR}/bin/startServices.${SCRIPT_EXT}
                     ${CMAKE_BINARY_DIR}/bin/stopServices.${SCRIPT_EXT}
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
    IF (EXISTS "${CMAKE_BINARY_DIR}/python.config")
        #If we don't have the string corpus_service add it
        SET (FILE_STRINGS "")
        FILE(STRINGS "${CMAKE_BINARY_DIR}/python.config" FILE_STRINGS  
             REGEX "src/easy/corpus_service.py")
        IF (FILE_STRINGS STREQUAL "")
            FILE(APPEND "${CMAKE_BINARY_DIR}/python.config" 
                 "src/easy/corpus_service.py\n")
        ENDIF (FILE_STRINGS STREQUAL "")
    ELSE (EXISTS "${CMAKE_BINARY_DIR}/python.config")
        FILE(WRITE "${CMAKE_BINARY_DIR}/python.config" 
             " src/easy/corpus_service.py\n")
    ENDIF (EXISTS "${CMAKE_BINARY_DIR}/python.config")
ENDIF( BUILD_CORPUS_SERVICE_PYTHON)

IF( BUILD_FILESERVER_PYTHON)
    # Add to the python config file since the python fileserver requires it.
    # If it already exists add to the end else create it
    IF (EXISTS "${CMAKE_BINARY_DIR}/python.config")
        #If we don't have the string fileservice add it
        SET (FILE_STRINGS "")
        FILE(STRINGS "${CMAKE_BINARY_DIR}/python.config" FILE_STRINGS  
             REGEX "src/easy/fileserver.py")
        IF (FILE_STRINGS STREQUAL "")
            FILE(APPEND "${CMAKE_BINARY_DIR}/python.config" 
                 " src/easy/fileserver.py\n")
        ENDIF (FILE_STRINGS STREQUAL "")
    ELSE (EXISTS "${CMAKE_BINARY_DIR}/python.config")
        FILE(WRITE "${CMAKE_BINARY_DIR}/python.config" 
             "src/easy/fileserver.py\n")
    ENDIF (EXISTS "${CMAKE_BINARY_DIR}/python.config")
ENDIF( BUILD_FILESERVER_PYTHON)

IF( BUILD_FILESERVICE_PYTHON OR BUILD_CORPUS_SERVICE_PYTHON)
    IF (NOT BUILD_3RDPARTY_PACKAGE)
        INSTALL(FILES ${CMAKE_BINARY_DIR}/python.config
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
CONFIGURE_FILE(PostInstall.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/PostInstall.cmake @ONLY)

IF (NOT PYTHON_ONLY_CLIENT OR BUILD_CORPUS_SERVICE_PYTHON)
    INSTALL(FILES ${CMAKE_BINARY_DIR}/config.admin ${CMAKE_BINARY_DIR}/config.icebox ${CMAKE_BINARY_DIR}/config.service ${CMAKE_BINARY_DIR}/config.java_admin ${CMAKE_BINARY_DIR}/config.java_icebox ${CMAKE_BINARY_DIR}/config.client 
       DESTINATION .
       COMPONENT Runtime
      )
    INSTALL(PROGRAMS ${CMAKE_BINARY_DIR}/bin/startServices.${SCRIPT_EXT} ${CMAKE_BINARY_DIR}/bin/stopServices.${SCRIPT_EXT}
       DESTINATION bin
       COMPONENT Runtime
      )
ENDIF (NOT PYTHON_ONLY_CLIENT OR BUILD_CORPUS_SERVICE_PYTHON)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/test/run_demos.py 
       DESTINATION test
       COMPONENT Runtime
      )

# 3rdparty dependencies for building CVAC
INSTALL(DIRECTORY   ${ICE_ROOT}/include  ${ICE_ROOT}/slice
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
        PATTERN vc110 EXCLUDE
        PATTERN vc120 EXCLUDE
        PATTERN x64 EXCLUDE
       )
ELSE(WIN32)
    IF (APPLE)
        INSTALL(DIRECTORY  ${ICE_ROOT}/lib
            DESTINATION 3rdparty/ICE
            USE_SOURCE_PERMISSIONS
            COMPONENT IceExt
            FILE_MATCHING PATTERN "*.dylib"
           )
    ELSE (APPLE)
        INSTALL(DIRECTORY  ${ICE_ROOT}/lib
            DESTINATION 3rdparty/ICE
            USE_SOURCE_PERMISSIONS
            COMPONENT IceExt
            FILES_MATCHING PATTERN "*.so"
           )
    ENDIF (APPLE)
ENDIF(WIN32)

SET (SVM_DIR "${CMAKE_SOURCE_DIR}/3rdparty/libsvm")
IF(APPLE)
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
    INSTALL(FILES ${SVM_DIR}/libsvm.so.2
              ${SVM_DIR}/setup.py
              ${SVM_DIR}/svm.py
              ${SVM_DIR}/svmutil.py
        DESTINATION 3rdparty/libsvm
        COMPONENT LibSVMExt
       )

ELSE(APPLE)

    INSTALL(PROGRAMS ${ICE_SLICE_EXECUTABLE}
                 ${ICE_SLICE2JAVA_EXECUTABLE} 
                 ${ICE_SLICE2PY_EXECUTABLE}
                 ${ICE_BOX_EXECUTABLE}
                 ${ICE_BOX_ADMIN}
        DESTINATION 3rdparty/ICE/bin
        COMPONENT IceExt
       )
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
    INSTALL(FILES ${SVM_DIR}/libsvm${CMAKE_SHARED_LIBRARY_SUFFIX}
              ${SVM_DIR}/setup.py
              ${SVM_DIR}/svm.py
              ${SVM_DIR}/svmutil.py
        DESTINATION 3rdparty/libsvm
        COMPONENT LibSVMExt
       )

    ENDIF( WIN32 )

ENDIF(APPLE)



IF (NOT PYTHON_ONLY_CLIENT)
    INSTALL(PROGRAMS ${CMAKE_SOURCE_DIR}/3rdparty/ICE/bin/icebox${CMAKE_EXECUTABLE_SUFFIX}
                 ${ICE_BOX_ADMIN}
        DESTINATION 3rdparty/ICE/bin
        COMPONENT ThirdParty
       )

    # OpenCV needs to be isolated in some staging area.  You need
    # to configure this CMake to pull OpenCV from there.
    # OpenCV_DIR points to the shared/OpenCV subdirectory.
    IF( WIN32 )
      GET_FILENAME_COMPONENT( OpenCV_INSTALL_PATH "${OpenCV_DIR}" REALPATH)
    ELSE( WIN32 )
      GET_FILENAME_COMPONENT( OpenCV_INSTALL_PATH "${OpenCV_DIR}/../.." REALPATH)
    ENDIF( WIN32 )
    MESSAGE( STATUS "OpenCV will be 3rd-party archived from this path: ${OpenCV_INSTALL_PATH}" )

    IF (WIN32)
    INSTALL(DIRECTORY ${OpenCV_INSTALL_PATH}/x86
      DESTINATION 3rdparty/opencv
       COMPONENT ThirdParty
      )
    INSTALL(DIRECTORY ${OpenCV_INSTALL_PATH}/python
          DESTINATION 3rdparty/opencv
           COMPONENT ThirdParty
          ) 

    ELSE (WIN32)
    INSTALL(DIRECTORY ${OpenCV_INSTALL_PATH}/bin
      DESTINATION 3rdparty/opencv
       COMPONENT ThirdParty
      )
    #If not windows need the libs for the shared libraries
    INSTALL(DIRECTORY ${OpenCV_INSTALL_PATH}/lib
          DESTINATION 3rdparty/opencv
           COMPONENT ThirdParty
          ) 
    ENDIF(WIN32)

ENDIF (NOT PYTHON_ONLY_CLIENT)

INSTALL(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/PostInstall.cmake")

IF( NOT BUILD_3RDPARTY_PACKAGE AND NOT BUILD_BINARY_PACKAGE)
ENDIF ()
