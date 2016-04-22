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

option(ZF_LOG_PERF_TEST_SAVE_TEMPS "Save preprocessor and disassembler output" OFF)
option(ZF_LOG_PERF_TEST_ZF_LOG_OS "Add zf_log built with ZF_LOG_OPTIMIZE_SIZE to performance tests" OFF)
option(ZF_LOG_PERF_TEST_VERBOSE_3P_BUILD "Enable verbose build output for 3rd party libraries (noisy)" OFF)

# Launch rules are target properties (RULE_LAUNCH_COMPILE, RULE_LAUNCH_LINK)
# that are used to time compilation and linking. Tests that require this
# properties will be disabled if current generator doesn't support launch
# rules. See CMake documentation for up to date list of generators that support
# launch rules.
if(CMAKE_GENERATOR MATCHES "Makefiles" OR
   CMAKE_GENERATOR MATCHES "Ninja")
	set(LAUNCH_RULES ON)
else()
	message(WARNING "Timing compilation and linking is not supported by \"${CMAKE_GENERATOR}\" generator!")
	set(LAUNCH_RULES OFF)
endif()
# Since a lot of 3rd party dependencies envolved, it's only so much we can do here.
# Non-Unix support is possible, but not in the scope right now.
if(NOT UNIX)
	message(WARNING "Performance tests only maintained for Unix platforms!")
endif()
# For convenience, ZF_LOG_PERF_TEST_VERBOSE_3P_BUILD is inverse of what is
# actually needed. SILENT_3P_BUILD variable will be used instead.
set(SILENT_3P_BUILD ON)
if(ZF_LOG_PERF_TEST_VERBOSE_3P_BUILD)
	set(SILENT_3P_BUILD OFF)
endif()

