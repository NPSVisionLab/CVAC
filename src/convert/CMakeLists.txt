PROJECT (conversion)


WRAP_SLICE(SLICE_FILES
           ${CMAKE_SOURCE_DIR}/src/Data.ice
           ${CMAKE_SOURCE_DIR}/src/Services.ice
           )

SET(SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/src/convert/ConvertVatic.cpp
    )

SET(HEADER_FILES
    ${CVAC_INCLUDE_DIR}/convert/ConvertVatic.h
    )

INCLUDE_DIRECTORIES(${CVAC_INCLUDE_DIR} ${ICE_INCLUDE}
                    ${CMAKE_CURRENT_BINARY_DIR}#for the generated slice files
                    )


ADD_LIBRARY(convertVaticLib
            ${SOURCE_FILES}
            ${HEADER_FILES}
            ${SLICE_FILES}
            )