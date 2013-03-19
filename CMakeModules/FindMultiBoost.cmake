#Finds the interally located, SVN'd externals, MultiBoost project
#defines MULTIBOOST_INCLUDE and MULTIBOOST_LIBRARIES for use in CMakeLists.txt files

FIND_PATH(MULTIBOOST_ROOT MultiBoost          
          PATHS
          ${CMAKE_CURRENT_SOURCE_DIR}/ext
          )

FIND_PATH(MULTIBOOST_INCLUDE MultiBoost.h
          PATHS 
          ${MULTIBOOST_ROOT}/MultiBoost
          )

MACRO(FIND_MULTIBOOST_LIB LIB_VAR LIB_NAME)
    FIND_LIBRARY(${LIB_VAR} NAMES ${LIB_NAME}
                 HINTS
                 ${MULTIBOOST_ROOT}
                 ${MULTIBOOST_ROOT}/build/Debug
                 ${MULTIBOOST_ROOT}/build/Release
                 )
ENDMACRO(FIND_MULTIBOOST_LIB)

FIND_MULTIBOOST_LIB(MULTIBOOST_LIBRARY mboost)
FIND_MULTIBOOST_LIB(MULTIBOOST_LIBRARY_DEBUG mboostd)
FIND_MULTIBOOST_LIB(MULTIBOOST_TRAINER_LIB mboost_train)
FIND_MULTIBOOST_LIB(MULTIBOOST_TRAINER_LIB_DEBUG mboost_traind)

IF (NOT MULTIBOOST_LIBRARY_DEBUG)
    SET(MULTIBOOST_LIBRARY_DEBUG ${MULTIBOOST_LIBRARY})
ENDIF()

IF (NOT MULTIBOOST_TRAINER_LIB_DEBUG)
    SET(MULTIBOOST_TRAINER_LIB_DEBUG ${MULTIBOOST_TRAINER_LIB})
ENDIF()
IF (MULTIBOOST_TRAINER_LIB AND MULTIBOOST_TRAINER_LIB_DEBUG)
  SET(MULTIBOOST_TRAINER_LIBRARIES
      optimized ${MULTIBOOST_TRAINER_LIB} debug ${MULTIBOOST_TRAINER_LIB_DEBUG}
     )
ENDIF()

SET(MULTIBOOST_LIBRARIES
    optimized ${MULTIBOOST_LIBRARY}     debug ${MULTIBOOST_LIBRARY_DEBUG}
    )
    

# handle the QUIETLY and REQUIRED arguments and set ICE_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MULTIBOOST DEFAULT_MSG MULTIBOOST_ROOT
                                                         MULTIBOOST_INCLUDE
                                                         MULTIBOOST_LIBRARY
                                                  )