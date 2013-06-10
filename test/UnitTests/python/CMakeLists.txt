#
# CMake script that runs the "python setup.py build" and install
# methods
# 

cmake_minimum_required(VERSION 2.6)

find_program(PYTHON_EXECUTABLE "python")

IF( NOT PYTHON_EXECUTABLE )
  MESSAGE( FATAL_ERROR "Python executable not found: cannot run Python UnitTests" )
ENDIF( NOT PYTHON_EXECUTABLE )

set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in")
set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")
#    set(DEPS        "${CMAKE_CURRENT_SOURCE_DIR}/module/__init__.py")
set(OUTPUT      "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

#    configure_file(${SETUP_PY_IN} ${SETUP_PY})
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/src/easy/paths.py.in ${CMAKE_CURRENT_SOURCE_DIR}/paths.py)

#    SLICE2PY
#  /opt/Ice-3.4.2/bin/slice2py -I/opt/Ice-3.4.2/slice ../src/Data.ice ../src/Services.ice

# Not needed since src/easy does this
#WRAP_SLICE2PY( SLICE_FILES
#    ${CMAKE_SOURCE_DIR}/src/Files.ice
#    ${CMAKE_SOURCE_DIR}/src/Data.ice
#    ${CMAKE_SOURCE_DIR}/src/Services.ice
#    ${CMAKE_SOURCE_DIR}/src/Corpus.ice
#)

#    Python build process
#    add_custom_command(OUTPUT ${OUTPUT}
#                       COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY} build
#                       COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT}
#                       DEPENDS ${DEPS})

add_custom_target(PythonUnitTests ALL DEPENDS ${SLICE_FILES})

# Test whether a Python client can connect and talk to the Corpus service
# Note: this currently needs the Java Corpus server started manually
# Note: quite likely we'll need this on Windows, and ; instead of :
#       FILE( TO_NATIVE_PATH ${ICE_ROOT}/python ICE_PYTHON_DIR_NATIVE )
ADD_TEST( PythonCorpusTest
  ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/CorpusServerTest.py )
SET_TESTS_PROPERTIES( PythonCorpusTest
  PROPERTIES ENVIRONMENT "PYTHONPATH=${ICE_PYTHON_DIR}:${SLICE_OUTPUT_PYTHONDIR}" )

ADD_TEST( PythonFileServerTest
  ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/FileServerTest.py )
SET_TESTS_PROPERTIES( PythonFileServerTest
  PROPERTIES ENVIRONMENT "PYTHONPATH=${ICE_PYTHON_DIR}:${SLICE_OUTPUT_PYTHONDIR}" )

#    install(CODE "execute_process(COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY} install)")
