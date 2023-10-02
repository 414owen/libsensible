# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

# Library

add_library(${PROJECT_NAME}-bitvec SHARED
  src/sensible-bitvec.c
  src/sensible-bitvec.h
)
set_target_properties(${PROJECT_NAME}-bitvec PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME}-bitvec PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME}-bitvec PROPERTIES PUBLIC_HEADER src/sensible-bitvec.h)


# Test suite

add_subdirectory(test EXCLUDE_FROM_ALL)
