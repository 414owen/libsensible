# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

# Library

add_library(${PROJECT_NAME}-test SHARED
  src/sensible-test.c
  src/sensible-test.h
)
set_target_properties(${PROJECT_NAME}-test PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME}-test PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME}-test PROPERTIES PUBLIC_HEADER include/sensible-test.h)


# Test suite

add_subdirectory(test EXCLUDE_FROM_ALL)
add_subdirectory(examples EXCLUDE_FROM_ALL)
