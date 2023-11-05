# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0

add_library(${PROJECT_NAME}-macros INTERFACE)

target_sources(${PROJECT_NAME}-macros
  PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES
      include/sensible-macros-hedley.h
      include/sensible-macros.h
)

target_include_directories(${PROJECT_NAME}-macros INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

install(TARGETS ${PROJECT_NAME}-macros FILE_SET public_headers)
