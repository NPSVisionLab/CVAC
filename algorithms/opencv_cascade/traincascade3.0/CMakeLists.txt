project(traincascade)

#Fix for old_ml_precomp.hpp including opencv's private.hpp
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__OPENCV_BUILD")

set(traincascade_include_files
  cascadeclassifier.h
  boost.h traincascade_features.h
  haarfeatures.h
  lbpfeatures.h
  HOGfeatures.h
  imagestorage.h
  old_ml.hpp 
  old_ml_precomp.hpp)

set(traincascade_files
  cascadeclassifier.cpp 
  boost.cpp features.cpp
  haarfeatures.cpp
  lbpfeatures.cpp
  HOGfeatures.cpp
  imagestorage.cpp
  traincascade.cpp
  old_ml_boost.cpp old_ml_data.cpp
  old_ml_inner_functions.cpp 
  old_ml_tree.cpp)

#OpenCV_DIR/.. for cvconfig.h
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS} ${OpenCV_DIR} ${OpenCV_DIR}/ippicv_win/include)

set(the_target traincascade)
add_library(${the_target} ${traincascade_files} ${traincascade_include_files})


IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
  install(TARGETS ${the_target} ARCHIVE DESTINATION lib COMPONENT algorithms)
ENDIF()
