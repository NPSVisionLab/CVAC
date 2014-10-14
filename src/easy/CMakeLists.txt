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

# These python files are installed into src/easy so the python.config file
# can find these since they are run as python src/easy/xxxx.py
IF (NOT BUILD_3RDPARTY_PACKAGE)
    INSTALL(FILES corpus_service.py gui.py fileserver.py
        DESTINATION src/easy
        COMPONENT Runtime
        )
ENDIF (NOT BUILD_3RDPARTY_PACKAGE)
    

#
# Copy easy python files to the staging area so we can build a python egg
#
CONFIGURE_FILE(easy.py "${SLICE_OUTPUT_PYTHONDIR}/easy/easy.py" COPYONLY)
CONFIGURE_FILE(version.py "${SLICE_OUTPUT_PYTHONDIR}/easy/version.py" COPYONLY)
CONFIGURE_FILE(evaluate.py "${SLICE_OUTPUT_PYTHONDIR}/easy/evaluate.py" COPYONLY)
CONFIGURE_FILE(corpus_service.py "${SLICE_OUTPUT_PYTHONDIR}/easy/corpus_service.py" COPYONLY)
CONFIGURE_FILE(fileserver.py "${SLICE_OUTPUT_PYTHONDIR}/easy/fileserver.py" COPYONLY)
CONFIGURE_FILE(labelme.py "${SLICE_OUTPUT_PYTHONDIR}/easy/labelme.py" COPYONLY)
CONFIGURE_FILE(vatic.py "${SLICE_OUTPUT_PYTHONDIR}/easy/vatic.py" COPYONLY)
CONFIGURE_FILE(videosegment.py "${SLICE_OUTPUT_PYTHONDIR}/easy/videosegment.py" COPYONLY)
CONFIGURE_FILE(gui.py "${SLICE_OUTPUT_PYTHONDIR}/easy/gui.py" COPYONLY)
CONFIGURE_FILE(__init__.py "${SLICE_OUTPUT_PYTHONDIR}/easy/__init__.py" COPYONLY)
CONFIGURE_FILE(util/ArchiveHandler.py "${SLICE_OUTPUT_PYTHONDIR}/easy/util/ArchiveHandler.py" COPYONLY)
CONFIGURE_FILE(util/misc.py "${SLICE_OUTPUT_PYTHONDIR}/easy/util/misc.py" COPYONLY)
CONFIGURE_FILE(util/__init__.py "${SLICE_OUTPUT_PYTHONDIR}/easy/util/__init__.py" COPYONLY)

add_custom_target( easy ALL DEPENDS ${SLICE_FILES} )

