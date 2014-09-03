# Locate the Jekyll web page builder

FIND_PROGRAM( JEKYLL_EXECUTABLE jekyll
              HINTS
              $ENV{JEKYLL_ROOT}
              PATHS          
              /usr/bin
              DOC "The Jekyll executable"
           )
           
# handle the QUIETLY and REQUIRED arguments and set ICE_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS( JEKYLL DEFAULT_MSG
                                   JEKYLL_EXECUTABLE
                                   )
