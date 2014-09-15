project(fsplay)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")

cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0037 OLD)
cmake_policy(SET CMP0022 OLD)

find_package(PureDataExtended REQUIRED)
set(USE_FLEXT_STATIC_THREAD TRUE)
find_package(Flext REQUIRED)
find_package(Boost REQUIRED)
find_package(QuickTime)
find_package(LibSndFile)

if(APPLE)
	set(ARCH "-arch i386")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARCH}")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ARCH}")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${ARCH}")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".pd_darwin")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ARCH} -undefined suppress -flat_namespace")
endif()

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PD_INCLUDE_DIR})
include_directories(${FLEXT_INCLUDE_DIR})

set(SOURCES
    main.cpp fsp_libsndfile.cpp 
)

set(LINK_LIBS ${FLEXT_LIBRARIES})

if(QUICKTIME_FOUND)
    list(APPEND SOURCES fsp_quicktime.cpp)
    include_directories(${QUICKTIME_INCLUDE_DIR})
    list(APPEND LINK_LIBS ${QUICKTIME_LIBRARY})
endif()

if(SNDFILE_FOUND)
    list(APPEND SOURCES fsp_libsndfile.cpp)
    include_directories(${SNDFILE_INCLUDE_DIR})
    list(APPEND LINK_LIBS ${SNDFILE_LIBRARIES})
endif()

add_library(fsplay~ SHARED ${SOURCES})
target_link_libraries(fsplay~ ${LINK_LIBS})

if(APPLE)
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.pd")
endif()

install(TARGETS fsplay~
    LIBRARY DESTINATION fsplay)
install(FILES fsplay~-help.pd DESTINATION fsplay)


include(Uninstall)
