# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0

# Library

add_library(${PROJECT_NAME}-arena SHARED src/sensible-arena.c)

target_link_libraries(
  ${PROJECT_NAME}-arena
  PRIVATE
    ${PROJECT_NAME}-macros
)

target_sources(${PROJECT_NAME}-arena
  PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/sensible-arena.h
)

set_target_properties(${PROJECT_NAME}-arena PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME}-arena PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(${PROJECT_NAME}-arena INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

install(TARGETS ${PROJECT_NAME}-arena FILE_SET public_headers)

# Test suite

add_subdirectory(test EXCLUDE_FROM_ALL)
