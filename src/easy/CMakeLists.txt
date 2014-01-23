#
# 

cmake_minimum_required(VERSION 2.6)

WRAP_SLICE2PY( SLICE_FILES
    ${CMAKE_SOURCE_DIR}/src/Files.ice
    ${CMAKE_SOURCE_DIR}/src/Data.ice
    ${CMAKE_SOURCE_DIR}/src/Services.ice
    ${CMAKE_SOURCE_DIR}/src/Corpus.ice
)

MAKE_DIRECTORY(${SLICE_OUTPUT_PYTHONDIR})

#
# Copy easy.py to the python lib dir without expanding
#
CONFIGURE_FILE(easy.py "${SLICE_OUTPUT_PYTHONDIR}/easy/easy.py" COPYONLY)
CONFIGURE_FILE(evaluate.py "${SLICE_OUTPUT_PYTHONDIR}/easy/evaluate.py" COPYONLY)
CONFIGURE_FILE(__init__.py "${SLICE_OUTPUT_PYTHONDIR}/easy/__init__.py" COPYONLY)
CONFIGURE_FILE(util/ArchiveHandler.py "${SLICE_OUTPUT_PYTHONDIR}/easy/util/ArchiveHandler.py" COPYONLY)
CONFIGURE_FILE(util/__init__.py "${SLICE_OUTPUT_PYTHONDIR}/easy/util/__init__.py" COPYONLY)

add_custom_target( easy ALL DEPENDS ${SLICE_FILES} )

