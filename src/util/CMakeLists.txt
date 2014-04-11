PROJECT (utilLib)


WRAP_SLICE(SLICE_FILES
           ${CMAKE_SOURCE_DIR}/src/Data.ice
           ${CMAKE_SOURCE_DIR}/src/Services.ice
           )

SET(SOURCE_FILES
    ConfusionMatrix.cpp
    FileUtils.cpp
    processRunSet.cpp
    processLabels.cpp
    DetectorDataArchive.cpp
    VLogger.cpp
    ServiceMan.cpp
    ServiceInvocation.cpp
    RunSetIterator.cpp
    RunSetWrapper.cpp
	MediaConverter.cpp
    OutputResults.cpp
)

SET(HEADER_FILES
    ${CVAC_INCLUDE_DIR}/util/ConfusionMatrix.h
    ${CVAC_INCLUDE_DIR}/util/processRunSet.h
    ${CVAC_INCLUDE_DIR}/util/processLabels.h
    ${CVAC_INCLUDE_DIR}/util/DetectorDataArchive.h
    ${CVAC_INCLUDE_DIR}/util/FileUtils.h
    ${CVAC_INCLUDE_DIR}/util/Timing.h
    ${CVAC_INCLUDE_DIR}/util/VLogger.h
    ${CVAC_INCLUDE_DIR}/util/ServiceMan.h
    ${CVAC_INCLUDE_DIR}/util/ServiceManI.h
    ${CVAC_INCLUDE_DIR}/util/ServiceInvocation.h
	${CVAC_INCLUDE_DIR}/util/RunSetIterator.h
	${CVAC_INCLUDE_DIR}/util/RunSetWrapper.h
	${CVAC_INCLUDE_DIR}/util/MediaConverter.h
	${CVAC_INCLUDE_DIR}/util/OutputResults.h
	${CVAC_INCLUDE_DIR}/util/MsgLogger.h
#if (WIN32)
    ${CVAC_INCLUDE_DIR}/util/wdirent.h
#endif (WIN32)
)

INCLUDE_DIRECTORIES(${CVAC_INCLUDE_DIR} ${ICE_INCLUDE}
                    ${OpenCV_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_BINARY_DIR}#for the generated slice files
                    ${LIBARCHIVE_INCLUDE}
                    )


ADD_LIBRARY(utils
            ${SOURCE_FILES}
            ${HEADER_FILES}
            ${SLICE_FILES}
            )

TARGET_LINK_LIBRARIES(utils
                      ${LIBARCHIVE_LIBRARY}
                     )
ADD_DEPENDENCIES(utils ${SLICE_FILES})
SET_PROPERTY(SOURCE processLabels.cpp APPEND PROPERTY OBJECT_DEPENDS ${SLICE_FILES})
SET_PROPERTY(SOURCE processRunSet.cpp APPEND PROPERTY OBJECT_DEPENDS ${SLICE_FILES})

IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
  INSTALL( TARGETS utils
                DESTINATION lib
               COMPONENT Runtime )
ENDIF( NOT ${BUILD_3RDPARTY_PACKAGE} )
