PROJECT (opencv_traincascade_service)

set(the_target OpenCVCascadeTrainer)

set(BUILD_opencv_apps 1)

WRAP_SLICE(SLICE_FILES
           ${CVAC_ROOT_DIR}/src/Data.ice
           ${CVAC_ROOT_DIR}/src/Services.ice
           )

SET(SOURCE_FILES
    CascadeTrainI.cpp
)

SET(HEADER_FILES
    CascadeTrainI.h
)

SET(INCLUDE_DIRS
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/../traincascade3.0
    ${CMAKE_CURRENT_SOURCE_DIR}/../createSamples
    ${CVAC_INCLUDE_DIR}
    ${ICE_INCLUDE}
    ${CMAKE_CURRENT_BINARY_DIR}#for the generated slice files
    ${OpenCV_INCLUDE_DIRS}
)

INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

ADD_LIBRARY(${the_target} SHARED
            ${SOURCE_FILES}
            ${HEADER_FILES}
            ${SLICE_FILES}
            )

TARGET_LINK_LIBRARIES(${the_target}
    traincascade
    createsamples
    ${ICE_LIBRARIES} ${ICE_BOX_LIBRARY}
    ${OpenCV_LIBS}
    utils
    )

ADD_DEPENDENCIES(${the_target} ${SLICE_FILES})

IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
IF (WIN32)
INSTALL( TARGETS ${the_target}
               DESTINATION bin
               COMPONENT Runtime
             )
ELSE (WIN32)
INSTALL( TARGETS ${the_target}
               DESTINATION lib
               COMPONENT Runtime
             )
ENDIF (WIN32)
ENDIF( NOT ${BUILD_3RDPARTY_PACKAGE} )
