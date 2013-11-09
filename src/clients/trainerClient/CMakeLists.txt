SET(APP_NAME trainerClient)
PROJECT (${APP_NAME})

WRAP_SLICE(SLICE_FILES
           ${CVAC_ROOT_DIR}/src/Services.ice
           ${CVAC_ROOT_DIR}/src/Data.ice
           )

SET(SOURCE_FILES
    trainerClient.cpp
    config.client
    readme.txt
)

SET(HEADER_FILES
)

INCLUDE_DIRECTORIES(. ..
                    ${ICE_INCLUDE}
                    ${CVAC_INCLUDE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR} #for the generated slice files
                    )


ADD_EXECUTABLE(${APP_NAME}
               ${SOURCE_FILES}
               ${HEADER_FILES}
               ${SLICE_FILES}
              )

SET(LIBS
    ${APP_NAME}
    utils
    ${ICE_LIBRARIES}
)                       

#libraries to link into the executable, both Release and Debug
TARGET_LINK_LIBRARIES( ${LIBS} )


SET_TARGET_PROPERTIES(${APP_NAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")

IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
INSTALL( TARGETS ${APP_NAME}
               DESTINATION bin
               COMPONENT Runtime
             )
ENDIF( NOT ${BUILD_3RDPARTY_PACKAGE} )
