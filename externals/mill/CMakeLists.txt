project(mill)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")

cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0037 OLD)

include(FindPureDataExtended REQUIRED)
include_directories(${PD_INCLUDE_DIR})

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


add_library(mill~ SHARED mill~.c)

if(APPLE)
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.pd")
endif()

install(TARGETS mill~
    LIBRARY DESTINATION mill)
install(FILES mill~-help.pd DESTINATION mill)


include(Uninstall)
