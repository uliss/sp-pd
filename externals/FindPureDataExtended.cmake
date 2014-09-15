# - Find PureData 
# Find PureData includes and libraries
#
# PD_INCLUDE_DIR - where to find m_pd.h, etc.
# PD_FOUND - True if PureData found.

if(APPLE)
    set(PD_EXT_APP Pd-extended.app)
    find_path(PD_EXTENDED_APP_DIR ${PD_EXT_APP}
                HINTS /Applications)
    if(PD_EXTENDED_APP_DIR)
        message(STATUS "PureData extended found: ${PD_EXTENDED_APP_DIR}/${PD_EXT_APP}")
        set(PD_INCLUDE_DIR "${PD_EXTENDED_APP_DIR}/${PD_EXT_APP}/Contents/Resources/include/")
        set(PD_FOUND True)
    else()
        message(FATAL_ERROR "PureData extended not found")
        set(PD_FOUND False)
    endif()
endif()

# Handle the QUIETLY and REQUIRED arguments and set PD_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PD DEFAULT_MSG PD_INCLUDE_DIR)