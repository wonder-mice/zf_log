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
option(ZF_LOG_CONFIGURE_INSTALL "Generate install target" ON)
option(ZF_LOG_EXAMPLES "Build examples" OFF)
option(ZF_LOG_TESTS "Build tests" OFF)
option(ZF_LOG_ANDROID_LOG "Output to Android log" OFF)
option(ZF_LOG_APPLE_LOG "Output to Apple log" OFF)

add_subdirectory(zf_log)

if(ZF_LOG_EXAMPLES)
	add_subdirectory(examples)
endif()
if(ZF_LOG_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()

if(ZF_LOG_CONFIGURE_INSTALL)
	export(EXPORT zf_log)
	install(EXPORT zf_log
		DESTINATION ${INSTALL_CMAKE_DIR})
	configure_file(zf_log-config.cmake.in zf_log-config.cmake @ONLY)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/zf_log-config.cmake
		DESTINATION ${INSTALL_CMAKE_DIR})
endif()
