project(cstr)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../")

include(FindPureDataExtended)

if(APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -m32")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -m32")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".pd_darwin")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32 -undefined suppress -flat_namespace")
endif()


add_subdirectory(lib)

add_library(bthresher~ SHARED bthresher~.c)
target_link_libraries(bthresher~ fftease)

add_library(burrow~ SHARED burrow~.c)
target_link_libraries(burrow~ fftease)

add_library(pvoc~ SHARED pvoc~.c)
target_link_libraries(pvoc~ fftease)

add_library(pvharm~ SHARED pvharm~.c)
target_link_libraries(pvharm~ fftease)
