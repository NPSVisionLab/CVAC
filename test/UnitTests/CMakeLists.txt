SET(APP_NAME UnitTests) 

ADD_SUBDIRECTORY( python )

WRAP_SLICE(SLICE_FILES
           ${CVAC_ROOT_DIR}/src/Data.ice
           ${CVAC_ROOT_DIR}/src/Services.ice
           )

SET (CVAC_APP_SOURCE_FILES 
      main.cpp
      FileUtilsTests.cpp
      Data.cpp
      Services.cpp
      ConfusionMatrixTests.cpp
     )

SET(HEADER_FILES
    Data.h
    Services.h
)

INCLUDE_DIRECTORIES( .
                     ${UNITTEST++_INCLUDE_DIR}
                     ${CVAC_INCLUDE_DIR}
		                 ${ICE_INCLUDE}
		                 ${CMAKE_CURRENT_BINARY_DIR} #for generated slice files
                   )

ADD_EXECUTABLE( ${APP_NAME}
                ${CVAC_APP_SOURCE_FILES}
              )

SET(LIBS
    ${APP_NAME}
    optimized ${UNITTEST++_LIBRARY} debug ${UNITTEST++_LIBRARY_DEBUG}
    utils
    ${ICE_LIBRARIES}
    ${OpenCV_LIBS}
)

#libraries to link into the executable, both Release and Debug
TARGET_LINK_LIBRARIES( ${LIBS} )
 
                      
SET_TARGET_PROPERTIES(${APP_NAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")

ADD_TEST(NAME UnitTests
         COMMAND UnitTests${DEBUG_POSTFIX}
         WORKING_DIRECTORY ${OUTPUT_BINDIR}
         )

IF( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    FILE( TO_NATIVE_PATH ${ICE_ROOT}/bin ICE_BIN_DIR_NATIVE )
    FILE( TO_NATIVE_PATH ${LIBARCHIVE_BIN_DIR} LIBARCHIVE_BIN_DIR_NATIVE )
    FILE( TO_NATIVE_PATH ${OpenCV_DIR}/bin OpenCV_BIN_DIR_NATIVE )
    SET_TESTS_PROPERTIES( UnitTests PROPERTIES ENVIRONMENT "PATH=${LIBARCHIVE_BIN_DIR_NATIVE}\;${ICE_BIN_DIR_NATIVE}\;${OpenCV_BIN_DIR_NATIVE}" )

    #
    # Debug code: to make sure that the env path gets set correctly on Windows,
    # run this pseudo test with "ctest --verbose"
    #
    #ADD_TEST(NAME EnvPathTest COMMAND ${CMAKE_COMMAND} -E environment)
    #SET_TESTS_PROPERTIES( EnvPathTest PROPERTIES ENVIRONMENT "PATH=${LIBARCHIVE_BIN_DIR_NATIVE}\;${ICE_BIN_DIR_NATIVE}\;${OpenCV_BIN_DIR_NATIVE}" )
    ##SET_PROPERTY(TEST EnvPathTest APPEND PROPERTY  ENVIRONMENT "Path=${LIBARCHIVE_BIN_DIR_NATIVE}" )
    ##SET_PROPERTY(TEST EnvPathTest APPEND PROPERTY  ENVIRONMENT "Path=${ICE_BIN_DIR_NATIVE}" )
ENDIF( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
