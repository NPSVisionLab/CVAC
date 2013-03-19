PROJECT (VideoValidatorForOpenCVServer)

WRAP_SLICE(SLICE_FILES
           ${CVAC_ROOT_DIR}/src/Data.ice
           ${CVAC_ROOT_DIR}/src/Services.ice
           )

SET(SOURCE_FILES
    VideoValidatorForOpenCVI.cpp
	VideoValidatorForOpenCVServiceI.cpp
	readme.txt
)

SET(HEADER_FILES
    VideoValidatorForOpenCVI.h
    VideoValidatorForOpenCVServiceI.h
)

SET(INCLUDE_DIRS
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/../VideoValidatorForOpenCV_source
    ${CVAC_INCLUDE_DIR}
    ${ICE_INCLUDE}
    ${CMAKE_CURRENT_BINARY_DIR}#for the generated slice files
)

INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

ADD_LIBRARY(VideoValidatorForOpenCVServer SHARED
            ${SOURCE_FILES}
            ${HEADER_FILES}
            ${SLICE_FILES}
            )

TARGET_LINK_LIBRARIES(
    VideoValidatorForOpenCVServer
    VideoValidatorForOpenCVSource
    ${ICE_LIBRARIES} ${ICE_BOX_LIBRARY}
    ${OpenCV_LIBS}
    utils
)

