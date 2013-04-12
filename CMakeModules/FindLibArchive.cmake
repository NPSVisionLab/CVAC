#Locate the libArchive project

FIND_PATH(LIBARCHIVE_INCLUDE archive.h
          HINTS
              ${CMAKE_SOURCE_DIR}/3rdparty/libarchive
          PATHS
              /opt/local
              "C:/Program Files (x86)/libarchive-3.1.2/libarchive"
              "C:/Program Files (x86)/libarchive-2.8.5/libarchive"
          DOC "Include directory for libarchive"
         )

# search for the libarchive library, first in the CVAC_extras folder,
# then in the location related to the include file, then in default locations
FIND_LIBRARY(LIBARCHIVE_LIBRARY NAMES archive
             HINTS
                ${CMAKE_SOURCE_DIR}/3rdparty/libarchive/lib

             PATHS 
                "C:/Program Files (x86)/libarchive/libarchive/Release"
                "C:/Program Files (x86)/libarchive/libarchive/Debug"
             DOC "Library directory for libarchive"
            )

IF (WIN32)
# We need path to the archive and zlib dlls
FIND_PATH(LIBARCHIVE_BIN_DIR NAMES archive.dll
             HINTS
                ../CVAC_extras/bin
                ${LIBARCHIVE_INCLUDE}/../bin
             PATHS 
                "C:/Program Files (x86)/libarchive/bin"
             DOC "BIN directory for libarchive"
            )
FIND_PATH(LIBZIP_BIN_DIR NAMES zlibd.dll
             HINTS
                ../CVAC_extras/bin
                ${LIBARCHIVE_INCLUDE}/../zlib/bin
             PATHS 
                "C:/Program Files (x86)/zlib/bin"
             DOC "BIN directory for libarchive"
            )
ENDIF (WIN32)

# handle the QUIETLY and REQUIRED arguments and set LibArchive_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibArchive DEFAULT_MSG LIBARCHIVE_INCLUDE
                                                  LIBARCHIVE_LIBRARY
                                                  )
