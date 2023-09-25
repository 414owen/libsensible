# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

cmake_minimum_required(VERSION 3.9)
project(sensible-test
  VERSION 1.0.0
  DESCRIPTION "Sensible test library"
  LANGUAGES C
)

set(CMAKE_C_STANDARD 99)

# Adds a compile option if we're on GCC or Clang
function(add_modern_compile_opt)
  if (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(${ARGV})
  endif()
endfunction()

add_modern_compile_opt(-pedantic -Wall -Wextra)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-O0)
  add_modern_compile_opt(-gdwarf-4 -fno-omit-frame-pointer)
endif()

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
