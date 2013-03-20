#Finds the interally located, SVN'd externals
#defines BOW_INCLUDE and BOW_LIBRARIES for use in CMakeLists.txt files

IF (NOT BOW_ROOT)
	SET(BOW_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/ext)
	ENDIF()

FIND_PATH(BOW_INCLUDE_DIR bowCV.h
          PATHS
          ${BOW_ROOT}/inc/bow
          )

MACRO(FIND_BOW_LIB LIB_VAR LIB_NAME)
    FIND_LIBRARY(${LIB_VAR} NAMES ${LIB_NAME}
                 HINTS
                 ${BOW_ROOT}/lib
                 )
ENDMACRO(FIND_BOW_LIB)

FIND_BOW_LIB(BOW_LIBRARY bowCV)
FIND_BOW_LIB(BOW_LIBRARY_DEBUG bowCV_d)

# Make sure that if the debug library is not available, we use the release version instead
IF (NOT BOW_LIBRARY_DEBUG)
    SET(BOW_LIBRARY_DEBUG ${BOW_LIBRARY})
ENDIF()

SET(BOW_LIBRARIES
    optimized ${BOW_LIBRARY} debug ${BOW_LIBRARY_DEBUG}
    )

# handle the QUIETLY and REQUIRED arguments and set ICE_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BOW DEFAULT_MSG 	BOW_ROOT
                                  BOW_INCLUDE_DIR
                                  BOW_LIBRARY
                                  )