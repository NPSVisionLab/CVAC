#Locate the ICE project

INCLUDE(ice_common)

IF(WIN32)
    GET_FILENAME_COMPONENT( ICE_INSTALLDIR "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ZeroC\\Ice 3.5.1;InstallDir]" ABSOLUTE CACHE)
    MARK_AS_ADVANCED (ICE_INSTALLDIR)
ENDIF(WIN32)

FIND_PATH (ICE_ROOT slice
           HINTS
           $ENV{ICE_ROOT}
           ${CVAC_ROOT_DIR}/3rdparty/ICE
           PATHS          
           ${ICE_INSTALLDIR}
           /opt/Ice-3.5
           /usr/include
           /usr/share/Ice-3.5
           /usr/share/Ice-3.5.1
           /Library/Developer/Ice-3.5
           DOC "The ICE root folder"
           )
           
FIND_PATH (ICE_INCLUDE Slice/Util.h
           PATH_SUFFIXES include
           HINTS
           ${ICE_ROOT}
           ${CVAC_ROOT_DIR}/3rdparty/ICE
           PATHS
           $ENV{ICE_ROOT}
           /opt/Ice-3.5
           /Library/Developer/Ice-3.5
           )

FIND_PATH (ICE_PYTHON_DIR Ice.py
           PATH_SUFFIXES python
           HINTS
           ${ICE_ROOT}
           ${CVAC_ROOT_DIR}/3rdparty/ICE
           PATHS
           $ENV{ICE_ROOT}
           /opt/Ice-3.4
           /usr/lib/pymodules/python2.7
           )

SET(CDIR "")
IF (APPLE)
    IF (NOT USE_LEGACY_STDC++LIB)
        SET(CDIR "/c++11")
    ENDIF (NOT USE_LEGACY_STDC++LIB)
ENDIF (APPLE)
# If we are running on Windows the we have ice 3.5 so vc10 is default
#IF (${CMAKE_SYSTEM} STREQUAL "Windows-6.2")
#where to find the ICE lib dir
SET(ICE_LIB_SEARCH_PATH
         ${ICE_ROOT}/lib${CDIR}
       )

MACRO(FIND_ICE_LIB LIB_VAR LIB_NAME)
    FIND_LIBRARY(${LIB_VAR} NAMES ${LIB_NAME}
                 HINTS
                 ${ICE_LIB_SEARCH_PATH}
                 )
ENDMACRO(FIND_ICE_LIB LIB_VAR LIB_NAME)

FIND_ICE_LIB(ICE_LIBRARY Ice)
FIND_ICE_LIB(ICE_LIBRARY_DEBUG iced)
FIND_ICE_LIB(ICE_UTIL_LIBRARY IceUtil)
FIND_ICE_LIB(ICE_UTIL_LIBRARY_DEBUG iceutild)
FIND_ICE_LIB(ICE_BOX_LIBRARY IceBox)
FIND_ICE_LIB(ICE_BOX_LIBRARY_DEBUG iceboxd)

#if there are no debug libs (like on linux), just use the release libraries
IF (NOT ICE_LIBRARY_DEBUG)
    SET(ICE_LIBRARY_DEBUG ${ICE_LIBRARY})
ENDIF()

IF (NOT ICE_UTIL_LIBRARY_DEBUG)
    SET(ICE_UTIL_LIBRARY_DEBUG ${ICE_UTIL_LIBRARY})
ENDIF()

IF (NOT ICE_BOX_LIBRARY_DEBUG)
    SET(ICE_BOX_LIBRARY_DEBUG ${ICE_BOX_LIBRARY})
ENDIF()

IF( CMAKE_BUILD_TYPE EQUAL Debug)

    FIND_PROGRAM( ICE_BOX_EXECUTABLE
                 NAMES icebox${CMAKE_DEBUG_POSTFIX}
                 HINTS ${ICE_ROOT}/bin
               )
ELSE ( CMAKE_BUILD_TYPE EQUAL Debug)
    FIND_PROGRAM( ICE_BOX_EXECUTABLE
                 NAMES icebox
                 HINTS ${ICE_ROOT}/bin
               )
ENDIF ( CMAKE_BUILD_TYPE EQUAL Debug)

FIND_PROGRAM( ICE_BOX_ADMIN
                 NAMES iceboxadmin
                 HINTS ${ICE_ROOT}/bin
               )

FIND_PROGRAM( ICE_SLICE_EXECUTABLE
                 NAMES slice2cpp
                 HINTS ${ICE_ROOT}/bin${CDIR}
               )

MARK_AS_ADVANCED (ICE_SLICE_EXECUTABLE)
FIND_PROGRAM( ICE_SLICE2JAVA_EXECUTABLE
                 NAMES slice2java
                 HINTS ${ICE_ROOT}/bin${CDIR}
               )
MARK_AS_ADVANCED (ICE_SLICE2JAVA_EXECUTABLE)
FIND_PROGRAM( ICE_SLICE2PY_EXECUTABLE
              NAMES slice2py
              HINTS ${ICE_ROOT}/bin
                    ${ICE_ROOT}/bin/vc100
            )
MARK_AS_ADVANCED (ICE_SLICE2PY_EXECUTABLE ICE_ROOT)

SET(ICE_LIBRARIES
    optimized ${ICE_LIBRARY} debug ${ICE_LIBRARY_DEBUG}
    optimized ${ICE_UTIL_LIBRARY} debug ${ICE_UTIL_LIBRARY_DEBUG}
    optimized ${ICE_BOX_LIBRARY} debug ${ICE_BOX_LIBRARY_DEBUG}
    )

# handle the QUIETLY and REQUIRED arguments and set ICE_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ICE DEFAULT_MSG ICE_INCLUDE
                                                  ICE_LIBRARY
                                                  ICE_UTIL_LIBRARY
                                                  ICE_BOX_LIBRARY
                                                  )
