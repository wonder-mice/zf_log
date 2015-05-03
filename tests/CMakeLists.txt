cmake_minimum_required(VERSION 3.2)

include(CMakeParseArguments)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic-errors")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic-errors")

# zf_test
set(HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_library(zf_test INTERFACE)
target_include_directories(zf_test INTERFACE $<BUILD_INTERFACE:${HEADERS_DIR}>)
set(HEADERS zf_test.h zf_test.hpp)
add_custom_target(zf_test_headers SOURCES ${HEADERS})

function(add_zf_log_test target)
    cmake_parse_arguments(arg
        ""
        ""
        "SOURCES;FLAGS"
        ${ARGN})
    add_executable(${target} ${arg_SOURCES})
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS "${arg_FLAGS}")
    target_link_libraries(${target} zf_log zf_test)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

add_zf_log_test(log_level_tests_c99
	SOURCES log_level_tests.c
	FLAGS -std=c99)
add_zf_log_test(log_level_tests_c11
	SOURCES log_level_tests.c
	FLAGS -std=c11)

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

function(add_code_size_test name)
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
	target_compile_options(${name}-prev PRIVATE  "-save-temps")
	# curr library
	add_library(${name}-curr STATIC
		"${PROJECT_SOURCE_DIR}/zf_log/zf_log.h"
		"${CODE_SIZE_SOURCE_DIR}/${name}-curr.c")
	target_include_directories(${name}-curr PRIVATE "${PROJECT_SOURCE_DIR}/zf_log")
	target_compile_options(${name}-curr PRIVATE  "-save-temps")
	# test case
	add_dependencies(filesize_check ${name}-prev ${name}-curr)
	add_test(NAME ${name}_test
		COMMAND filesize_check "$<TARGET_FILE:${name}-prev>" "$<TARGET_FILE:${name}-curr>")
endfunction()

add_code_size_test(code_size_msg_only)
add_code_size_test(code_size_fmt_args)
