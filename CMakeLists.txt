# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

cmake_minimum_required(VERSION 3.9)
project(sensible
  VERSION 1.0.0
  DESCRIPTION "Sensible libraries"
  LANGUAGES C
)

set(CMAKE_C_STANDARD 99)

# Adds a compile option if we're on GCC or Clang
function(add_modern_compile_opt)
  if (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(${ARGV})
  endif()
endfunction()

add_modern_compile_opt(-pedantic -Wall -Wextra -fdata-sections -ffunction-sections)
if (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
  add_link_options(-Wl,--gc-sections)
endif()

# TODO migrate to unconditional CMAKE_C_FLAGS_DEBUG?
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-O0)
  if (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-gdwarf-4 -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
  endif()
endif()

add_subdirectory(test EXCLUDE_FROM_ALL)
add_subdirectory(sensible-test)
add_subdirectory(sensible-data-structures)
add_subdirectory(sensible-allocators)
add_subdirectory(sensible-timing)
