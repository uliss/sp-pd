project(ffmpeg)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")


include(FindPureDataExtended)
include(FindFFmpeg)

if(!FFMPEG_FOUND)
    message(FATAL_ERROR "FFmpeg not found.")
endif()

include_directories(${FFMPEG_LIBAVCODEC_INCLUDE_DIRS} ${FFMPEG_LIBAVFORMAT_INCLUDE_DIRS})

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;i386")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".pd_darwin")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined suppress -flat_namespace")
endif()

add_library(ffmpeg_common STATIC ffmpeg_internal.cpp)
target_link_libraries(ffmpeg_common)

add_library(ffmpegfiler SHARED ffmpeg.c)
target_link_libraries(ffmpegfiler ${FFMPEG_LIBAVCODEC_LIBRARIES} ${FFMPEG_LIBAVFORMAT_LIBRARIES} ffmpeg_common)
