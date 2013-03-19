PROJECT( WireDiagram Java )

cmake_minimum_required(VERSION 2.8.6)
# Java support since CMake 2.8.6
FIND_PACKAGE( Java )
INCLUDE( UseJava )

WRAP_SLICE2JAVA( SLICE_FILES src cvac
           ${CVAC_ROOT_DIR}/src/Data.ice
           ${CVAC_ROOT_DIR}/src/Services.ice
)

SET(SOURCE_FILES
    ${CVAC_ROOT_DIR}/Algorithms/WireDiagram/src/wirediagram/WireDiagram.java
    ${CVAC_ROOT_DIR}/Algorithms/WireDiagram/src/wirediagram/WireDiagramDetector.java
    ${CVAC_ROOT_DIR}/Algorithms/WireDiagram/src/wirediagram/WireDiagramTrainer.java
    ${CVAC_ROOT_DIR}/src/CVACUtil/RunSetWrapper.java
#    ${SLICE_FILES}
    ${CMAKE_CURRENT_BINARY_DIR}/src/cvac/*.java
)

SET(HEADER_FILES
)

INCLUDE_DIRECTORIES(
)

FIND_JAR( JavaCPP_JAR javacpp PATHS ${CVAC_ROOT_DIR}/lib )
FIND_JAR( JavaCV_JAR javacv PATHS ${CVAC_ROOT_DIR}/lib )
FIND_JAR( Ice.jar Ice PATHS ${CVAC_ROOT_DIR}/lib ${ICE_ROOT}/lib )
SET( CMAKE_JAVA_INCLUDE_PATH ${JavaCPP_JAR} ${JavaCV_JAR} ${Ice.jar} )

ADD_JAR( WireDiagram ${SOURCE_FILES} )

