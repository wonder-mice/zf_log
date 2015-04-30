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
