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
		"COMPILE_ONLY"
		""
		"SOURCES;CSTD;CXXSTD;DEFINES"
		${ARGN})
	if(arg_COMPILE_ONLY)
		add_library(${target} STATIC ${arg_SOURCES})
	else()
		add_executable(${target} ${arg_SOURCES})
		target_link_libraries(${target} zf_test)
		add_test(NAME ${target} COMMAND ${target})
	endif()
	if(arg_CSTD)
		set_property(TARGET ${target} PROPERTY C_STANDARD "${arg_CSTD}")
	endif()
	if(arg_CXXSTD)
		set_property(TARGET ${target} PROPERTY CXX_STANDARD "${arg_CXXSTD}")
	endif()
	set_property(TARGET ${target} PROPERTY COMPILE_DEFINITIONS "${arg_DEFINES}")
	target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}/zf_log")
endfunction()

function(add_test_target_group target)
	add_test_target(${target}_c99 ${ARGN} CSTD 99)
	add_test_target(${target}_c11 ${ARGN} CSTD 11)
endfunction()

add_test_target_group(test_log_level_switches_verbose SOURCES test_log_level_switches.c DEFINES ZF_LOG_LEVEL=ZF_LOG_VERBOSE)
add_test_target_group(test_log_level_switches_debug SOURCES test_log_level_switches.c DEFINES ZF_LOG_LEVEL=ZF_LOG_DEBUG)
add_test_target_group(test_log_level_switches_info SOURCES test_log_level_switches.c DEFINES ZF_LOG_LEVEL=ZF_LOG_INFO)
add_test_target_group(test_log_level_switches_warn SOURCES test_log_level_switches.c DEFINES ZF_LOG_LEVEL=ZF_LOG_WARN)
add_test_target_group(test_log_level_switches_error SOURCES test_log_level_switches.c DEFINES ZF_LOG_LEVEL=ZF_LOG_ERROR)
add_test_target_group(test_log_level_switches_fatal SOURCES test_log_level_switches.c DEFINES ZF_LOG_LEVEL=ZF_LOG_FATAL)
add_test_target_group(test_log_level_switches_none SOURCES test_log_level_switches.c DEFINES ZF_LOG_LEVEL=ZF_LOG_NONE)
add_test_target_group(test_log_level_override SOURCES test_log_level_override.c)

add_test_target_group(test_log_message_content SOURCES test_log_message_content.c)
add_test_target_group(test_log_message_content_Os SOURCES test_log_message_content.c DEFINES ZF_LOG_OPTIMIZE_SIZE=1)

add_test_target_group(test_log_message_content_empty_format SOURCES test_log_message_content.c DEFINES
	"ZF_LOG_MESSAGE_CTX_FORMAT=()"
	"ZF_LOG_MESSAGE_TAG_FORMAT=()"
	"ZF_LOG_MESSAGE_SRC_FORMAT=()")
add_test_target_group(test_log_message_content_empty_format_Os SOURCES test_log_message_content.c DEFINES
	"ZF_LOG_OPTIMIZE_SIZE=1"
	"ZF_LOG_MESSAGE_CTX_FORMAT=()"
	"ZF_LOG_MESSAGE_TAG_FORMAT=()"
	"ZF_LOG_MESSAGE_SRC_FORMAT=()")

# Here we use several F_INIT() fields in a row to workaround CMake problem with ";", which acts
# as a list separator in CMake. If not for CMake, context format specification could look like:
#   #define ZF_LOG_MESSAGE_CTX_FORMAT \
#       (YEAR, S("-ctx:"), F_INIT(( struct { int v; } f_box_a; f_box_a.v = 45; )), F_UINT(4, f_box_a.v))
add_test_target_group(test_log_message_content_custom_fields SOURCES test_log_message_content.c DEFINES
	"ZF_LOG_MESSAGE_CTX_FORMAT=(YEAR, S(\"-ctx:\"), F_INIT(( struct { int v )), F_INIT(( } f_box_a )), F_INIT(( f_box_a.v = 45 )), F_UINT(4, f_box_a.v))"
	"ZF_LOG_MESSAGE_TAG_FORMAT=(S(\"-tag:\"), F_INIT(( struct { int v )), F_INIT(( } f_box_b )), F_INIT(( f_box_b.v = 36 )), F_UINT(4, f_box_b.v))"
	"ZF_LOG_MESSAGE_SRC_FORMAT=(S(\"-src:\"), F_INIT(( struct { int v )), F_INIT(( } f_box_c )), F_INIT(( f_box_c.v = 27 )), F_UINT(4, f_box_c.v))")
