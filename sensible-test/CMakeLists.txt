cmake_minimum_required(VERSION 3.9)
project(sensible-test
  VERSION 1.0.0
  DESCRIPTION "Sensible test library"
  LANGUAGES C
)

set(CMAKE_C_STANDARD 99)


# Library

add_library(${PROJECT_NAME} SHARED
  src/sensible-test.c
  include/sensible-test.h
)
target_include_directories(${PROJECT_NAME} PRIVATE include)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER include/sensible-test.h)



# Test suite

add_subdirectory(test EXCLUDE_FROM_ALL)