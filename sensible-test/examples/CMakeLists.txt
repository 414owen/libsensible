# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0

add_executable(${PROJECT_NAME}-test-example example.c)

target_link_libraries(
  ${PROJECT_NAME}-test-example
  ${PROJECT_NAME}-test
)
