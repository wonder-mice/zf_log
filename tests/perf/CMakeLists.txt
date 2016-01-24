cmake_minimum_required(VERSION 3.2)

include(ExternalProject)
include(CMakeParseArguments)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(PythonInterp 2.7 REQUIRED)

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
		"STATICLIB;EXECUTABLE;NO_THREADS"
		"TIME_COMPILE;TIME_LINK"
		"SOURCES;DEFINES;INCLUDES;COMPILE_OPTIONS;LIBRARIES"
		${ARGN})
	if(arg_STATICLIB)
		add_library(${target} STATIC ${arg_SOURCES})
	elseif(arg_EXECUTABLE)
		add_executable(${target} ${arg_SOURCES})
		if(NOT NO_THREADS)
			target_link_libraries(${target} Threads::Threads)
		endif()
	else()
		message(FATAL_ERROR "Test target type is not specified.")
	endif()
	if(arg_DEFINES)
		set_property(TARGET ${target} PROPERTY COMPILE_DEFINITIONS "${arg_DEFINES}")
	endif()
	if(arg_INCLUDES)
		target_include_directories(${target} PRIVATE ${arg_INCLUDES})
	endif()
	if(arg_COMPILE_OPTIONS)
		target_compile_options(${target} PRIVATE ${arg_COMPILE_OPTIONS})
	endif()
	if(arg_LIBRARIES)
		target_link_libraries(${target} ${arg_LIBRARIES})
	endif()
	if(arg_TIME_COMPILE)
		set_property(TARGET ${target} PROPERTY RULE_LAUNCH_COMPILE
			"\"${PYTHON_EXECUTABLE}\" \"${CMAKE_CURRENT_SOURCE_DIR}/time_it.py\" \"${arg_TIME_COMPILE}\"")
		set_property(TARGET ${target} PROPERTY TIME_COMPILE "${arg_TIME_COMPILE}")
	endif()
	if(arg_TIME_LINK)
		set_property(TARGET ${target} PROPERTY RULE_LAUNCH_LINK
			"\"${PYTHON_EXECUTABLE}\" \"${CMAKE_CURRENT_SOURCE_DIR}/time_it.py\" \"${arg_TIME_LINK}\"")
		set_property(TARGET ${target} PROPERTY TIME_LINK "${arg_TIME_LINK}")
	endif()
endfunction()

# zf_log
set(ZF_LOG_DIR "${PROJECT_SOURCE_DIR}/zf_log")
add_library(zf_log_n STATIC "${ZF_LOG_DIR}/zf_log.h" "${ZF_LOG_DIR}/zf_log.c")
target_include_directories(zf_log_n PUBLIC "${ZF_LOG_DIR}")
add_library(zf_log_Os STATIC "${ZF_LOG_DIR}/zf_log.h" "${ZF_LOG_DIR}/zf_log.c")
target_include_directories(zf_log_Os PUBLIC "${ZF_LOG_DIR}")
set_property(TARGET zf_log_Os PROPERTY COMPILE_DEFINITIONS "ZF_LOG_OPTIMIZE_SIZE")

# spdlog
set(SPDLOG_DIR "${CMAKE_CURRENT_BINARY_DIR}/spdlog")
ExternalProject_Add(spdlog_ep
	PREFIX "${SPDLOG_DIR}"
	UPDATE_COMMAND ""
	GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
	GIT_TAG "e91e1b80f9c4332bcef8388ff48ee705128e5519"
	CMAKE_GENERATOR "${CMAKE_GENERATOR}"
	CMAKE_ARGS
		"-DCMAKE_TOOLCHAIN_FILE:filepath=${CMAKE_TOOLCHAIN_FILE}"
		"-DCMAKE_INSTALL_PREFIX:path=<INSTALL_DIR>"
		"-DCMAKE_BUILD_TYPE:string=${CMAKE_BUILD_TYPE}"
		"-DCMAKE_OSX_ARCHITECTURES:string=${CMAKE_OSX_ARCHITECTURES}"
		"-DCMAKE_OSX_DEPLOYMENT_TARGET:string=${CMAKE_OSX_DEPLOYMENT_TARGET}"
		"-DCMAKE_OSX_SYSROOT:path=${CMAKE_OSX_SYSROOT}"
)
add_library(spdlog INTERFACE)
add_dependencies(spdlog spdlog_ep)
set_target_properties(spdlog PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SPDLOG_DIR}/include")

