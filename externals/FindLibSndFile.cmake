# - Find sndfile
# Find the native sndfile includes and libraries
#
# SNDFILE_INCLUDE_DIR - where to find sndfile.h, etc.
# SNDFILE_LIBRARIES - List of libraries when using libsndfile.
# SNDFILE_FOUND - True if libsndfile found.
# USE_SNDFILE_STATIC - if true, try to find static libs
if(SNDFILE_INCLUDE_DIR)
# Already in cache, be silent
	set(SNDFILE_FIND_QUIETLY TRUE)
endif(SNDFILE_INCLUDE_DIR)

find_path(SNDFILE_INCLUDE_DIR sndfile.h 
	PATHS /opt/local/include /sw/include)

set(LIB_NAMES sndfile sndfile-1)

if(USE_SNDFILE_STATIC)
	list(INSERT LIB_NAMES 0 libsndfile.a)
endif()

find_library(SNDFILE_LIBRARY NAMES ${LIB_NAMES}
	NO_DEFAULT_PATH
	PATHS /opt/local/lib /sw/lib)

# Handle the QUIETLY and REQUIRED arguments and set SNDFILE_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SNDFILE DEFAULT_MSG SNDFILE_LIBRARY SNDFILE_INCLUDE_DIR)
if(SNDFILE_FOUND)
	set(SNDFILE_LIBRARIES ${SNDFILE_LIBRARY})
else(SNDFILE_FOUND)
	set(SNDFILE_LIBRARIES)
endif(SNDFILE_FOUND)

mark_as_advanced(SNDFILE_INCLUDE_DIR SNDFILE_LIBRARY)
