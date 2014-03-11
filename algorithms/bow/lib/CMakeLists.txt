PROJECT (bow)

WRAP_SLICE(SLICE_FILES
           ${CVAC_ROOT_DIR}/src/Data.ice
           ${CVAC_ROOT_DIR}/src/Services.ice
           )

SET(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/bowCV.cpp
)

SET(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/bowCV.h
)

SET(INCLUDE_DIRS
    .
    ${CVAC_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
	${ICE_INCLUDE}
    ${CMAKE_CURRENT_BINARY_DIR}#for the generated slice files
)

INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

ADD_LIBRARY(bow STATIC
            ${SOURCE_FILES}
            ${HEADER_FILES}
			${SLICE_FILES}
            )

