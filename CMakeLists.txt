cmake_minimum_required (VERSION 3.1)

project(MIMUW_SIK_TCP_MULTICAST)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-g -Wall -Wextra")

add_executable(time-client time-client6.c)
add_executable(time-server time-server6.c)
add_executable(multi-recv multi-recv.c)
add_executable(multi-recv6 multi-recv6.c)
add_executable(multi-send multi-send.c)
add_executable(multi-send6 multi-send6.c)

install(TARGETS DESTINATION .)
