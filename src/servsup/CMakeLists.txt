#
# 

cmake_minimum_required(VERSION 2.6)

MAKE_DIRECTORY(${SLICE_OUTPUT_PYTHONDIR})

#
# Copy python files to the staging area so we can build a python egg
#
CONFIGURE_FILE(ArchiveHandler.py "${SLICE_OUTPUT_PYTHONDIR}/servsup/ArchiveHandler.py" COPYONLY)
CONFIGURE_FILE(__init__.py "${SLICE_OUTPUT_PYTHONDIR}/servsup/__init__.py" COPYONLY)
CONFIGURE_FILE(cvutil.py "${SLICE_OUTPUT_PYTHONDIR}/servsup/cvutil.py" COPYONLY)
CONFIGURE_FILE(files.py "${SLICE_OUTPUT_PYTHONDIR}/servsup/files.py" COPYONLY)

