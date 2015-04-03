cmake_minimum_required(VERSION 3.2)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
if(NOT DEFINED CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif()

project(zf_log)

set(INSTALL_INCLUDE_DIR include CACHE PATH
	"Installation directory for header files")
set(INSTALL_LIB_DIR lib CACHE PATH
	"Installation directory for libraries")
set(INSTALL_CMAKE_DIR lib/cmake/zf_queue CACHE PATH
	"Installation directory for CMake files")

# Don't add "-save-temps" to FLAGS - this will break Qt Creator debugger
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -Wextra -Werror -pedantic-errors")

# zf_log
add_subdirectory(zf_log)

# install
export(EXPORT zf_log)
install(EXPORT zf_log
	DESTINATION ${INSTALL_CMAKE_DIR})
configure_file(zf_log-config.cmake.in zf_log-config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/zf_log-config.cmake
	DESTINATION ${INSTALL_CMAKE_DIR})
