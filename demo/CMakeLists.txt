#
# 

cmake_minimum_required(VERSION 2.6)

WRAP_SLICE2PY( SLICE_FILES
    ${CMAKE_SOURCE_DIR}/src/Files.ice
    ${CMAKE_SOURCE_DIR}/src/Data.ice
    ${CMAKE_SOURCE_DIR}/src/Services.ice
    ${CMAKE_SOURCE_DIR}/src/Corpus.ice
)

add_custom_target( demo ALL DEPENDS ${SLICE_FILES} )

IF( BUILD_BINARY_PACKAGE )
  INSTALL(FILES bootstrapping.py demos.html detect.py full_image_corpus.py labelme_corpus.py prerequisites.py remote_services.py training.py runset.py
    DESTINATION demo
    COMPONENT Runtime
  )
ENDIF( BUILD_BINARY_PACKAGE )

INCLUDE (UserCMakeLists.txt OPTIONAL)
