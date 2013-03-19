PROJECT (SBDICEServerTest)

WRAP_SLICE(SLICE_FILES
           ${CVAC_ROOT_DIR}/src/Data.ice
           ${CVAC_ROOT_DIR}/src/Services.ice
           )

SET(SOURCE_FILES
    SBDICETestI.cpp
    SBDICEServiceTestI.cpp
    readme.txt
)

SET(HEADER_FILES
    SBDICETestI.h
    SBDICEServiceTestI.h
)

SET(INCLUDE_DIRS
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/../SBDCV_source
    ${CVAC_INCLUDE_DIR}
    ${ICE_INCLUDE}
    ${CMAKE_CURRENT_BINARY_DIR}#for the generated slice files
)

INCLUDE_DIRECTORIES(. ${ICE_INCLUDE}
					  ${INCLUDE_DIRS}
					  ${CVAC_INCLUDE_DIR} 
					  ${CMAKE_CURRENT_BINARY_DIR}#for the generated slice files
				   )

ADD_LIBRARY(SBDICEServerTest SHARED
            ${SOURCE_FILES}
            ${HEADER_FILES}
            ${SLICE_FILES}
            )

TARGET_LINK_LIBRARIES( SBDICEServerTest
    ${ICE_LIBRARIES} ${ICE_BOX_LIBRARY}
    ${OpenCV_LIBS}
    SBDCVSource
    utils
    )

