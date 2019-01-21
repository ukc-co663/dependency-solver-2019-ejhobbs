cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C++_COMPILER g++)

set(PROJECT_NAME solve)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

## project specific
project(${PROJECT_NAME})
add_executable(${PROJECT_NAME}
  main.c
)

target_link_libraries(${PROJECT_NAME}
  cJSON
)

add_library(cJSON
  lib/cJSON/cJSON.h
  lib/cJSON/cJSON.c
)

target_include_directories(cJSON
  PUBLIC
  lib/cJSON
)

