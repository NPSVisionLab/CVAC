#Locate the libArchive project

FIND_PATH(LIBARCHIVE_INCLUDE archive.h
          HINTS
              ${CMAKE_SOURCE_DIR}/3rdparty/libarchive-3.1.2/libarchive
              ${CMAKE_SOURCE_DIR}/3rdparty/libarchive-2.8.5/libarchive
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
                ${CMAKE_SOURCE_DIR}/3rdparty/libarchive/libarchive/Release
                ${CMAKE_SOURCE_DIR}/3rdparty/libarchive/libarchive/Debug

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