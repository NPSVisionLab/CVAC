#
# 

cmake_minimum_required(VERSION 2.6)

WRAP_SLICE2PY( SLICE_FILES
    ${CMAKE_SOURCE_DIR}/src/Files.ice
    ${CMAKE_SOURCE_DIR}/src/Data.ice
    ${CMAKE_SOURCE_DIR}/src/Services.ice
    ${CMAKE_SOURCE_DIR}/src/Corpus.ice
)

#
# Copy paths.py to the python lib dir expanding variables
#
CONFIGURE_FILE(paths.py.in "${SLICE_OUTPUT_PYTHONDIR}/paths.py")

#
# Copy easy.py to the python lib dir without expanding
#
CONFIGURE_FILE(easy.py "${SLICE_OUTPUT_PYTHONDIR}/easy.py" COPYONLY)

add_custom_target( easy ALL DEPENDS ${SLICE_FILES} )
