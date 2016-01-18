cmake_minimum_required(VERSION 3.2)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /WX")
else()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic-errors")
endif()

if(ZF_LOG_LIBRARY_PREFIX)
	add_definitions("-DZF_LOG_LIBRARY_PREFIX=${ZF_LOG_LIBRARY_PREFIX}")
endif()

add_executable(hello_c hello.c)
target_link_libraries(hello_c zf_log)

add_executable(custom_output custom_output.c)
target_link_libraries(custom_output zf_log)

add_executable(file_output file_output.c)
target_link_libraries(file_output zf_log)

add_executable(args_eval args_eval.c)
target_link_libraries(args_eval zf_log)
