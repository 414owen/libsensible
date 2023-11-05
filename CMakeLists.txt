# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.9)
project(sensible
  VERSION 1.0.0
  DESCRIPTION "Sensible libraries"
  LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
  # Windows expects DLLS to be in the same path as the EXE, or in the
  # path or something...
  set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

# Adds a compile option if we're on GCC or Clang
function(add_modern_compile_opt)
  if (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(${ARGV})
  endif()
endfunction()

add_modern_compile_opt(-Wall -Wextra -fdata-sections -ffunction-sections)
if (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
  if (APPLE)
    add_link_options(-Wl,-dead_strip)
  else()
    add_link_options(-Wl,--gc-sections)
  endif()
endif()

# TODO migrate to unconditional CMAKE_C_FLAGS_DEBUG?
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-O0)
  if (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-pedantic -gdwarf-4 -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
  endif()
endif()

add_subdirectory(test EXCLUDE_FROM_ALL)
add_subdirectory(sensible-allocators)
add_subdirectory(sensible-args)
add_subdirectory(sensible-data-structures)
add_subdirectory(sensible-macros)
add_subdirectory(sensible-test)
add_subdirectory(sensible-timing)
