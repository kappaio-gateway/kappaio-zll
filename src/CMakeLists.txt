cmake_minimum_required(VERSION 2.8.10)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)

project(kappaio-zll C CXX)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -D__LITTLE_ENDIAN__")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -D__LITTLE_ENDIAN__")

include_directories(include)
include_directories(/usr/include/kappaio)
file(GLOB CPP_FILES source/*.cpp)
add_library(${PROJECT_NAME} SHARED ${CPP_FILES})
