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

TARGET_LINK_LIBRARIES(${APP_NAME}
                      utils
                      ${ICE_LIBRARIES}
                     )

SET_TARGET_PROPERTIES(${APP_NAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")

INSTALL( TARGETS ${APP_NAME}
               DESTINATION bin
             )


IF (WIN32)
    SET (SCRIPT_EXT "bat")
ELSE (WIN32)
    SET (SCRIPT_EXT "sh")
ENDIF (WIN32)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/etc/runIceTest.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/runIceTest.${SCRIPT_EXT})

# Ice Box Test Client CTest Suite:
# To get debug output go to visualStudio directory and run ctest --verbose
# All IceBoxClient tests return 0 on success ('EXIT_SUCCESS'), and 1 on failure ('EXIT_FAILURE')
# CTest treats any non-zero return as FAILURE
#MESSAGE("/CVAC_GENERATED_OUTPUT_DIR/Testing/Temporary/LastTest.log")    #Display path to test-log file

IF (BUILD_WITH_BOW) 
ADD_TEST(detectorClient_BowDetector
         ${CVAC_ROOT_DIR}/bin/runIceTest.${SCRIPT_EXT} BagOfWordsUSKOCA
         )
ADD_TEST(detectorClient_BowDetector_Verify
         ${CVAC_ROOT_DIR}/bin/runIceTest.${SCRIPT_EXT} CTest BagOfWordsUSKOCA
         )
INSTALL( FILES ${CVAC_ROOT_DIR}/bin/runIceTest.${SCRIPT_EXT}
               DESTINATION bin
             )
ENDIF (BUILD_WITH_BOW)

## Positive-Image Live-Verification for Detectors ###
#ADD_TEST(Ice_Box_Test_Client_cvPerformanceDetector_Verify
#         ${OUTPUT_BINDIR}/${APP_NAME}${CMAKE_DEBUG_POSTFIX} CvFaces CTest ${CVAC_ROOT_DIR}/config.client verifyresults
#         )
         
