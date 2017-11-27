cmake_minimum_required (VERSION 2.6)

project (http-server)

set(CMAKE_CXX_STANDARD 14)

include_directories(include/)

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

file(GLOB SOURCES src/*.cc src/*.c include/*)
add_executable(http-server ${SOURCES} include/Config.hh src/Config.cc include/Timer.hh)
