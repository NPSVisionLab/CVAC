#
# CMake script for unit tests that are written in Python
# 

cmake_minimum_required(VERSION 2.6)

find_program(PYTHON_EXECUTABLE "python")

IF( NOT PYTHON_EXECUTABLE )
  MESSAGE( FATAL_ERROR "Python executable not found: cannot run Python UnitTests" )
ENDIF( NOT PYTHON_EXECUTABLE )

set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in")
set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")
set(OUTPUT      "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")


add_custom_target(PythonUnitTests ALL DEPENDS ${SLICE_FILES})
IF (WIN32)
    FILE( TO_NATIVE_PATH ${ICE_PYTHON_DIR} ICE_PYTHON_NATIVE )
    FILE( TO_NATIVE_PATH ${SLICE_OUTPUT_PYTHONDIR} SLICE_OUTPUT_PYTHONDIR_NATIVE )
    FILE( TO_NATIVE_PATH ${ICE_ROOT}/bin ICE_BIN_NATIVE )
    SET (MY_PYTHONPATH "${ICE_PYTHON_NATIVE}\;${SLICE_OUTPUT_PYTHONDIR_NATIVE}" )
    SET (MY_PATH "${ICE_BIN_NATIVE}\;${ICE_PYTHON_NATIVE}\;${PATH}" )

ELSE (WIN32)
    SET (MY_PYTHONPATH "${ICE_PYTHON_DIR}:${SLICE_OUTPUT_PYTHONDIR}" )
    SET (MY_PATH "${PATH}" )
ENDIF (WIN32)

# Test whether a Python client can connect and talk to the Corpus service
# Note: this currently needs the Java Corpus server started manually
# Note: quite likely we'll need this on Windows, and ; instead of :
#       FILE( TO_NATIVE_PATH ${ICE_ROOT}/python ICE_PYTHON_DIR_NATIVE )
ADD_TEST( PythonCorpusTest
  ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/CorpusServerTest.py )
SET_TESTS_PROPERTIES( PythonCorpusTest
  PROPERTIES ENVIRONMENT "PYTHONPATH=${MY_PYTHONPATH};PATH=${MY_PATH}")

ADD_TEST( PythonFileServerTest
  ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/FileServerTest.py )
SET_TESTS_PROPERTIES( PythonFileServerTest
  PROPERTIES ENVIRONMENT "PYTHONPATH=${MY_PYTHONPATH};PATH=${MY_PATH}")

ADD_TEST( MiniTutorialTest
  ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/MiniTutorialTest.py )
SET_TESTS_PROPERTIES( MiniTutorialTest
  PROPERTIES ENVIRONMENT "PYTHONPATH=${MY_PYTHONPATH};PATH=${MY_PATH}")

IF( BUILD_WITH_RSI )
  ADD_TEST( RunsetIteratorTest
    ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/RunsetIteratorTest.py )
  SET_TESTS_PROPERTIES( RunsetIteratorTest
    PROPERTIES ENVIRONMENT "PYTHONPATH=${MY_PYTHONPATH};PATH=${MY_PATH}")
ENDIF()

INCLUDE (UserCMakeLists.txt OPTIONAL)
