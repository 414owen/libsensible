# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0


# Library

add_library(${PROJECT_NAME}-timing SHARED src/sensible-timing.c)

target_link_libraries(${PROJECT_NAME}-timing
  PRIVATE
    ${PROJECT_NAME}-macros
)

target_sources(${PROJECT_NAME}-timing
  PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/sensible-timing.h
      include/sensible-timing-windows.h
      include/sensible-timing-posix.h
)

set_target_properties(${PROJECT_NAME}-timing PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME}-timing PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(${PROJECT_NAME}-arena INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

install(TARGETS ${PROJECT_NAME}-timing FILE_SET public_headers)

# Test suite

add_subdirectory(test EXCLUDE_FROM_ALL)
