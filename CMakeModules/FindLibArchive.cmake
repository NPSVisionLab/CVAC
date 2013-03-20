#Locate the libArchive project

FIND_PATH(LIBARCHIVE_INCLUDE archive.h
           PATHS "/opt/local/libarchive/include" 
                 "c:/Program files (x86)/libarchive/include"
           DOC "Include directory for libarchive"
           )
           
# search for the libarchive library, first in the CVAC_extras folder,
# then in the location related to the include file, then in default locations
FIND_LIBRARY(LIBARCHIVE_LIBRARY NAMES archive
             HINTS
             ../CVAC_extras/lib
             ${LIBARCHIVE_INCLUDE}/../lib
             PATHS "${SEARCH_PATH}/libarchive/lib"
             DOC "Library directory for libarchive"
             )
