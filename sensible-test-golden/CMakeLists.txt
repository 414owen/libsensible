# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0

# Library
# add_definitions(-DBUILD_DLL)

add_library(${PROJECT_NAME}-test-golden SHARED src/sensible-test-golden.c)

target_link_libraries(
  ${PROJECT_NAME}-test-golden
  PUBLIC
    ${PROJECT_NAME}-macros
)

target_sources(${PROJECT_NAME}-test-golden
  PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/sensible-test-golden.h
)

set_target_properties(${PROJECT_NAME}-test-golden PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME}-test-golden PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(${PROJECT_NAME}-arena INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

install(TARGETS ${PROJECT_NAME}-test-golden FILE_SET public_headers)
