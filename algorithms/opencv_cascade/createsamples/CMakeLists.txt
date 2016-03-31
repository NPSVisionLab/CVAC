project(createsamples)

set(HEADER_FILES
  _cvcommon.h
  _cvhaartraining.h
  cvhaartraining.h
  cvclassifier.h
  utility.hpp)

set(SOURCE_FILES
  mergevec.cpp
  utility.cpp
  )

INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})

ADD_LIBRARY(createsamples
            ${SOURCE_FILES}
            ${HEADER_FILES}
           )

IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
  INSTALL( TARGETS createsamples
           DESTINATION lib
           COMPONENT runtime
         )
ENDIF( NOT ${BUILD_3RDPARTY_PACKAGE} )
