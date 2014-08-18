#Locate the libArchive project

FIND_PATH(LIBARCHIVE_INCLUDE archive.h archive_entry.h
          HINTS
              ${CMAKE_SOURCE_DIR}/3rdparty/libarchive/include
          PATHS
              /opt/local
              "C:/Program Files (x86)/libarchive-3.1.2/libarchive"
              "C:/Program Files (x86)/libarchive-2.8.5/libarchive"
              "C:/Program Files (x86)/libarchive/include"
          DOC "Include directory for libarchive"
         )

# search for the libarchive library, first in the CVAC/3rdparty folder,
# then in the location related to the include file, then in default locations
FIND_LIBRARY(LIBARCHIVE_LIBRARY NAMES archive
             HINTS
#                ${CMAKE_SOURCE_DIR}/3rdparty/libarchive/lib
                /usr/lib

             PATHS 
                "C:/Program Files (x86)/libarchive/lib"
             DOC "Library directory for libarchive"
            )

IF (WIN32)
# We need path to the archive and zlib dlls
FIND_PATH(LIBARCHIVE_BIN_DIR NAMES archive.dll
             HINTS
                ${LIBARCHIVE_INCLUDE}/../bin
             PATHS 
                "C:/Program Files (x86)/libarchive/bin"
             DOC "BIN directory for libarchive"
            )
FIND_PATH(LIBZIP_BIN_DIR NAMES zlibd.dll
             HINTS
                ${LIBARCHIVE_INCLUDE}/../zlib/bin
             PATHS 
                "C:/Program Files (x86)/zlib/bin"
             DOC "BIN directory for libarchive"
            )
ENDIF (WIN32)

IF (APPLE)
FIND_PATH(LIBARCHIVE_LIB_DIR NAMES libarchive.dylib
             HINTS
                ${LIBARCHIVE_INCLUDE}/../lib
             DOC "lib directory for libarchive"
            )
ENDIF (APPLE)

# handle the QUIETLY and REQUIRED arguments and set LibArchive_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibArchive DEFAULT_MSG LIBARCHIVE_INCLUDE
                                                  LIBARCHIVE_LIBRARY
                                                  )
