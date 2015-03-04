cmake_minimum_required(VERSION 3.1)

set(HEADERS zf_log.h)
set(SOURCES zf_log.c)

add_library(zf_log ${HEADERS} ${SOURCES})
target_include_directories(zf_log PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if(ANDROID)
	target_link_libraries(zf_log log)
endif()
