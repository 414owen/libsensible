# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0


# Library

add_library(${PROJECT_NAME}-args SHARED src/sensible-args.c)
set_target_properties(${PROJECT_NAME}-args PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME}-args PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

target_sources(${PROJECT_NAME}-args
  PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/sensible-args.h
)

target_link_libraries(
  ${PROJECT_NAME}-args
  PRIVATE
    ${PROJECT_NAME}-bitvec
    ${PROJECT_NAME}-macros
)

install(TARGETS ${PROJECT_NAME}-args FILE_SET public_headers)


# Test suite

add_subdirectory(test EXCLUDE_FROM_ALL)
add_subdirectory(examples EXCLUDE_FROM_ALL)
