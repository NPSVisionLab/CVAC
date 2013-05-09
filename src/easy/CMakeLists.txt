#
# 

cmake_minimum_required(VERSION 2.6)

WRAP_SLICE2PY( SLICE_FILES
    ${CMAKE_SOURCE_DIR}/src/Data.ice
    ${CMAKE_SOURCE_DIR}/src/Services.ice
    ${CMAKE_SOURCE_DIR}/src/Corpus.ice
)

add_custom_target( easy ALL DEPENDS ${SLICE_FILES} )
