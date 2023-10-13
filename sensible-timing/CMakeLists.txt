# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense


# Library

add_library(${PROJECT_NAME}-timing SHARED
  src/sensible-timing.c
  src/sensible-timing.h
)
set_target_properties(${PROJECT_NAME}-timing PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME}-timing PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME}-timing PROPERTIES PUBLIC_HEADER src/sensible-timing.h)


# Test suite

add_subdirectory(test EXCLUDE_FROM_ALL)
