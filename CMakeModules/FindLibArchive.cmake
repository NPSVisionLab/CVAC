#Locate the libArchive project

FIND_PATH(LIBARCHIVE_INCLUDE archive.h
          HINTS
              ../CVAC_extras/include
          PATHS
              /opt/local
              "C:/Program Files (x86)/libarchive/include"
          DOC "Include directory for libarchive"
         )
           
# search for the libarchive library, first in the CVAC_extras folder,
# then in the location related to the include file, then in default locations
FIND_LIBRARY(LIBARCHIVE_LIBRARY NAMES archive
             HINTS
                ../CVAC_extras/lib
                ${LIBARCHIVE_INCLUDE}/../lib
             PATHS 
                "C:/Program Files (x86)/libarchive/lib"
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
