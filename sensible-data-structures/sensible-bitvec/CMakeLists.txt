# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0

# Library

add_library(${PROJECT_NAME}-bitvec SHARED src/sensible-bitvec.c)

target_link_libraries(
  ${PROJECT_NAME}-bitvec
  PUBLIC
    ${PROJECT_NAME}-macros
)


target_sources(${PROJECT_NAME}-bitvec
  PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/sensible-bitvec.h
)

set_target_properties(${PROJECT_NAME}-bitvec PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME}-bitvec PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME}-bitvec PROPERTIES PUBLIC_HEADER include/sensible-bitvec.h)

install(TARGETS ${PROJECT_NAME}-bitvec FILE_SET public_headers)


# Test suite

add_subdirectory(test EXCLUDE_FROM_ALL)
