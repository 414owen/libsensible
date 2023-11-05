# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0

# Library

add_library(${PROJECT_NAME}-arena SHARED src/sensible-arena.c)
set_target_properties(${PROJECT_NAME}-arena PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME}-arena PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME}-arena PROPERTIES PUBLIC_HEADER include/sensible-arena.h)

install(TARGETS ${PROJECT_NAME}-arena)

# Test suite

add_subdirectory(test EXCLUDE_FROM_ALL)
