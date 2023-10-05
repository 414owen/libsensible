# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

# Library

add_library(${PROJECT_NAME}-arena SHARED
  src/sensible-arena.c
  src/sensible-arena.h
)
set_target_properties(${PROJECT_NAME}-arena PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME}-arena PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME}-arena PROPERTIES PUBLIC_HEADER src/sensible-arena.h)


# Test suite

add_subdirectory(test EXCLUDE_FROM_ALL)
