#Locate the libArchive project

FIND_PATH (LIBARCHIVE_INCLUDE archive.h
           HINTS
           ../CVAC_extras/include
           PATHS
           /opt/local
           DOC "The libArchive root folder"
           )
           
# search for the libarchive library, first in the CVAC_extras folder,
# then in the location related to the include file, then in default locations
FIND_LIBRARY(LIBARCHIVE_LIBRARY NAMES archive
             HINTS
             ../CVAC_extras/lib
             ${LIBARCHIVE_INCLUDE}/../lib
             PATHS
             )
