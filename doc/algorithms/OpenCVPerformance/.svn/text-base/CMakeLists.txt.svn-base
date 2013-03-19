PROJECT (OpenCVPerformance)

#FIND_PACKAGE(OpenCV REQUIRED core highgui objdetect)

WRAP_SLICE(SLICE_FILES
           ${CVAC_ROOT_DIR}/src/Data.ice
           ${CVAC_ROOT_DIR}/src/Services.ice
           )
IF (!${CMAKE_CXX_COMPILER}==g++)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast /Oi")
ENDIF (!${CMAKE_CXX_COMPILER}==g++)

SET(SOURCE_FILES
  performance.cpp
  haartrainingCVAC.cpp
  cvPeformanceTrainerCVAC.cpp
  CvPerfICEServiceTrainI.cpp
  CvPerfICETrainI.cpp
  cvhaartraining.cpp
  cvhaarclassifier.cpp
  cvboost.cpp
  cvcommon.cpp
  cvsamples.cpp
)

SET(HEADER_FILES
  DetectorI.h
  cvhaartrainingCVAC.h
  cvclassifierCVAC.h
  cvcommonCVAC.h
  cvPerformanceTrainerCVAC.h
  CvPerfICEServiceTrainI.h
  CvPerfICETrainI.h
  file_open_options.h
  cvclassifier.h
  cvhaartraining.h
  _cvcommon.h
  _cvhaartraining.h
)

INCLUDE_DIRECTORIES(. ${ICE_INCLUDE}
                    ${CVAC_INCLUDE_DIR}
                    ${OpenCV_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_BINARY_DIR}#for the generated slice files
                    )


ADD_LIBRARY(OpenCVPerformance SHARED
            ${SOURCE_FILES}
            ${HEADER_FILES}
            ${SLICE_FILES}
            )

TARGET_LINK_LIBRARIES(OpenCVPerformance
                      utils
                      ${ICE_LIBRARIES} ${ICE_BOX_LIBRARY}
                      ${OpenCV_LIBS}
                     )

