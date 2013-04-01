PROJECT (sampleICEServer)

WRAP_SLICE(SLICE_FILES
           ${CVAC_ROOT_DIR}/src/Data.ice
           ${CVAC_ROOT_DIR}/src/Services.ice
           )

SET(SOURCE_FILES
    SampleICEI.cpp
)

SET(HEADER_FILES
    SampleICEI.h
)

SET(INCLUDE_DIRS
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CVAC_INCLUDE_DIR}
    ${ICE_INCLUDE}
    ${CMAKE_CURRENT_BINARY_DIR}#for the generated slice files
)

INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

ADD_LIBRARY(sampleICEServer SHARED
            ${SOURCE_FILES}
            ${HEADER_FILES}
            ${SLICE_FILES}
            )

TARGET_LINK_LIBRARIES(sampleICEServer
    sample
    utils
    ${ICE_LIBRARIES} ${ICE_BOX_LIBRARY}
    )
          

