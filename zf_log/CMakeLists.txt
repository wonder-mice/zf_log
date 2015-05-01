cmake_minimum_required(VERSION 3.2)

# zf_log target (required)
set(HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(HEADERS zf_log.h)
set(SOURCES zf_log.c)

add_library(zf_log ${HEADERS} ${SOURCES})
target_include_directories(zf_log PUBLIC $<BUILD_INTERFACE:${HEADERS_DIR}>)
if(ANDROID)
	target_link_libraries(zf_log log)
endif()

# install (optional)
if(ZF_LOG_CONFIGURE_INSTALL)
	if(NOT DEFINED INSTALL_INCLUDE_DIR)
		set(INSTALL_INCLUDE_DIR include)
	endif()
	if(NOT DEFINED INSTALL_LIB_DIR)
		set(INSTALL_LIB_DIR lib)
	endif()
	install(TARGETS zf_log EXPORT zf_log
		INCLUDES DESTINATION ${INSTALL_INCLUDE_DIR}
		ARCHIVE DESTINATION ${INSTALL_LIB_DIR})
	install(DIRECTORY ${HEADERS_DIR}/
		DESTINATION ${INSTALL_INCLUDE_DIR}
		FILES_MATCHING PATTERN "zf_*.h*")
endif()
