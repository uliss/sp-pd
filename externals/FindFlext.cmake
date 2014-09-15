# - Find flext
# Find the native flext includes and libraries
#
# FLEXT_INCLUDE_DIR - where to find flext.h, etc.
# FLEXT_LIBRARIES - List of libraries when using flext.
# FLEXT_FOUND - True if libflext found.
# USE_FLEXT_SHARED - prefare shared lib
# USE_FLEXT_STATIC_THREAD - prefare static thread lib
# USE_FLEXT_STATIC_SINGLE - prefare static single lib
if(FLEXT_INCLUDE_DIR)
# Already in cache, be silent
	set(FLEXT_FIND_QUIETLY TRUE)
endif()

find_path(FLEXT_INCLUDE_DIR flext.h 
	PATHS /opt/local/include /sw/include
	PATH_SUFFIXES flext)

set(FLEXT_LIB_NAMES flext-pd flext-pd_s flext-pd_sd flext-pd_t flext-pd_td)

if(USE_FLEXT_SHARED)
	list(REMOVE_ITEM FLEXT_LIB_NAMES flext-pd)
	list(INSERT FLEXT_LIB_NAMES 0 flext-pd)
endif()

if(USE_FLEXT_STATIC_THREAD)
	list(REMOVE_ITEM FLEXT_LIB_NAMES flext-pd_t flext-pd_td)
	list(INSERT FLEXT_LIB_NAMES 0 flext-pd_t flext-pd_td)
endif()

if(USE_FLEXT_STATIC_SINGLE)
	list(REMOVE_ITEM FLEXT_LIB_NAMES flext-pd_s flext-pd_sd)
	list(INSERT FLEXT_LIB_NAMES 0 flext-pd_s flext-pd_sd)
endif()

find_library(FLEXT_LIBRARY NAMES ${FLEXT_LIB_NAMES}
	PATHS /opt/local/lib /sw/lib)

# Handle the QUIETLY and REQUIRED arguments and set FLEXT_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FLEXT DEFAULT_MSG FLEXT_LIBRARY FLEXT_INCLUDE_DIR)
if(FLEXT_FOUND)
	set(FLEXT_LIBRARIES ${FLEXT_LIBRARY})
	if(PD_FOUND)
		add_definitions(-DFLEXT_SYS_PD)
	endif()

	get_filename_component(EXTENSION ${FLEXT_LIBRARY} EXT)
	if(APPLE)
		if(EXTENSION STREQUAL ".a")
			# use posix threads
			add_definitions(-DFLEXT_THREADS=1)
		endif()

		if(EXTENSION STREQUAL ".dylib")
			add_definitions(-DFLEXT_SHARED)
		endif()
	endif()
else()
	set(FLEXT_LIBRARIES)
endif()

mark_as_advanced(FLEXT_INCLUDE_DIR FLEXT_LIBRARY)
