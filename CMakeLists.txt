cmake_minimum_required(VERSION 3.0)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C++_COMPILER g++)

# output dir

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/clib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/clib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


set(PROJECT_NAME solve)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")

## project specific
project(${PROJECT_NAME})
add_executable(${PROJECT_NAME}
  main.c
)

target_link_libraries(${PROJECT_NAME} repo_package)

add_library(cJSON SHARED
  lib/cJSON/cJSON.h
  lib/cJSON/cJSON.c
)

target_include_directories(cJSON
  PUBLIC
  lib/cJSON
)

add_library(repo_package
  repo_package.c
  repo_package.h
)

target_link_libraries(repo_package cJSON)
