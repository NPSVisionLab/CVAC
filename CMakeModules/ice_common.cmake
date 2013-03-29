#Defines the WRAP_SLICE macro
#  usage:
#    WRAP_SLICE(SLICE_FILES myIceFile.ice)
#    ADD_LIBRARY (myLib 
#                 ...
#                 ${SLICE_FILES})

MACRO (EXTRACT_OPTIONS _files _options)
  SET(${_files})
  SET(${_options})
  SET(_QT4_DOING_OPTIONS FALSE)
  FOREACH(_currentArg ${ARGN})
    IF ("${_currentArg}" STREQUAL "OPTIONS")
      SET(_QT4_DOING_OPTIONS TRUE)
    ELSE ("${_currentArg}" STREQUAL "OPTIONS")
      IF(_QT4_DOING_OPTIONS) 
        LIST(APPEND ${_options} "${_currentArg}")
      ELSE(_QT4_DOING_OPTIONS)
        LIST(APPEND ${_files} "${_currentArg}")
      ENDIF(_QT4_DOING_OPTIONS)
    ENDIF ("${_currentArg}" STREQUAL "OPTIONS")
  ENDFOREACH(_currentArg) 
ENDMACRO (EXTRACT_OPTIONS)

MACRO(WRAP_SLICE outfiles)
    EXTRACT_OPTIONS(slice_files options ${ARGN})
    
    FOREACH(it ${slice_files})
        GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)
        GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
        SET(_header ${CMAKE_CURRENT_BINARY_DIR}/${outfile}.h)
        SET(_src ${CMAKE_CURRENT_BINARY_DIR}/${outfile}.cpp)
        ADD_CUSTOM_COMMAND(OUTPUT ${_header} ${_src}
          COMMAND ${ICE_SLICE_EXECUTABLE}
          ARGS --output-dir ${CMAKE_CURRENT_BINARY_DIR} -I${ICE_ROOT}/slice ${infile}
          MAIN_DEPENDENCY ${infile})

        SET(${outfiles} ${${outfiles}} ${_header} ${_src})

    ENDFOREACH(it)
ENDMACRO(WRAP_SLICE)

# note: Other than slice2cpp, slice2java automatically places generated files into
# the respective namespace directory. So the output dir is just 'java' not 'java/<namespace>'
MACRO(WRAP_SLICE2JAVA outfiles cpout package)
    EXTRACT_OPTIONS(slice_files options ${ARGN})
    
    FOREACH(it ${slice_files})
        GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)
        GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
        SET(_java ${CMAKE_CURRENT_BINARY_DIR}/${cpout}/${package}/${outfile}.java)
        ADD_CUSTOM_COMMAND(OUTPUT ${_java}
          COMMAND ${ICE_SLICE2JAVA_EXECUTABLE}
          ARGS --output-dir ${CMAKE_CURRENT_BINARY_DIR}/${cpout} -I${ICE_ROOT}/slice ${infile}
          MAIN_DEPENDENCY ${infile})

        SET(${outfiles} ${${outfiles}} ${_java})

    ENDFOREACH(it)
#    FILE( GLOB_RECURSE suite ${cpout}/${package}/*.java )
#    SET( ${${outfiles}} ${suite} )
#    FOREACH(item ${suite})
#      MESSAGE ("Globbed item: ${item}")
#    ENDFOREACH(${item})
ENDMACRO(WRAP_SLICE2JAVA)
