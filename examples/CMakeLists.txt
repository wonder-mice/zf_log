 cmake_minimum_required(VERSION 3.2)

 set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra -Werror -pedantic-errors")

 add_executable(hello_c hello.c)
 target_link_libraries(hello_c zf_log)

 add_executable(custom_output custom_output.c)
 target_link_libraries(custom_output zf_log)

 add_executable(args_eval args_eval.c)
 target_link_libraries(args_eval zf_log)
