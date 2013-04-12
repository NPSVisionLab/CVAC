SET(APP_NAME UnitTests) 

ADD_SUBDIRECTORY( python )

WRAP_SLICE(SLICE_FILES
           ${CVAC_ROOT_DIR}/src/Data.ice
           ${CVAC_ROOT_DIR}/src/Services.ice
           )

SET (CVAC_APP_SOURCE_FILES 
      main.cpp
      FileUtilsTests.cpp
      Data.cpp
      Services.cpp
      ConfusionMatrixTests.cpp
     )

SET(HEADER_FILES
    Data.h
    Services.h
)

INCLUDE_DIRECTORIES( .
                     ${UNITTEST++_INCLUDE_DIR}
                     ${CVAC_INCLUDE_DIR}
		                 ${ICE_INCLUDE}
		                 ${CMAKE_CURRENT_BINARY_DIR} #for generated slice files
                   )

ADD_EXECUTABLE( ${APP_NAME}
                ${CVAC_APP_SOURCE_FILES}
              )

SET(LIBS
    ${APP_NAME}
    optimized ${UNITTEST++_LIBRARY} debug ${UNITTEST++_LIBRARY_DEBUG}
    utils
    ${ICE_LIBRARIES}
    ${OpenCV_LIBS}
)

#libraries to link into the executable, both Release and Debug
TARGET_LINK_LIBRARIES( ${LIBS} )
 
                      
SET_TARGET_PROPERTIES(${APP_NAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")

ADD_TEST(NAME UnitTests
         COMMAND UnitTests${DEBUG_POSTFIX}
         WORKING_DIRECTORY ${OUTPUT_BINDIR}
         )
