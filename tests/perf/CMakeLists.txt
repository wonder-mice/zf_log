cmake_minimum_required(VERSION 3.2)

include(ExternalProject)

set(SPDLOG_DIR "${CMAKE_CURRENT_BINARY_DIR}/spdlog")
ExternalProject_Add(spdlog_ep
	PREFIX "${SPDLOG_DIR}"
	UPDATE_COMMAND ""
	GIT_REPOSITORY https://github.com/gabime/spdlog.git
	GIT_TAG e91e1b80f9c4332bcef8388ff48ee705128e5519
	CMAKE_ARGS
		"-DCMAKE_INSTALL_PREFIX:string=<INSTALL_DIR>"
		"-DCMAKE_BUILD_TYPE:string=${CMAKE_BUILD_TYPE}"
)
add_library(spdlog INTERFACE IMPORTED)
add_dependencies(spdlog spdlog_ep)
set_target_properties(spdlog PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SPDLOG_DIR}/include")


set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /WX")
else()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic-errors")
endif()

add_executable(spdlog_perf spdlog.cpp)
set_property(TARGET spdlog_perf PROPERTY CXX_STANDARD "11")
target_link_libraries(spdlog_perf spdlog)

add_executable(zf_log_perf perf_zf_log.c)
set_property(TARGET zf_log_perf PROPERTY C_STANDARD "11")
target_include_directories(zf_log_perf PRIVATE "${PROJECT_SOURCE_DIR}/zf_log")
