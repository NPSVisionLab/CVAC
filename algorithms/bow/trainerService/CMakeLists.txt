PROJECT (bowTrainerService)

WRAP_SLICE(SLICE_FILES
           ${CVAC_ROOT_DIR}/src/Data.ice
           ${CVAC_ROOT_DIR}/src/Services.ice
           )

SET(SOURCE_FILES
    BowICETrainI.cpp
)

SET(HEADER_FILES
    BowICETrainI.h
)

SET(INCLUDE_DIRS
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
    ${CVAC_INCLUDE_DIR}
    ${ICE_INCLUDE}
    ${CMAKE_CURRENT_BINARY_DIR}#for the generated slice files
)

INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

ADD_LIBRARY(bowICEServerTrain SHARED
            ${SOURCE_FILES}
            ${HEADER_FILES}
            ${SLICE_FILES}
            )

TARGET_LINK_LIBRARIES(bowICEServerTrain
    bow
    ${ICE_LIBRARIES} ${ICE_BOX_LIBRARY}
    ${OpenCV_LIBS}
    utils
    )

IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
IF (WIN32)
INSTALL( TARGETS bowICEServerTrain
               DESTINATION bin
               COMPONENT Runtime
             )
ELSE (WIN32)
INSTALL( TARGETS bowICEServerTrain
               DESTINATION lib
               COMPONENT Runtime
             )
ENDIF (WIN32)
ENDIF( NOT ${BUILD_3RDPARTY_PACKAGE} )
