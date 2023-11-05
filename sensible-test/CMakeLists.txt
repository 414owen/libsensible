# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0

# Library
# add_definitions(-DBUILD_DLL)

add_library(${PROJECT_NAME}-test SHARED src/sensible-test.c)

target_link_libraries(
  ${PROJECT_NAME}-test
  PUBLIC
    ${PROJECT_NAME}-macros
)

target_sources(${PROJECT_NAME}-test
  PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/sensible-test.h
)

set_target_properties(${PROJECT_NAME}-test PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME}-test PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(${PROJECT_NAME}-arena INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

install(TARGETS ${PROJECT_NAME}-test FILE_SET public_headers)

# Test suite

add_subdirectory(test EXCLUDE_FROM_ALL)
add_subdirectory(examples EXCLUDE_FROM_ALL)
