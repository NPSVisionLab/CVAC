PROJECT (opencv_detectcascade_service)
set( the_target OpenCVCascadeDetector )

WRAP_SLICE(SLICE_FILES
           ${CVAC_ROOT_DIR}/src/Data.ice
           ${CVAC_ROOT_DIR}/src/Services.ice
           )

SET(SOURCE_FILES
  CascadeDetectI.cpp
)

SET(HEADER_FILES
  CascadeDetectI.h
)

SET(INCLUDE_DIRS
    .
    ${LIBARCHIVE_INCLUDE}
    ${CVAC_INCLUDE_DIR}
    ${ICE_INCLUDE}
    ${CMAKE_CURRENT_BINARY_DIR}#for the generated slice files
)

INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

ADD_LIBRARY(${the_target} SHARED
            ${SOURCE_FILES}
            ${HEADER_FILES}
            ${SLICE_FILES}
            )

TARGET_LINK_LIBRARIES(${the_target}
    utils
    ${ICE_LIBRARIES} ${ICE_BOX_LIBRARY}
    ${OpenCV_LIBS} ${LIBARCHIVE_LIBRARY}
    )
          
ADD_DEPENDENCIES(${the_target} ${SLICE_FILES})

IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
IF(WIN32)
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
