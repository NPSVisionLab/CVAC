
#
# produce run scripts, put them in the binary folder
#

IF (WIN32)
    SET( SCRIPT_EXT bat )
ELSE (WIN32)
    SET( SCRIPT_EXT sh )
ENDIF (WIN32)

CONFIGURE_FILE( startIcebox.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/startIcebox.${SCRIPT_EXT} )
CONFIGURE_FILE( stopIcebox.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/stopIcebox.${SCRIPT_EXT} )

IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
INSTALL( PROGRAMS
                     ${CMAKE_SOURCE_DIR}/bin/startIcebox.${SCRIPT_EXT}
                     ${CMAKE_SOURCE_DIR}/bin/stopIcebox.${SCRIPT_EXT}
               DESTINATION bin
               COMPONENT Runtime
             )
ENDIF( NOT ${BUILD_3RDPARTY_PACKAGE} )
