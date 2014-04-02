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

IF (NOT BUILD_3RDPARY_PACKAGE)
    INSTALL(FILES corpus_service.py gui.py labelme.py evaluate.py vatic.py 
        DESTINATION src/easy
        COMPONENT Runtime
        )
ENDIF (NOT BUILD_3RDPARY_PACKAGE)
    

#
# Copy easy.py to the python lib dir without expanding
#
CONFIGURE_FILE(easy.py "${SLICE_OUTPUT_PYTHONDIR}/easy/easy.py" COPYONLY)
CONFIGURE_FILE(evaluate.py "${SLICE_OUTPUT_PYTHONDIR}/easy/evaluate.py" COPYONLY)
CONFIGURE_FILE(corpus_service.py "${SLICE_OUTPUT_PYTHONDIR}/easy/corpus_service.py" COPYONLY)
CONFIGURE_FILE(labelme.py "${SLICE_OUTPUT_PYTHONDIR}/easy/labelme.py" COPYONLY)
CONFIGURE_FILE(vatic.py "${SLICE_OUTPUT_PYTHONDIR}/easy/vatic.py" COPYONLY)
CONFIGURE_FILE(gui.py "${SLICE_OUTPUT_PYTHONDIR}/easy/gui.py" COPYONLY)
CONFIGURE_FILE(__init__.py "${SLICE_OUTPUT_PYTHONDIR}/easy/__init__.py" COPYONLY)
CONFIGURE_FILE(util/ArchiveHandler.py "${SLICE_OUTPUT_PYTHONDIR}/easy/util/ArchiveHandler.py" COPYONLY)
CONFIGURE_FILE(util/__init__.py "${SLICE_OUTPUT_PYTHONDIR}/easy/util/__init__.py" COPYONLY)

add_custom_target( easy ALL DEPENDS ${SLICE_FILES} )

