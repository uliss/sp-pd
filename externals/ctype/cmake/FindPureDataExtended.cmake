if(APPLE)
    set(PD_EXT_APP Pd-extended.app)
    find_path(PD_EXTENDED_APP_DIR ${PD_EXT_APP}
                HINTS /Applications)
endif()

if(PD_EXTENDED_APP_DIR)
    message(STATUS "PureData extended found: ${PD_EXTENDED_APP_DIR}/${PD_EXT_APP}")
    set(PD_INCLUDE_DIR "${PD_EXTENDED_APP_DIR}/${PD_EXT_APP}/Contents/Resources/include/pd")
else()
    message(FATAL_ERROR "PureData extended not found")
endif()
