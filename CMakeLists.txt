cmake_minimum_required(VERSION 3.0)
set(CMAKE_C_COMPILER clang)
set(CMAKE_C++_COMPILER clang++)

# output dir

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/clib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/clib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


set(PROJECT_NAME solve)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")#-fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g ")#-fsanitize=address -fno-omit-frame-pointer")

## project specific
project(${PROJECT_NAME})
add_executable(${PROJECT_NAME}
  main.c
        state.c state.h)

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
        repository.c repository.h
        file.c file.h
        relation.c relation.h
        constraints.c constraints.h
        solver.c solver.h)

target_link_libraries(repo_package cJSON)

enable_testing()
add_executable(relation_test test/relation_test.c)
target_link_libraries(relation_test repo_package check)

add_executable(repository_test test/repository_test.c)
target_link_libraries(repository_test repo_package check)
add_test(relation_test ${CMAKE_CURRENT_BINARY_DIR}/relation_test)
add_test(repository_test ${CMAKE_CURRENT_BINARY_DIR}/repository_test)
