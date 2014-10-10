project(haartraining)


# -----------------------------------------------------------
#  Library
# -----------------------------------------------------------
set(cvhaartraining_lib_src
  _cvcommon.h
  cvclassifier.h
  _cvhaartraining.h
  cvhaartraining.h
  cvboost.cpp
  cvcommon.cpp
  cvhaarclassifier.cpp
  cvhaartraining.cpp
  cvsamples.cpp
  mergevec.cpp
  )

add_library(opencv_haartraining_engine STATIC ${cvhaartraining_lib_src})
set_target_properties(opencv_haartraining_engine PROPERTIES
                      DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
                      ARCHIVE_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH}
                      RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
                      INSTALL_NAME_DIR lib
                     )

if(WIN32)
  link_directories(${CMAKE_CURRENT_BINARY_DIR})
endif()

target_link_libraries(opencv_haartraining_engine ${OPENCV_HAARTRAINING_DEPS})

# -----------------------------------------------------------
#  haartraining
# -----------------------------------------------------------

#add_executable(opencv_haartraining cvhaartraining.h haartraining.cpp)
#set_target_properties(opencv_haartraining PROPERTIES
#                      DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
#                      OUTPUT_NAME "opencv_haartraining")

# -----------------------------------------------------------
#  createsamples
# -----------------------------------------------------------

#add_executable(opencv_createsamples cvhaartraining.h createsamples.cpp)
#set_target_properties(opencv_createsamples PROPERTIES
#                      DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
#                      OUTPUT_NAME "opencv_createsamples")

# -----------------------------------------------------------
#  performance
# -----------------------------------------------------------
#add_executable(opencv_performance performance.cpp)
#set_target_properties(opencv_performance PROPERTIES
#                      DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
#                      OUTPUT_NAME "opencv_performance")

# -----------------------------------------------------------
#  Install part
# -----------------------------------------------------------

#install(TARGETS opencv_haartraining RUNTIME DESTINATION bin COMPONENT main)
#install(TARGETS opencv_createsamples RUNTIME DESTINATION bin COMPONENT main)
#install(TARGETS opencv_performance RUNTIME DESTINATION bin COMPONENT main)

