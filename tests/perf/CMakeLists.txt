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

# zf_log
set(ZF_LOG_DIR "${PROJECT_SOURCE_DIR}/zf_log")

# spdlog
set(SPDLOG_DIR "${CMAKE_CURRENT_BINARY_DIR}/spdlog")
ExternalProject_Add(spdlog_ep
	PREFIX "${SPDLOG_DIR}"
	UPDATE_COMMAND ""
	GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
	GIT_TAG "e91e1b80f9c4332bcef8388ff48ee705128e5519"
	CMAKE_ARGS
		"-G${CMAKE_GENERATOR}"
		"-DCMAKE_TOOLCHAIN_FILE:filepath=${CMAKE_TOOLCHAIN_FILE}"
		"-DCMAKE_INSTALL_PREFIX:path=<INSTALL_DIR>"
		"-DCMAKE_BUILD_TYPE:string=${CMAKE_BUILD_TYPE}"
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
	CMAKE_ARGS
		"-Wno-dev"
		"-G${CMAKE_GENERATOR}"
		"-DCMAKE_TOOLCHAIN_FILE:filepath=${CMAKE_TOOLCHAIN_FILE}"
		"-DCMAKE_INSTALL_PREFIX:path=<INSTALL_DIR>"
		"-DCMAKE_BUILD_TYPE:string=${CMAKE_BUILD_TYPE}"
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
	CMAKE_ARGS
		"-Wno-dev"
		"-G${CMAKE_GENERATOR}"
		"-DCMAKE_TOOLCHAIN_FILE:filepath=${CMAKE_TOOLCHAIN_FILE}"
		"-DCMAKE_INSTALL_PREFIX:path=<INSTALL_DIR>"
		"-DCMAKE_BUILD_TYPE:string=${CMAKE_BUILD_TYPE}"
)
add_library(glog INTERFACE)
add_dependencies(glog glog_ep)
set_target_properties(glog PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLOG_DIR}/include")
set_target_properties(glog PROPERTIES INTERFACE_LINK_LIBRARIES "${GLOG_DIR}/lib/${GLOG_LIBRARY}")

# call site size
add_target(test_call_site_size.zf_log.1 STATICLIB
	SOURCES test_call_site_size.cpp
	DEFINES TEST_LIBRARY=zf_log
	INCLUDES "${ZF_LOG_DIR}")
add_target(test_call_site_size.zf_log.2 STATICLIB
	SOURCES test_call_site_size.cpp
	DEFINES TEST_LIBRARY=zf_log EXTRA_STATEMENT
	INCLUDES "${ZF_LOG_DIR}")
add_target(test_call_site_size.zf_log_Os.1 STATICLIB
	SOURCES test_call_site_size.cpp
	DEFINES TEST_LIBRARY=zf_log ZF_LOG_OPTIMIZE_SIZE=1
	INCLUDES "${ZF_LOG_DIR}")
add_target(test_call_site_size.zf_log_Os.2 STATICLIB
	SOURCES test_call_site_size.cpp
	DEFINES TEST_LIBRARY=zf_log EXTRA_STATEMENT ZF_LOG_OPTIMIZE_SIZE=1
	INCLUDES "${ZF_LOG_DIR}")
add_target(test_call_site_size.spdlog.1 STATICLIB
	SOURCES test_call_site_size.cpp
	DEFINES TEST_LIBRARY=spdlog
	LIBRARIES spdlog)
add_target(test_call_site_size.spdlog.2 STATICLIB
	SOURCES test_call_site_size.cpp
	DEFINES TEST_LIBRARY=spdlog EXTRA_STATEMENT
	LIBRARIES spdlog)
add_target(test_call_site_size.easylog.1 STATICLIB
	SOURCES test_call_site_size.cpp
	DEFINES TEST_LIBRARY=easylog
	LIBRARIES easylog)
