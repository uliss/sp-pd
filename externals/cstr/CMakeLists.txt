project(cstr)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

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

add_library(cstr_common STATIC utf8.c str_common.c)

add_library(substr SHARED substr.c)
target_link_libraries(substr cstr_common)

add_library(strlen SHARED strlen.c)
target_link_libraries(strlen cstr_common)

add_library(str_at SHARED str_at.c)
target_link_libraries(str_at cstr_common)

add_library(str_append SHARED str_append.c)
add_library(str_prepend SHARED str_prepend.c)
add_library(str_compare SHARED str_compare.c)

