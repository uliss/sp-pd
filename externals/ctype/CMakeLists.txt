project(ctype)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

include(FindPureDataExtended)
include_directories(${PD_INCLUDE_DIR})

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

add_library(is_upper SHARED is_upper.c)
add_library(is_lower SHARED is_lower.c)
add_library(is_digit SHARED is_digit.c)
add_library(is_alpha SHARED is_alpha.c)
add_library(is_punct SHARED is_punct.c)

