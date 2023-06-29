cmake_minimum_required(VERSION 3.17)
project(time_tick C)

add_subdirectory("deps/piot/clog/src/lib/")
add_subdirectory("deps/piot/tiny-libc/src/lib/")
add_subdirectory("deps/piot/monotonic-time-c/src/lib/")
add_subdirectory("lib")
#add_subdirectory("test")
#add_subdirectory("examples")