add_test_target_group(test_log_message_content_custom_fields_Os SOURCES test_log_message_content.c DEFINES
	"ZF_LOG_OPTIMIZE_SIZE=1"
	"ZF_LOG_MESSAGE_CTX_FORMAT=(YEAR, S(\"ctx:\"), F_INIT(( struct { int v )), F_INIT(( } f_box_a )), F_INIT(( f_box_a.v = 45 )), F_UINT(4, f_box_a.v))"
	"ZF_LOG_MESSAGE_TAG_FORMAT=(S(\"tag:\"), F_INIT(( struct { int v )), F_INIT(( } f_box_b )), F_INIT(( f_box_b.v = 36 )), F_UINT(4, f_box_b.v))"
	"ZF_LOG_MESSAGE_SRC_FORMAT=(S(\"src:\"), F_INIT(( struct { int v )), F_INIT(( } f_box_c )), F_INIT(( f_box_c.v = 27 )), F_UINT(4, f_box_c.v))")

add_test_target_group(test_source_location_none SOURCES test_source_location.c
		DEFINES ZF_LOG_SRCLOC=ZF_LOG_SRCLOC_NONE TEST_SRCLOC=ZF_LOG_SRCLOC_NONE)
add_test_target_group(test_source_location_short SOURCES test_source_location.c
		DEFINES ZF_LOG_SRCLOC=ZF_LOG_SRCLOC_SHORT TEST_SRCLOC=ZF_LOG_SRCLOC_SHORT)
add_test_target_group(test_source_location_long SOURCES test_source_location.c
		DEFINES ZF_LOG_SRCLOC=ZF_LOG_SRCLOC_LONG TEST_SRCLOC=ZF_LOG_SRCLOC_LONG)
add_test_target_group(test_source_location_none_Os SOURCES test_source_location.c
		DEFINES ZF_LOG_SRCLOC=ZF_LOG_SRCLOC_NONE TEST_SRCLOC=ZF_LOG_SRCLOC_NONE ZF_LOG_OPTIMIZE_SIZE=1)
add_test_target_group(test_source_location_short_Os SOURCES test_source_location.c
		DEFINES ZF_LOG_SRCLOC=ZF_LOG_SRCLOC_SHORT TEST_SRCLOC=ZF_LOG_SRCLOC_SHORT ZF_LOG_OPTIMIZE_SIZE=1)
add_test_target_group(test_source_location_long_Os SOURCES test_source_location.c
		DEFINES ZF_LOG_SRCLOC=ZF_LOG_SRCLOC_LONG TEST_SRCLOC=ZF_LOG_SRCLOC_LONG ZF_LOG_OPTIMIZE_SIZE=1)

add_test_target_group(test_conditional SOURCES test_conditional.c)
add_test_target_group(test_censoring_off SOURCES test_censoring.c DEFINES ZF_LOG_CENSORING=ZF_LOG_UNCENSORED TEST_LOG_SECRETS=1)
add_test_target_group(test_censoring_on SOURCES test_censoring.c DEFINES ZF_LOG_CENSORING=ZF_LOG_CENSORED TEST_LOG_SECRETS=0)

add_test_target_group(test_private_parts SOURCES test_private_parts.c)
add_test_target_group(test_decoration SOURCES test_decoration.module.c test_decoration.main.c)
add_test_target_group(test_aux_spec SOURCES test_aux_spec.c)
add_test_target_group(test_builtin_output_facilities SOURCES test_builtin_output_facilities.c COMPILE_ONLY)
add_test_target_group(test_externally_defined_state SOURCES test_externally_defined_state.c)
add_test_target(test_externally_defined_state_cpp SOURCES test_externally_defined_state_cpp.cpp CXXSTD 11)
add_test_target(test_compilation_cpp SOURCES test_compilation_cpp.cpp CXXSTD 11)

