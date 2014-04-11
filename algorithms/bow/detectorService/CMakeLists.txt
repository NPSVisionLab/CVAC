PROJECT (bowICEServer)

WRAP_SLICE(SLICE_FILES
           ${CVAC_ROOT_DIR}/src/Data.ice
           ${CVAC_ROOT_DIR}/src/Services.ice
           )

SET(SOURCE_FILES
    BowICEI.cpp
)

SET(HEADER_FILES
    BowICEI.h
)

SET(INCLUDE_DIRS
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
    ${CVAC_INCLUDE_DIR}
    ${ICE_INCLUDE}
    ${CMAKE_CURRENT_BINARY_DIR}#for the generated slice files
)

INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

ADD_LIBRARY(bowICEServer SHARED
            ${SOURCE_FILES}
            ${HEADER_FILES}
            ${SLICE_FILES}
            )

TARGET_LINK_LIBRARIES(bowICEServer
    bow
    utils
    ${ICE_LIBRARIES} ${ICE_BOX_LIBRARY}
    ${OpenCV_LIBS}
    )

ADD_DEPENDENCIES(bowICEServer ${SLICE_FILES})
SET_PROPERTY(SOURCE BowICEI.cpp APPEND PROPERTY OBJECT_DEPENDS ${SLICE_FILES})
          
IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
IF(WIN32)
INSTALL( TARGETS bowICEServer
               DESTINATION bin
               COMPONENT Runtime
             )
ELSE (WIN32)
INSTALL( TARGETS bowICEServer
               DESTINATION lib
               COMPONENT Runtime
             )

ENDIF (WIN32)
ENDIF( NOT ${BUILD_3RDPARTY_PACKAGE} )
