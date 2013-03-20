#Locate the libArchive project

FIND_PATH(LIBARCHIVE_INCLUDE archive.h
           PATHS "/opt/local/libarchive/include" 
                 "c:/Program files (x86)/libarchive/include"
           DOC "Include directory for libarchive"
           )
           
FIND_LIBRARY(LIBARCHIVE_LIBRARY NAMES archive
           PATHS "${SEARCH_PATH}/libarchive/lib"
           DOC "Library directory for libarchive"
             )
