PROJECT (VideoValidatorForOpenCVSource)

SET(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/VideoValidatorForOpenCV.cpp
)

SET(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/VideoValidatorForOpenCV.h
)

SET(INCLUDE_DIRS
    .
    ${OpenCV_INCLUDE_DIRS}
)

INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

ADD_LIBRARY(VideoValidatorForOpenCVSource SHARED
            ${SOURCE_FILES}
            ${HEADER_FILES}
            )

TARGET_LINK_LIBRARIES(VideoValidatorForOpenCVSource
    ${OpenCV_LIBS}
)


