project(traincascade)

set(traincascade_files
  cascadeclassifier.cpp cascadeclassifier.h
  boost.cpp boost.h features.cpp traincascade_features.h
  haarfeatures.cpp haarfeatures.h
  lbpfeatures.cpp lbpfeatures.h
  HOGfeatures.cpp HOGfeatures.h
  imagestorage.cpp imagestorage.h)


SET(INCLUDE_DIRS
    .
    ${OpenCV_INCLUDE_DIRS}
)

set(the_target opencv_traincascade)    
add_library(${the_target} ${traincascade_files})
target_link_libraries(${the_target} ${OPENCV_TRAINCASCADE_DEPS} opencv_haartraining_engine)

set_target_properties(${the_target} PROPERTIES
                      DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
                      ARCHIVE_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH}
                      RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
                      INSTALL_NAME_DIR lib
                      OUTPUT_NAME "opencv_traincascade")

IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
  install(TARGETS ${the_target} ARCHIVE DESTINATION lib COMPONENT algorithms)
ENDIF()
