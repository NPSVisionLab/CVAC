#
# CMake script that runs the "python setup.py build" and install
# methods
# 

cmake_minimum_required(VERSION 2.6)

find_program(PYTHON "python")

if (PYTHON)
    set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in")
    set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")
#    set(DEPS        "${CMAKE_CURRENT_SOURCE_DIR}/module/__init__.py")
    set(OUTPUT      "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

#    configure_file(${SETUP_PY_IN} ${SETUP_PY})

#    SLICE2PY
#  /opt/Ice-3.4.2/bin/slice2py -I/opt/Ice-3.4.2/slice ../src/Data.ice ../src/Services.ice

WRAP_SLICE2PY( SLICE_FILES
    ${CMAKE_SOURCE_DIR}/src/Data.ice
    ${CMAKE_SOURCE_DIR}/src/Services.ice
    ${CMAKE_SOURCE_DIR}/src/Corpus.ice
)

# need to single out one class so we can address it explicitly instead of with *
#SET ( SLICE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
#MAKE_DIRECTORY ( ${SLICE_OUTPUT_PATH} )

#
# Command to run slice2py to produce all *_ice.py files
#
#SET ( SLICE_ARGS ${SLICE_FILES} -I${ICE_ROOT}/slice --output-dir ${SLICE_OUTPUT_PATH} )
#ADD_CUSTOM_COMMAND(
#    OUTPUT ${SLICE_OUTPUT_PATH}/cvac/${SLICE_TOKEN_CLASS}.java
#    COMMAND ${ICE_SLICE2JAVA_EXECUTABLE}
#    ARGS ${SLICE_ARGS}
#    DEPENDS ${SLICE_FILES}
#    COMMENT "Generating .java files from .ice sources." )
#MESSAGE ( STATUS "Java source files will be created from Slice definitions using this command:" )
#MESSAGE ( STATUS "${ICE_SLICE2JAVA_EXECUTABLE} ${SLICE_ARGS}" )


#    add_custom_command(OUTPUT ${OUTPUT}
#                       COMMAND ${PYTHON} ${SETUP_PY} build
#                       COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT}
#                       DEPENDS ${DEPS})

    add_custom_target(PythonUnitTests ALL DEPENDS ${SLICE_FILES})

    # Test whether a Python client can connect and talk to the Corpus service
    # Note: this currently needs the Java Corpus server started manually
    # Note: quite likely we'll need this on Windows, and ; instead of :
    #       FILE( TO_NATIVE_PATH ${ICE_ROOT}/python ICE_PYTHON_DIR_NATIVE )
    ADD_TEST( PythonCorpusTest
       ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/CorpusServerTest.py )
    SET_TESTS_PROPERTIES( PythonCorpusTest
      PROPERTIES ENVIRONMENT "PYTHONPATH=${ICE_ROOT}/python:${CMAKE_CURRENT_BINARY_DIR}" )

    #    install(CODE "execute_process(COMMAND ${PYTHON} ${SETUP_PY} install)")
endif()