# easyloggingpp
set(EASYLOG_DIR "${CMAKE_CURRENT_BINARY_DIR}/easyloggingpp")
ExternalProject_Add(easylog_ep
	PREFIX "${EASYLOG_DIR}"
	UPDATE_COMMAND ""
	GIT_REPOSITORY "https://github.com/easylogging/easyloggingpp.git"
	GIT_TAG "f926802dfbde716d82b64b8ef3c25b7f0fcfec65"
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND "${CMAKE_COMMAND}" -E copy_directory
		"<SOURCE_DIR>/src" "<INSTALL_DIR>/include"
)
add_library(easylog INTERFACE)
add_dependencies(easylog easylog_ep)
set_target_properties(easylog PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${EASYLOG_DIR}/include")

# g3log
set(G3LOG_DIR "${CMAKE_CURRENT_BINARY_DIR}/g3log")
set(G3LOG_LIBRARY "${CMAKE_STATIC_LIBRARY_PREFIX}g3logger${CMAKE_STATIC_LIBRARY_SUFFIX}")
ExternalProject_Add(g3log_ep
	PREFIX "${G3LOG_DIR}"
	UPDATE_COMMAND ""
	GIT_REPOSITORY "https://github.com/KjellKod/g3log.git"
	GIT_TAG "1c6ede6db4fbb12006b61a913de737df56b9dd32"
	CMAKE_GENERATOR "${CMAKE_GENERATOR}"
	CMAKE_ARGS
		"-Wno-dev"
		"-DCMAKE_TOOLCHAIN_FILE:filepath=${CMAKE_TOOLCHAIN_FILE}"
		"-DCMAKE_INSTALL_PREFIX:path=<INSTALL_DIR>"
		"-DCMAKE_BUILD_TYPE:string=${CMAKE_BUILD_TYPE}"
		"-DCMAKE_OSX_ARCHITECTURES:string=${CMAKE_OSX_ARCHITECTURES}"
		"-DCMAKE_OSX_DEPLOYMENT_TARGET:string=${CMAKE_OSX_DEPLOYMENT_TARGET}"
		"-DCMAKE_OSX_SYSROOT:path=${CMAKE_OSX_SYSROOT}"
	INSTALL_COMMAND "${CMAKE_COMMAND}" -E copy_directory
		"<SOURCE_DIR>/src/g3log" "<INSTALL_DIR>/include/g3log"
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different
		"<BINARY_DIR>/${G3LOG_LIBRARY}" "<INSTALL_DIR>/lib/${G3LOG_LIBRARY}"
)
add_library(g3log INTERFACE)
add_dependencies(g3log g3log_ep)
set_target_properties(g3log PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${G3LOG_DIR}/include")
set_target_properties(g3log PROPERTIES INTERFACE_LINK_LIBRARIES "${G3LOG_DIR}/lib/${G3LOG_LIBRARY}")

# glog
set(GLOG_DIR "${CMAKE_CURRENT_BINARY_DIR}/glog")
set(GLOG_LIBRARY "${CMAKE_STATIC_LIBRARY_PREFIX}glog${CMAKE_STATIC_LIBRARY_SUFFIX}")
ExternalProject_Add(glog_ep
	PREFIX "${GLOG_DIR}"
	UPDATE_COMMAND ""
	GIT_REPOSITORY "https://github.com/google/glog.git"
	GIT_TAG "4d391fe692ae6b9e0105f473945c415a3ce5a401"
	CMAKE_GENERATOR "${CMAKE_GENERATOR}"
	CMAKE_ARGS
		"-Wno-dev"
		"-DCMAKE_TOOLCHAIN_FILE:filepath=${CMAKE_TOOLCHAIN_FILE}"
		"-DCMAKE_INSTALL_PREFIX:path=<INSTALL_DIR>"
		"-DCMAKE_BUILD_TYPE:string=${CMAKE_BUILD_TYPE}"
		"-DCMAKE_OSX_ARCHITECTURES:string=${CMAKE_OSX_ARCHITECTURES}"
		"-DCMAKE_OSX_DEPLOYMENT_TARGET:string=${CMAKE_OSX_DEPLOYMENT_TARGET}"
		"-DCMAKE_OSX_SYSROOT:path=${CMAKE_OSX_SYSROOT}"
)
add_library(glog INTERFACE)
add_dependencies(glog glog_ep)
set_target_properties(glog PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLOG_DIR}/include")
set_target_properties(glog PROPERTIES INTERFACE_LINK_LIBRARIES "${GLOG_DIR}/lib/${GLOG_LIBRARY}")

function(get_test_library lib var)
	if((lib STREQUAL "zf_log_n") OR (lib STREQUAL "zf_log_Os"))
		set(lib "zf_log")
	endif()
	set(${var} "${lib}" PARENT_SCOPE)
endfunction()

function(add_executable_size_test lib)
	get_test_library("${lib}" test_library)
	add_target(test_call_site_size.str.${lib}.1 STATICLIB LIBRARIES "${lib}"
		TIME_COMPILE "${CMAKE_CURRENT_BINARY_DIR}/compile_time.${lib}.json"
		SOURCES test_executable_size.cpp
		DEFINES "TEST_LIBRARY=${test_library}"
			"TEST_SEVERAL_STATEMENTS")
	add_target(test_call_site_size.str.${lib}.2 STATICLIB LIBRARIES "${lib}"
		SOURCES test_executable_size.cpp
		DEFINES "TEST_LIBRARY=${test_library}"
			"TEST_SEVERAL_STATEMENTS" "TEST_EXTRA_STATEMENT")
	add_target(test_call_site_size.fmti.${lib}.1 STATICLIB LIBRARIES "${lib}"
		SOURCES test_executable_size.cpp
		DEFINES "TEST_LIBRARY=${test_library}"
			"TEST_FORMAT_INTS" "TEST_SEVERAL_STATEMENTS")
	add_target(test_call_site_size.fmti.${lib}.2 STATICLIB LIBRARIES "${lib}"
		SOURCES test_executable_size.cpp
		DEFINES "TEST_LIBRARY=${test_library}"
			"TEST_FORMAT_INTS" "TEST_SEVERAL_STATEMENTS" "TEST_EXTRA_STATEMENT")
	add_target(test_executable_size.m1.${lib} EXECUTABLE LIBRARIES "${lib}" NO_THREADS
		TIME_LINK "${CMAKE_CURRENT_BINARY_DIR}/link_time.${lib}.json"
		SOURCES test_executable_size.cpp
		DEFINES "TEST_LIBRARY=${test_library}")
	list(APPEND PARAMETERS "-p" "call_site_size:str:${lib}:1:$<TARGET_FILE:test_call_site_size.str.${lib}.1>")
	list(APPEND PARAMETERS "-p" "call_site_size:str:${lib}:2:$<TARGET_FILE:test_call_site_size.str.${lib}.2>")
	list(APPEND PARAMETERS "-p" "call_site_size:fmti:${lib}:1:$<TARGET_FILE:test_call_site_size.fmti.${lib}.1>")
	list(APPEND PARAMETERS "-p" "call_site_size:fmti:${lib}:2:$<TARGET_FILE:test_call_site_size.fmti.${lib}.2>")
	list(APPEND PARAMETERS "-p" "executable_size:m1:${lib}:$<TARGET_FILE:test_executable_size.m1.${lib}>")
	list(APPEND PARAMETERS "-p" "compile_time:${lib}:$<TARGET_PROPERTY:test_call_site_size.str.${lib}.1,TIME_COMPILE>")
	list(APPEND PARAMETERS "-p" "link_time:${lib}:$<TARGET_PROPERTY:test_executable_size.m1.${lib},TIME_LINK>")
	set(PARAMETERS "${PARAMETERS}" PARENT_SCOPE)
endfunction()
add_executable_size_test(zf_log_n)
add_executable_size_test(zf_log_Os)
add_executable_size_test(spdlog)
add_executable_size_test(easylog)
add_executable_size_test(g3log)
add_executable_size_test(glog)

function(add_speed_test lib)
	get_test_library("${lib}" test_library)
	add_target(test_speed.str.${lib} EXECUTABLE
		SOURCES test_speed.cpp
		DEFINES "TEST_LIBRARY=${test_library}" "TEST_NULL_SINK"
		LIBRARIES "${lib}")
	add_target(test_speed.fmti.${lib} EXECUTABLE
		SOURCES test_speed.cpp
		DEFINES "TEST_LIBRARY=${test_library}" "TEST_NULL_SINK" "TEST_FORMAT_INTS"
		LIBRARIES "${lib}")
	list(APPEND PARAMETERS "-p" "speed:str:${lib}:$<TARGET_FILE:test_speed.str.${lib}>")
	list(APPEND PARAMETERS "-p" "speed:fmti:${lib}:$<TARGET_FILE:test_speed.fmti.${lib}>")
	set(PARAMETERS "${PARAMETERS}" PARENT_SCOPE)
endfunction()
add_speed_test(zf_log_n)
add_speed_test(zf_log_Os)
add_speed_test(spdlog)
add_speed_test(easylog)
add_speed_test(g3log)
add_speed_test(glog)

# results
add_test(NAME perf_tests COMMAND "${PYTHON_EXECUTABLE}"
	"${CMAKE_CURRENT_SOURCE_DIR}/run_tests.py"
	-o "${CMAKE_CURRENT_BINARY_DIR}/results.txt"
	-b "${CMAKE_BUILD_TYPE}"
	${PARAMETERS})
