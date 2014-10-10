ENABLE_TESTING()
SET(APP_NAME detectorClient)
PROJECT (${APP_NAME})

WRAP_SLICE(SLICE_FILES
           ${CVAC_ROOT_DIR}/src/Services.ice
           ${CVAC_ROOT_DIR}/src/Data.ice
           )

SET(SOURCE_FILES
    Client.cpp
    readme.txt
)

SET(HEADER_FILES
)

INCLUDE_DIRECTORIES(. ..
                    ${ICE_INCLUDE}
                    ${CMAKE_CURRENT_BINARY_DIR} #for the generated slice files
                    ${CVAC_INCLUDE_DIR}
                    )


ADD_EXECUTABLE(${APP_NAME}
               ${SOURCE_FILES}
               ${HEADER_FILES}
               ${SLICE_FILES}
              )
ADD_DEPENDENCIES(${APP_NAME} ${SLICE_FILES})

TARGET_LINK_LIBRARIES(${APP_NAME}
                      utils
                      ${ICE_LIBRARIES}
                     )
ADD_DEPENDENCIES(${APP_NAME} ${SLICE_FILES})
SET_PROPERTY(SOURCE Client.cpp APPEND PROPERTY OBJECT_DEPENDS ${SLICE_FILES})

SET_TARGET_PROPERTIES(${APP_NAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
# since I can't get CMake RPATH to work, invoke a custom command:
#IF( APPLE )
#  SET_TARGET_PROPERTIES( ${APP_NAME} PROPERTIES INSTALL_RPATH @executable_path/.. )
#  ADD_CUSTOM_COMMAND(TARGET ${APP_NAME}
#    POST_BUILD
#    COMMAND install_name_tool -add_rpath @executable_path/.. ${OUTPUT_BINDIR}/${APP_NAME}
#    )
#ENDIF()

IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
INSTALL( TARGETS ${APP_NAME}
               DESTINATION bin
               COMPONENT Runtime
             )
ENDIF( NOT ${BUILD_3RDPARTY_PACKAGE} )


IF (WIN32)
    SET (SCRIPT_EXT "bat")
ELSE (WIN32)
    SET (SCRIPT_EXT "sh")
ENDIF (WIN32)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/etc/runIceTest.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/runIceTest.${SCRIPT_EXT})

IF( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    FILE( TO_NATIVE_PATH ${ICE_ROOT}/bin ICE_BIN_DIR_NATIVE )
    FILE( TO_NATIVE_PATH ${LIBARCHIVE_BIN_DIR} LIBARCHIVE_BIN_DIR_NATIVE )
    FILE( TO_NATIVE_PATH ${OpenCV_DIR}/bin OpenCV_BIN_DIR_NATIVE )
    SET (ENV_PATH "${LIBARCHIVE_BIN_DIR_NATIVE}\;${ICE_BIN_DIR_NATIVE}\;${OpenCV_BIN_DIR_NATIVE}" )
ELSE( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    SET (ENV_PATH "${ICE_ROOT}/bin:$ENV{PATH}" )
ENDIF( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )

# Ice Box Test Client CTest Suite:
# To get debug output go to visualStudio directory and run ctest --verbose
# All IceBoxClient tests return 0 on success ('EXIT_SUCCESS'), and 1 on failure ('EXIT_FAILURE')
# CTest treats any non-zero return as FAILURE
#MESSAGE("/CVAC_GENERATED_OUTPUT_DIR/Testing/Temporary/LastTest.log")    #Display path to test-log file

IF (BUILD_WITH_BOW) 
    # see if we can talk to vision.nps.edu
    ADD_TEST(detectorClient_BowDetector_NPS
             ${OUTPUT_BINDIR}/${APP_NAME} NpsBagOfWordsUSKOCA testImg ${CVAC_ROOT_DIR}/config.client
             )
    # note that a missing PATH to bzip2 in 3rdparty/ICE/bin might cause problems
    SET_TESTS_PROPERTIES( detectorClient_BowDetector_NPS
          PROPERTIES ENVIRONMENT "PATH=${ENV_PATH}" )

    # local service; each of these starts an IceBox in the script
    ADD_TEST(detectorClient_BowDetector
             ${CVAC_ROOT_DIR}/bin/detectorClient BagOfWordsUSKOCA testImg ${CVAC_ROOT_DIR}/config.client
             )
    SET_TESTS_PROPERTIES( detectorClient_BowDetector
          PROPERTIES ENVIRONMENT "PATH=${ENV_PATH}" )
    ADD_TEST(detectorClient_BowDetector_Verify
             ${CVAC_ROOT_DIR}/bin/detectorClient BagOfWordsUSKOCA CTest ${CVAC_ROOT_DIR}/config.client verifyresults
             )
    SET_TESTS_PROPERTIES( detectorClient_BowDetector_Verify
          PROPERTIES ENVIRONMENT "PATH=${ENV_PATH}" )
    IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
        INSTALL( FILES ${CVAC_ROOT_DIR}/bin/runIceTest.${SCRIPT_EXT}
                   DESTINATION bin
                   COMPONENT Runtime
                 )
    ENDIF( NOT ${BUILD_3RDPARTY_PACKAGE} )
ENDIF (BUILD_WITH_BOW)

## Positive-Image Live-Verification for Detectors ###
#ADD_TEST(Ice_Box_Test_Client_cvPerformanceDetector_Verify
#         ${OUTPUT_BINDIR}/${APP_NAME}${CMAKE_DEBUG_POSTFIX} CvFaces CTest ${CVAC_ROOT_DIR}/config.client verifyresults
#         )
         