function(add_target target)
	cmake_parse_arguments(arg
		"STATICLIB;EXECUTABLE;NO_THREADS"
		"TIME_COMPILE;TIME_LINK"
		"SOURCES;DEFINES;INCLUDES;COMPILE_OPTIONS;LIBRARIES"
		${ARGN})
	if(ZF_LOG_PERF_TEST_SAVE_TEMPS)
		# Clang writes *.s and *.ii files into its current working directory.
		# Since the same source files are used in multiple targets, need to
		# copy them, so they will have different names for different targets.
		set(SOURCES "")
		foreach(source ${arg_SOURCES})
			if(IS_ABSOLUTE source)
				set(src "${source}")
			else()
				set(src "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
			endif()
			get_filename_component(src_name "${source}" NAME)
			set(dst "${CMAKE_CURRENT_BINARY_DIR}/sources/${target}-${src_name}")
			add_custom_command(OUTPUT "${dst}"
				COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${src}" "${dst}"
				DEPENDS "${src}")
			list(APPEND SOURCES "${dst}")
		endforeach()
	else()
		set(SOURCES ${arg_SOURCES})
	endif()
	if(arg_STATICLIB)
		add_library(${target} STATIC ${SOURCES})
	elseif(arg_EXECUTABLE)
		add_executable(${target} ${SOURCES})
		if(NOT NO_THREADS)
			target_link_libraries(${target} Threads::Threads)
		endif()
	else()
		message(FATAL_ERROR "Test target type is not specified.")
	endif()
	if(ZF_LOG_PERF_SAVE_TEMPS)
		target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
		target_compile_options(${target} PRIVATE "-save-temps")
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
	LOG_DOWNLOAD ${SILENT_3P_BUILD}
	LOG_UPDATE ${SILENT_3P_BUILD}
	LOG_CONFIGURE ${SILENT_3P_BUILD}
	LOG_BUILD ${SILENT_3P_BUILD}
	LOG_TEST ${SILENT_3P_BUILD}
	LOG_INSTALL ${SILENT_3P_BUILD}
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
	LOG_DOWNLOAD ${SILENT_3P_BUILD}
	LOG_UPDATE ${SILENT_3P_BUILD}
	LOG_CONFIGURE ${SILENT_3P_BUILD}
	LOG_BUILD ${SILENT_3P_BUILD}
	LOG_TEST ${SILENT_3P_BUILD}
	LOG_INSTALL ${SILENT_3P_BUILD}
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
		"-DUSE_DYNAMIC_LOGGING_LEVELS:bool=ON"
	INSTALL_COMMAND "${CMAKE_COMMAND}" -E copy_directory
		"<SOURCE_DIR>/src/g3log" "<INSTALL_DIR>/include/g3log"
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different
		"<BINARY_DIR>/${G3LOG_LIBRARY}" "<INSTALL_DIR>/lib/${G3LOG_LIBRARY}"
	LOG_DOWNLOAD ${SILENT_3P_BUILD}
	LOG_UPDATE ${SILENT_3P_BUILD}
	LOG_CONFIGURE ${SILENT_3P_BUILD}
	LOG_BUILD ${SILENT_3P_BUILD}
	LOG_TEST ${SILENT_3P_BUILD}
	LOG_INSTALL ${SILENT_3P_BUILD}
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
	LOG_DOWNLOAD ${SILENT_3P_BUILD}
	LOG_UPDATE ${SILENT_3P_BUILD}
	LOG_CONFIGURE ${SILENT_3P_BUILD}
	LOG_BUILD ${SILENT_3P_BUILD}
	LOG_TEST ${SILENT_3P_BUILD}
	LOG_INSTALL ${SILENT_3P_BUILD}
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

function(get_test_compile_options lib var)
	set(compile_options)
	if(lib STREQUAL "g3log")
		# g3log public headers generate "braced-scalar-init" warnings
		set(compile_options "-Wno-braced-scalar-init")
	endif()
	set(${var} "${compile_options}" PARENT_SCOPE)
endfunction()

function(add_executable_size_test lib)
	get_test_library("${lib}" test_library)
	get_test_compile_options("${lib}" compile_options)
	add_target(test_call_site_size.str.${lib}.1 STATICLIB LIBRARIES "${lib}"
		COMPILE_OPTIONS ${compile_options}
		TIME_COMPILE "${CMAKE_CURRENT_BINARY_DIR}/compile_time.${lib}.json"
		SOURCES test_executable_size.cpp
		DEFINES "TEST_LIBRARY=${test_library}"
			"TEST_SEVERAL_STATEMENTS")
	add_target(test_call_site_size.str.${lib}.2 STATICLIB LIBRARIES "${lib}"
		COMPILE_OPTIONS ${compile_options}
		SOURCES test_executable_size.cpp
		DEFINES "TEST_LIBRARY=${test_library}"
			"TEST_SEVERAL_STATEMENTS" "TEST_EXTRA_STATEMENT")
	add_target(test_call_site_size.fmti.${lib}.1 STATICLIB LIBRARIES "${lib}"
		COMPILE_OPTIONS ${compile_options}
		SOURCES test_executable_size.cpp
		DEFINES "TEST_LIBRARY=${test_library}"
			"TEST_FORMAT_INTS" "TEST_SEVERAL_STATEMENTS")
	add_target(test_call_site_size.fmti.${lib}.2 STATICLIB LIBRARIES "${lib}"
		COMPILE_OPTIONS ${compile_options}
		SOURCES test_executable_size.cpp
		DEFINES "TEST_LIBRARY=${test_library}"
			"TEST_FORMAT_INTS" "TEST_SEVERAL_STATEMENTS" "TEST_EXTRA_STATEMENT")
	add_target(test_executable_size.m1.${lib} EXECUTABLE LIBRARIES "${lib}" NO_THREADS
		COMPILE_OPTIONS ${compile_options}
		TIME_LINK "${CMAKE_CURRENT_BINARY_DIR}/link_time.${lib}.json"
		SOURCES test_executable_size.cpp
		DEFINES "TEST_LIBRARY=${test_library}")
	list(APPEND PARAMETERS "-p" "call_site_size:str:${lib}:1:$<TARGET_FILE:test_call_site_size.str.${lib}.1>")
	list(APPEND PARAMETERS "-p" "call_site_size:str:${lib}:2:$<TARGET_FILE:test_call_site_size.str.${lib}.2>")
	list(APPEND PARAMETERS "-p" "call_site_size:fmti:${lib}:1:$<TARGET_FILE:test_call_site_size.fmti.${lib}.1>")
	list(APPEND PARAMETERS "-p" "call_site_size:fmti:${lib}:2:$<TARGET_FILE:test_call_site_size.fmti.${lib}.2>")
	list(APPEND PARAMETERS "-p" "executable_size:m1:${lib}:$<TARGET_FILE:test_executable_size.m1.${lib}>")
	if(LAUNCH_RULES)
		list(APPEND PARAMETERS "-p" "compile_time:${lib}:$<TARGET_PROPERTY:test_call_site_size.str.${lib}.1,TIME_COMPILE>")
		list(APPEND PARAMETERS "-p" "link_time:${lib}:$<TARGET_PROPERTY:test_executable_size.m1.${lib},TIME_LINK>")
	endif()
	set(PARAMETERS "${PARAMETERS}" PARENT_SCOPE)
endfunction()

if(ZF_LOG_PERF_TEST_ZF_LOG_OS)
	add_executable_size_test(zf_log_Os)
endif()
add_executable_size_test(zf_log_n)
add_executable_size_test(spdlog)
add_executable_size_test(easylog)
add_executable_size_test(g3log)
add_executable_size_test(glog)

function(add_speed_test lib)
	get_test_library("${lib}" test_library)
	get_test_compile_options("${lib}" compile_options)
	add_target(test_speed.str.${lib} EXECUTABLE
		COMPILE_OPTIONS ${compile_options}
		SOURCES test_speed.cpp
		DEFINES "TEST_LIBRARY=${test_library}" "TEST_NULL_SINK"
		LIBRARIES "${lib}")
	add_target(test_speed.fmti.${lib} EXECUTABLE
		COMPILE_OPTIONS ${compile_options}
		SOURCES test_speed.cpp
		DEFINES "TEST_LIBRARY=${test_library}" "TEST_NULL_SINK" "TEST_FORMAT_INTS"
		LIBRARIES "${lib}")
	add_target(test_speed.str-off.${lib} EXECUTABLE
		COMPILE_OPTIONS ${compile_options}
		SOURCES test_speed.cpp
		DEFINES "TEST_LIBRARY=${test_library}" "TEST_NULL_SINK" "TEST_LOG_OFF"
		LIBRARIES "${lib}")
	add_target(test_speed.slowf-off.${lib} EXECUTABLE
		COMPILE_OPTIONS ${compile_options}
		SOURCES test_speed.cpp
		DEFINES "TEST_LIBRARY=${test_library}" "TEST_NULL_SINK" "TEST_FORMAT_SLOW_FUNC" "TEST_LOG_OFF"
		LIBRARIES "${lib}")
	list(APPEND PARAMETERS "-p" "speed:str:${lib}:$<TARGET_FILE:test_speed.str.${lib}>")
	list(APPEND PARAMETERS "-p" "speed:fmti:${lib}:$<TARGET_FILE:test_speed.fmti.${lib}>")
	list(APPEND PARAMETERS "-p" "speed:str-off:${lib}:$<TARGET_FILE:test_speed.str-off.${lib}>")
	list(APPEND PARAMETERS "-p" "speed:slowf-off:${lib}:$<TARGET_FILE:test_speed.slowf-off.${lib}>")
	set(PARAMETERS "${PARAMETERS}" PARENT_SCOPE)
endfunction()

if(ZF_LOG_PERF_TEST_ZF_LOG_OS)
	add_speed_test(zf_log_Os)
endif()
add_speed_test(zf_log_n)
add_speed_test(spdlog)
add_speed_test(easylog)
add_speed_test(g3log)
add_speed_test(glog)

# results
add_test(NAME perf_tests COMMAND "${PYTHON_EXECUTABLE}"
	"${CMAKE_CURRENT_SOURCE_DIR}/run_tests.py"
	-t "${CMAKE_CURRENT_BINARY_DIR}/results.txt"
	-m "${CMAKE_CURRENT_BINARY_DIR}/results.md"
	-b "${CMAKE_BUILD_TYPE}"
	-v
	${PARAMETERS})
