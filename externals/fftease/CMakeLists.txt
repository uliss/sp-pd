project(fftease)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../")

cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0037 OLD)

include(FindPureDataExtended REQUIRED)

include_directories(${PD_INCLUDE_DIR})
include_directories("${CMAKE_SOURCE_DIR}/lib")

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


add_subdirectory(lib)
add_subdirectory(doc)

add_library(bthresher~ SHARED bthresher~.c)
target_link_libraries(bthresher~ fftease)

add_library(burrow~ SHARED burrow~.c)
target_link_libraries(burrow~ fftease)

add_library(cavoc~ SHARED cavoc~.c)
target_link_libraries(cavoc~ fftease)

add_library(cavoc27~ SHARED cavoc27~.c)
target_link_libraries(cavoc27~ fftease)

add_library(centerring~ SHARED centerring~.c)
target_link_libraries(centerring~ fftease)

add_library(codepend~ SHARED codepend~.c)
target_link_libraries(codepend~ fftease)

add_library(crossx~ SHARED crossx~.c)
target_link_libraries(crossx~ fftease)

add_library(dentist~ SHARED dentist~.c)
target_link_libraries(dentist~ fftease)

add_library(disarrain~ SHARED disarrain~.c)
target_link_libraries(disarrain~ fftease)

add_library(disarray~ SHARED disarray~.c)
target_link_libraries(disarray~ fftease)

add_library(drown~ SHARED drown~.c)
target_link_libraries(drown~ fftease)

add_library(ether~ SHARED ether~.c)
target_link_libraries(ether~ fftease)

add_library(leaker~ SHARED leaker~.c)
target_link_libraries(leaker~ fftease)

add_library(mindwarp~ SHARED mindwarp~.c)
target_link_libraries(mindwarp~ fftease)

add_library(morphine~ SHARED morphine~.c)
target_link_libraries(morphine~ fftease)

add_library(multyq~ SHARED multyq~.c)
target_link_libraries(multyq~ fftease)

add_library(presidency~ SHARED presidency~.c)
target_link_libraries(presidency~ fftease)

add_library(pvcompand~ SHARED pvcompand~.c)
target_link_libraries(pvcompand~ fftease)

add_library(pvoc~ SHARED pvoc~.c)
target_link_libraries(pvoc~ fftease)

add_library(pvharm~ SHARED pvharm~.c)
target_link_libraries(pvharm~ fftease)

add_library(pvgrain~ SHARED pvgrain~.c)
target_link_libraries(pvgrain~ fftease)

add_library(pvtuner~ SHARED pvtuner~.c)
target_link_libraries(pvtuner~ fftease)

add_library(pvwarp~ SHARED pvwarp~.c)
target_link_libraries(pvwarp~ fftease)

add_library(reanimator~ SHARED reanimator~.c)
target_link_libraries(reanimator~ fftease)

add_library(resent~ SHARED resent~.c)
target_link_libraries(resent~ fftease)

add_library(residency~ SHARED residency~.c)
target_link_libraries(residency~ fftease)

add_library(scrape~ SHARED scrape~.c)
target_link_libraries(scrape~ fftease)

add_library(shapee~ SHARED shapee~.c)
target_link_libraries(shapee~ fftease)

add_library(swinger~ SHARED swinger~.c)
target_link_libraries(swinger~ fftease)

add_library(taint~ SHARED taint~.c)
target_link_libraries(taint~ fftease)

add_library(thresher~ SHARED thresher~.c)
target_link_libraries(thresher~ fftease)

add_library(vacancy~ SHARED vacancy~.c)
target_link_libraries(vacancy~ fftease)

add_library(xsyn~ SHARED xsyn~.c)
target_link_libraries(xsyn~ fftease)

# installation
if(APPLE)
	set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.pd")
endif()

set(PD_TARGETS
	bthresher~ 
	burrow~
	cavoc27~
	cavoc~
	centerring~
	codepend~
	crossx~
	dentist~
	disarrain~
	disarray~
	drown~
	ether~
	leaker~
	mindwarp~
	morphine~
	multyq~
	presidency~
	pvcompand~
	pvgrain~
	pvharm~
	pvoc~
	pvtuner~
	pvwarp~
	reanimator~
	resent~
	residency~
	scrape~
	shapee~
	swinger~
	taint~
	thresher~
	vacancy~
	xsyn~
)

install(TARGETS ${PD_TARGETS}
	LIBRARY DESTINATION fftease)


# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)






