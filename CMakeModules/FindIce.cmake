#Locate the ICE project

INCLUDE(ice_common)

IF(WIN32)
    GET_FILENAME_COMPONENT( ICE_INSTALLDIR "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ZeroC\\Ice 3.4.2;InstallDir]" ABSOLUTE CACHE)
ENDIF(WIN32)

FIND_PATH (ICE_ROOT slice
           HINTS
           $ENV{ICE_ROOT}
           PATHS          
           ${ICE_INSTALLDIR}
           /opt/Ice-3.4
           /usr/include
           /usr/share/Ice-3.4
#           "C:\\Program Files (x86)\\ZeroC\\Ice-3.4.2"
           DOC "The ICE root folder"
           )
           
FIND_PATH (ICE_INCLUDE Slice
           PATH_SUFFIXES include
           HINTS
           ${ICE_ROOT}
           PATHS
           $ENV{ICE_ROOT}
           /opt/Ice-3.4
           )

#where to find the ICE lib dir
IF (MSVC10)
    SET(ICE_LIB_SEARCH_PATH
         ${ICE_ROOT}/lib/vc100
       )
ELSE (MSVC10)
    SET(ICE_LIB_SEARCH_PATH
         ${ICE_ROOT}/lib
       )
ENDIF (MSVC10)

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
    SET(ICE_BOX_LIBRARY_DEBUG ${ICE_LIBRARY})
ENDIF()

FIND_PROGRAM( ICE_BOX_EXECUTABLE
                 NAMES icebox
                 PATHS ${ICE_ROOT}/bin
               )

FIND_PROGRAM( ICE_BOX_ADMIN
                 NAMES iceboxadmin
                 PATHS ${ICE_ROOT}/bin
               )


IF (MSVC10)
   FIND_PROGRAM( ICE_SLICE_EXECUTABLE
                 NAMES slice2cpp
                 PATHS ${ICE_ROOT}/bin/vc100
               )
ELSE (MSVC10)
   FIND_PROGRAM( ICE_SLICE_EXECUTABLE
                 NAMES slice2cpp
                 PATHS ${ICE_ROOT}/bin
               )
ENDIF (MSVC10)
MARK_AS_ADVANCED (ICE_SLICE_EXECUTABLE)
IF (MSVC10)
   FIND_PROGRAM( ICE_SLICE2JAVA_EXECUTABLE
                 NAMES slice2java
                 PATHS ${ICE_ROOT}/bin/vc100
               )
ELSE (MSVC10)
   FIND_PROGRAM( ICE_SLICE2JAVA_EXECUTABLE
                 NAMES slice2java
                 PATHS ${ICE_ROOT}/bin
               )
ENDIF (MSVC10)
MARK_AS_ADVANCED (ICE_SLICE2JAVA_EXECUTABLE)

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
