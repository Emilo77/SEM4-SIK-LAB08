cmake_minimum_required (VERSION 3.1)

set(CMAKE_C_FLAGS "-g -Wall")

# http://stackoverflow.com/questions/10555706/
macro (add_executable _name)
  _add_executable(${ARGV})
  if (TARGET ${_name})
    target_link_libraries(${_name} err)
  endif()
endmacro()

add_library(err err.c)

add_executable(multi-recv multi-recv.c)
add_executable(multi-recv6 multi-recv6.c)
add_executable(multi-send multi-send.c)
add_executable(multi-send6 multi-send6.c)

install(TARGETS DESTINATION .)
