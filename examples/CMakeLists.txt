 cmake_minimum_required(VERSION 3.2)

 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic-errors")
 set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic-errors")

 add_executable(hello_c hello.c)
 target_link_libraries(hello_c zf_log)
