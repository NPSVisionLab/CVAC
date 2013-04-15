
#
# produce run scripts, put them in the binary folder
#

IF (WIN32)
    SET( SCRIPT_EXT bat )
ELSE (WIN32)
    SET( SCRIPT_EXT sh )
ENDIF (WIN32)

CONFIGURE_FILE( runIcebox.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/runIcebox.${SCRIPT_EXT} )
CONFIGURE_FILE( stopIcebox.${SCRIPT_EXT}.in  ${CMAKE_SOURCE_DIR}/bin/stopIcebox.${SCRIPT_EXT} )

INSTALL( FILES
                     ${CMAKE_SOURCE_DIR}/bin/runIcebox.${SCRIPT_EXT}
                     ${CMAKE_SOURCE_DIR}/bin/stopIcebox.${SCRIPT_EXT}
               DESTINATION bin
              )
