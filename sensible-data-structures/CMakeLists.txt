# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

# Library

add_library(${PROJECT_NAME}-data-structures SHARED
  src/sensible-data-structures.c
  src/sensible-data-structures.h
)
set_target_properties(${PROJECT_NAME}-data-structures PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME}-data-structures PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME}-data-structures PROPERTIES PUBLIC_HEADER src/sensible-data-structures.h)


# Test suite

add_subdirectory(test EXCLUDE_FROM_ALL)
