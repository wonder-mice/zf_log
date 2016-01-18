cmake_minimum_required(VERSION 3.2)

include(ExternalProject)
include(CMakeParseArguments)
find_package(PythonInterp 2.7)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /WX")
else()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic-errors")
endif()

function(add_target target)
	cmake_parse_arguments(arg
		"STATICLIB;EXECUTABLE"
		""
		"SOURCES;DEFINES;INCLUDES;LIBRARIES"
		${ARGN})
	if(arg_STATICLIB)
		add_library(${target} STATIC ${arg_SOURCES})
	elseif(arg_EXECUTABLE)
		add_executable(${target} ${arg_SOURCES})
	else()
		message(FATAL_ERROR "Test target type is not specified.")
	endif()
	if(arg_DEFINES)
		set_property(TARGET ${target} PROPERTY COMPILE_DEFINITIONS "${arg_DEFINES}")
	endif()
	if(arg_INCLUDES)
		target_include_directories(${target} PRIVATE ${arg_INCLUDES})
	endif()
	if(arg_LIBRARIES)
		target_link_libraries(${target} ${arg_LIBRARIES})
	endif()
endfunction()

# spdlog
set(SPDLOG_DIR "${CMAKE_CURRENT_BINARY_DIR}/spdlog")
ExternalProject_Add(spdlog_ep
	PREFIX "${SPDLOG_DIR}"
	UPDATE_COMMAND ""
	GIT_REPOSITORY https://github.com/gabime/spdlog.git
	GIT_TAG e91e1b80f9c4332bcef8388ff48ee705128e5519
	CMAKE_ARGS
		"-G${CMAKE_GENERATOR}"
		"-DCMAKE_TOOLCHAIN_FILE:filepath=${CMAKE_TOOLCHAIN_FILE}"
		"-DCMAKE_INSTALL_PREFIX:path=<INSTALL_DIR>"
		"-DCMAKE_BUILD_TYPE:string=${CMAKE_BUILD_TYPE}"
)
add_library(spdlog INTERFACE)
add_dependencies(spdlog spdlog_ep)
set_target_properties(spdlog PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SPDLOG_DIR}/include")

# zf_log
set(ZF_LOG_DIR "${PROJECT_SOURCE_DIR}/zf_log")


# call site size
add_target(test_call_site_size.zf_log.1 STATICLIB
	SOURCES test_call_site_size.cpp
	DEFINES TEST_LIBRARY=zf_log
	INCLUDES "${ZF_LOG_DIR}")
add_target(test_call_site_size.zf_log.2 STATICLIB
	SOURCES test_call_site_size.cpp
	DEFINES TEST_LIBRARY=zf_log EXTRA_STATEMENT
	INCLUDES "${ZF_LOG_DIR}")
add_target(test_call_site_size.spdlog.1 STATICLIB
	SOURCES test_call_site_size.cpp
	DEFINES TEST_LIBRARY=spdlog
	LIBRARIES spdlog)
add_target(test_call_site_size.spdlog.2 STATICLIB
	SOURCES test_call_site_size.cpp
	DEFINES TEST_LIBRARY=spdlog EXTRA_STATEMENT
	LIBRARIES spdlog)

# min executable size
add_target(test_min_executable_size.zf_log EXECUTABLE
	SOURCES test_min_executable_size.cpp
	DEFINES TEST_LIBRARY=zf_log
	INCLUDES "${ZF_LOG_DIR}")
add_target(test_min_executable_size.zf_log_Os EXECUTABLE
	SOURCES test_min_executable_size.cpp
	DEFINES TEST_LIBRARY=zf_log ZF_LOG_OPTIMIZE_SIZE=1
	INCLUDES "${ZF_LOG_DIR}")
add_target(test_min_executable_size.spdlog EXECUTABLE
	SOURCES test_min_executable_size.cpp
	DEFINES TEST_LIBRARY=spdlog
	LIBRARIES spdlog)

# results
add_test(NAME run_tests COMMAND "${PYTHON_EXECUTABLE}"
	"${CMAKE_CURRENT_SOURCE_DIR}/run_tests.py"
	-o "${CMAKE_CURRENT_BINARY_DIR}/results.txt"
	-b "${CMAKE_BUILD_TYPE}"
	# call size size
	-p "call_site_size:zf_log:1:$<TARGET_FILE:test_call_site_size.zf_log.1>"
	-p "call_site_size:zf_log:2:$<TARGET_FILE:test_call_site_size.zf_log.2>"
	-p "call_site_size:spdlog:1:$<TARGET_FILE:test_call_site_size.spdlog.1>"
	-p "call_site_size:spdlog:2:$<TARGET_FILE:test_call_site_size.spdlog.2>"
	# min executable size
	-p "min_executable_size:zf_log:$<TARGET_FILE:test_min_executable_size.zf_log>"
	-p "min_executable_size:zf_log_Os:$<TARGET_FILE:test_min_executable_size.zf_log_Os>"
	-p "min_executable_size:spdlog:$<TARGET_FILE:test_min_executable_size.spdlog>")