# generated code size tests
add_executable(filesize_check filesize_check.c)
set(CODE_SIZE_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/code_size_tests")

function(save_preprocessor_output target)
	if(MSVC)
		# MSVC lacks this feature, /P suppresses compilation.
		#target_compile_options(${target} PRIVATE "/P")
	else()
		target_compile_options(${target} PRIVATE "-save-temps")
	endif()
endfunction()

function(add_copy_command src dst)
	add_custom_command(OUTPUT "${dst}"
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${src}" "${dst}"
		DEPENDS "${src}")
	set_source_files_properties("${dst}" PROPERTIES GENERATED TRUE)
endfunction()

# copy reference and current versions of zf_log.h
add_copy_command("${CMAKE_CURRENT_SOURCE_DIR}/zf_log.h.master" "${CODE_SIZE_SOURCE_DIR}/prev/zf_log.h")
add_copy_command("${PROJECT_SOURCE_DIR}/zf_log/zf_log.h" "${CODE_SIZE_SOURCE_DIR}/curr/zf_log.h")

function(add_code_size_test target)
	cmake_parse_arguments(arg
		""
		""
		"SOURCES;DEFINES"
		${ARGN})
	set(prev_sources "${CODE_SIZE_SOURCE_DIR}/prev/zf_log.h")
	set(curr_sources "${CODE_SIZE_SOURCE_DIR}/curr/zf_log.h")
	foreach(src ${arg_SOURCES})
		# "-save-temps" will put all ".s" files directly in the "tests" build
		# directory which is not specific for the target. Since it's useful to
		# examine those files, test sources will be copied with different
		# names for *-prev and *-curr targets. That way generated "*.s" files
		# will have different names too and will not overwrite each other.
		get_filename_component(src_name "${src}" NAME_WE)
		get_filename_component(src_ext "${src}" EXT)
		get_filename_component(src_path "${src}" ABSOLUTE)
		set(src_copy_prev "${CODE_SIZE_SOURCE_DIR}/${target}-${src_name}-prev${src_ext}")
		set(src_copy_curr "${CODE_SIZE_SOURCE_DIR}/${target}-${src_name}-curr${src_ext}")
		add_copy_command("${src_path}" "${src_copy_prev}")
		add_copy_command("${src_path}" "${src_copy_curr}")
		list(APPEND prev_sources "${src_copy_prev}")
		list(APPEND curr_sources "${src_copy_curr}")
	endforeach()

	# prev library
	add_library(${target}-prev STATIC ${prev_sources})
	target_include_directories(${target}-prev PRIVATE "${CODE_SIZE_SOURCE_DIR}/prev")
	target_compile_definitions(${target}-prev PRIVATE ${arg_DEFINES})
	save_preprocessor_output(${target}-prev)
	# curr library
	add_library(${target}-curr STATIC ${curr_sources})
	target_include_directories(${target}-curr PRIVATE "${CODE_SIZE_SOURCE_DIR}/curr")
	target_compile_definitions(${target}-curr PRIVATE ${arg_DEFINES})
	save_preprocessor_output(${target}-curr)
	# test case
	add_dependencies(filesize_check ${target}-prev ${target}-curr)
	add_test(NAME ${target}_check
		COMMAND filesize_check "$<TARGET_FILE:${target}-prev>" "$<TARGET_FILE:${target}-curr>")
endfunction()

add_code_size_test(test_call_site_size_msg_only SOURCES test_call_site_size_msg_only.c)
add_code_size_test(test_call_site_size_fmt_args SOURCES test_call_site_size_fmt_args.c)
add_code_size_test(test_call_site_size_conditional_true  SOURCES test_call_site_size_conditional.c DEFINES TEST_CONDITION=1)
add_code_size_test(test_call_site_size_conditional_false SOURCES test_call_site_size_conditional.c DEFINES TEST_CONDITION=0)
add_code_size_test(test_call_site_size_censoring_on  SOURCES test_call_site_size_censoring.c DEFINES ZF_LOG_CENSORING=ZF_LOG_CENSORED)
add_code_size_test(test_call_site_size_censoring_off SOURCES test_call_site_size_censoring.c DEFINES ZF_LOG_CENSORING=ZF_LOG_UNCENSORED)
