project(tracker)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")

cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0037 OLD)
cmake_policy(SET CMP0022 OLD)

find_package(PureDataExtended REQUIRED)
include_directories(${PD_INCLUDE_DIR}/pdextended)

if(APPLE)
	set(ARCH "-arch i386 -arch x86_64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARCH}")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ARCH}")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${ARCH}")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".pd_darwin")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ARCH} -undefined suppress -flat_namespace")
endif()

add_custom_target(tracker_gui_gen
    COMMAND sh ${CMAKE_SOURCE_DIR}/tk2c.bash < ${CMAKE_SOURCE_DIR}/tracker.tk > ${CMAKE_BINARY_DIR}/tracker.tk2c
    COMMENT "Generating tracker.tk2c"
    DEPENDS ${CMAKE_SOURCE_DIR}/tracker.tk)

include_directories(${CMAKE_BINARY_DIR})
add_library(tracker SHARED tracker.c tracker.tk2c)
add_dependencies(tracker tracker_gui_gen)

if(APPLE)
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.pd")
endif()

install(TARGETS tracker
    LIBRARY DESTINATION tracker)
install(FILES tracker-help.pd DESTINATION tracker)

include(Uninstall)

