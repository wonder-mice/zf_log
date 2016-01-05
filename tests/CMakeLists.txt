cmake_minimum_required(VERSION 3.2)

include(CMakeParseArguments)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /WX")
else()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic-errors")
endif()

# zf_test
set(HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_library(zf_test INTERFACE)
target_include_directories(zf_test INTERFACE $<BUILD_INTERFACE:${HEADERS_DIR}>)
set(HEADERS zf_test.h)
add_custom_target(zf_test_headers SOURCES ${HEADERS})

function(add_test_target target)
	cmake_parse_arguments(arg
		""
		""
		"SOURCES;CSTD;CXXSTD;DEFINES"
		${ARGN})
	add_executable(${target} ${arg_SOURCES})
	if(arg_CSTD)
		set_property(TARGET ${target} PROPERTY C_STANDARD "${arg_CSTD}")
	endif()
	if(arg_CXXSTD)
		set_property(TARGET ${target} PROPERTY CXX_STANDARD "${arg_CXXSTD}")
	endif()
	set_property(TARGET ${target} PROPERTY COMPILE_DEFINITIONS "${arg_DEFINES}")
	target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}/zf_log")
	target_link_libraries(${target}  zf_test)
	add_test(NAME ${target} COMMAND ${target})
endfunction()

function(add_test_target_group target)
	add_test_target(${target}_c99 ${ARGN} CSTD 99)
	add_test_target(${target}_c11 ${ARGN} CSTD 11)
endfunction()

add_test_target_group(log_level_tests_verbose SOURCES log_level_tests.c DEFINES ZF_LOG_LEVEL=ZF_LOG_VERBOSE)
add_test_target_group(log_level_tests_debug SOURCES log_level_tests.c DEFINES ZF_LOG_LEVEL=ZF_LOG_DEBUG)
add_test_target_group(log_level_tests_info SOURCES log_level_tests.c DEFINES ZF_LOG_LEVEL=ZF_LOG_INFO)
add_test_target_group(log_level_tests_warn SOURCES log_level_tests.c DEFINES ZF_LOG_LEVEL=ZF_LOG_WARN)
add_test_target_group(log_level_tests_error SOURCES log_level_tests.c DEFINES ZF_LOG_LEVEL=ZF_LOG_ERROR)
add_test_target_group(log_level_tests_fatal SOURCES log_level_tests.c DEFINES ZF_LOG_LEVEL=ZF_LOG_FATAL)
add_test_target_group(log_level_tests_none SOURCES log_level_tests.c DEFINES ZF_LOG_LEVEL=ZF_LOG_NONE)
add_test_target_group(log_level_override_tests SOURCES log_level_override_tests.c)
add_test_target_group(extern_definition_tests SOURCES extern_definition_tests.c)
add_test_target_group(internals_tests SOURCES internals_tests.c)
add_test_target_group(log_message_tests SOURCES log_message_tests.c)
add_test_target_group(log_message_tests_Os SOURCES log_message_tests.c DEFINES ZF_LOG_OPTIMIZE_SIZE=1)
add_test_target_group(aux_spec_tests SOURCES aux_spec_tests.c)
add_test_target_group(decoration_test SOURCES decoration_module.c decoration_main.c)
add_test_target(cpp_compilation_tests SOURCES cpp_compilation_tests.cpp CXXSTD 11)
add_test_target(cpp_extern_definition_tests SOURCES cpp_extern_definition_tests.cpp CXXSTD 11)

# generated code size tests
add_executable(filesize_check filesize_check.c)
set(CODE_SIZE_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/code_size_tests")

# copy reference zf_log.h to compare with
add_custom_command(OUTPUT "${CODE_SIZE_SOURCE_DIR}/zf_log.h"
	COMMAND "${CMAKE_COMMAND}" -E copy_if_different
		"${CMAKE_CURRENT_SOURCE_DIR}/zf_log.h.master"
		"${CODE_SIZE_SOURCE_DIR}/zf_log.h"
	DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/zf_log.h.master")

function(save_preprocessor_output target)
	if(MSVC)
		# MSVC lacks this feature, /P suppresses compilation.
		#target_compile_options(${target} PRIVATE "/P")
	else()
		target_compile_options(${target} PRIVATE "-save-temps")
	endif()
endfunction()

function(add_code_size_test name)
	# "-save-temps" will put all ".s" files directly in the "tests" build
	# directory which is not specific for the target. Since it's useful to
	# examine those files, test source file will copied with different
	# names for *-prev and *-curr targets. That way generated "*.s" files
	# will have different names too.

	# copy source for prev
	add_custom_command(OUTPUT "${CODE_SIZE_SOURCE_DIR}/${name}-prev.c"
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different
			"${CMAKE_CURRENT_SOURCE_DIR}/${name}.c"
			"${CODE_SIZE_SOURCE_DIR}/${name}-prev.c"
		DEPENDS
			"${CMAKE_CURRENT_SOURCE_DIR}/${name}.c")
	# copy source for curr
	add_custom_command(OUTPUT "${CODE_SIZE_SOURCE_DIR}/${name}-curr.c"
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different
			"${CMAKE_CURRENT_SOURCE_DIR}/${name}.c"
			"${CODE_SIZE_SOURCE_DIR}/${name}-curr.c"
		DEPENDS
			"${CMAKE_CURRENT_SOURCE_DIR}/${name}.c")
	# prev library
	add_library(${name}-prev STATIC
		"${CODE_SIZE_SOURCE_DIR}/zf_log.h"
		"${CODE_SIZE_SOURCE_DIR}/${name}-prev.c")
	target_include_directories(${name}-prev PRIVATE "${CODE_SIZE_SOURCE_DIR}")
	save_preprocessor_output(${name}-prev)
	# curr library
	add_library(${name}-curr STATIC
		"${PROJECT_SOURCE_DIR}/zf_log/zf_log.h"
		"${CODE_SIZE_SOURCE_DIR}/${name}-curr.c")
	target_include_directories(${name}-curr PRIVATE "${PROJECT_SOURCE_DIR}/zf_log")
	save_preprocessor_output(${name}-curr)
	# test case
	add_dependencies(filesize_check ${name}-prev ${name}-curr)
	add_test(NAME ${name}_test
		COMMAND filesize_check "$<TARGET_FILE:${name}-prev>" "$<TARGET_FILE:${name}-curr>")
endfunction()

add_code_size_test(code_size_msg_only)
add_code_size_test(code_size_fmt_args)
