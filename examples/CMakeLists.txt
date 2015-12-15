cmake_minimum_required(VERSION 3.2)

if(MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /Wall /WX")
else()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic-errors")
endif()

add_executable(hello_c hello.c)
target_link_libraries(hello_c zf_log)
set_property(TARGET hello_c PROPERTY C_STANDARD 99)

add_executable(custom_output custom_output.c)
target_link_libraries(custom_output zf_log)
set_property(TARGET custom_output PROPERTY C_STANDARD 99)

add_executable(file_output file_output.c)
target_link_libraries(file_output zf_log)
set_property(TARGET file_output PROPERTY C_STANDARD 99)

add_executable(args_eval args_eval.c)
target_link_libraries(args_eval zf_log)
set_property(TARGET args_eval PROPERTY C_STANDARD 99)
