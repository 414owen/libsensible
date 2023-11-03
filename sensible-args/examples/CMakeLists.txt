# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

add_executable(${PROJECT_NAME}-args-example example.c)

target_link_libraries(
  ${PROJECT_NAME}-args-example
  ${PROJECT_NAME}-args
)
