cmake_minimum_required (VERSION 2.6)

project (http-server)

set(CMAKE_CXX_STANDARD 14)

include_directories(include/)

file(GLOB SOURCES src/*.cc src/*.c include/*)
add_executable(http-server ${SOURCES} include/Config.hh src/Config.cc)