add_target(test_call_site_size.easylog.2 STATICLIB
	SOURCES test_call_site_size.cpp
	DEFINES TEST_LIBRARY=easylog EXTRA_STATEMENT
	LIBRARIES easylog)
add_target(test_call_site_size.g3log.1 STATICLIB
	SOURCES test_call_site_size.cpp
	DEFINES TEST_LIBRARY=g3log
	LIBRARIES g3log)
add_target(test_call_site_size.g3log.2 STATICLIB
	SOURCES test_call_site_size.cpp
	DEFINES TEST_LIBRARY=g3log EXTRA_STATEMENT
	LIBRARIES g3log)
add_target(test_call_site_size.glog.1 STATICLIB
	SOURCES test_call_site_size.cpp
	DEFINES TEST_LIBRARY=glog
	LIBRARIES glog)
add_target(test_call_site_size.glog.2 STATICLIB
	SOURCES test_call_site_size.cpp
	DEFINES TEST_LIBRARY=glog EXTRA_STATEMENT
	LIBRARIES glog)

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
add_target(test_min_executable_size.easylog EXECUTABLE
	SOURCES test_min_executable_size.cpp
	DEFINES TEST_LIBRARY=easylog
	LIBRARIES easylog)
add_target(test_min_executable_size.g3log EXECUTABLE
	SOURCES test_min_executable_size.cpp
	DEFINES TEST_LIBRARY=g3log
	LIBRARIES g3log)
add_target(test_min_executable_size.glog EXECUTABLE
	SOURCES test_min_executable_size.cpp
	DEFINES TEST_LIBRARY=glog
	LIBRARIES glog)

# results
add_test(NAME run_tests COMMAND "${PYTHON_EXECUTABLE}"
	"${CMAKE_CURRENT_SOURCE_DIR}/run_tests.py"
	-o "${CMAKE_CURRENT_BINARY_DIR}/results.txt"
	-b "${CMAKE_BUILD_TYPE}"
	# call size size
	-p "call_site_size:zf_log:1:$<TARGET_FILE:test_call_site_size.zf_log.1>"
	-p "call_site_size:zf_log:2:$<TARGET_FILE:test_call_site_size.zf_log.2>"
	-p "call_site_size:zf_log_Os:1:$<TARGET_FILE:test_call_site_size.zf_log_Os.1>"
	-p "call_site_size:zf_log_Os:2:$<TARGET_FILE:test_call_site_size.zf_log_Os.2>"
	-p "call_site_size:spdlog:1:$<TARGET_FILE:test_call_site_size.spdlog.1>"
	-p "call_site_size:spdlog:2:$<TARGET_FILE:test_call_site_size.spdlog.2>"
	-p "call_site_size:easylogging++:1:$<TARGET_FILE:test_call_site_size.easylog.1>"
	-p "call_site_size:easylogging++:2:$<TARGET_FILE:test_call_site_size.easylog.2>"
	-p "call_site_size:g3log:1:$<TARGET_FILE:test_call_site_size.g3log.1>"
	-p "call_site_size:g3log:2:$<TARGET_FILE:test_call_site_size.g3log.2>"
	-p "call_site_size:glog:1:$<TARGET_FILE:test_call_site_size.glog.1>"
	-p "call_site_size:glog:2:$<TARGET_FILE:test_call_site_size.glog.2>"
	# min executable size
	-p "min_executable_size:zf_log:$<TARGET_FILE:test_min_executable_size.zf_log>"
	-p "min_executable_size:zf_log_Os:$<TARGET_FILE:test_min_executable_size.zf_log_Os>"
	-p "min_executable_size:spdlog:$<TARGET_FILE:test_min_executable_size.spdlog>"
	-p "min_executable_size:easylogging++:$<TARGET_FILE:test_min_executable_size.easylog>"
	-p "min_executable_size:g3log:$<TARGET_FILE:test_min_executable_size.g3log>"
	-p "min_executable_size:glog:$<TARGET_FILE:test_min_executable_size.glog>")
