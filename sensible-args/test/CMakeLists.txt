# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0

add_library(${PROJECT_NAME}-args-suite SHARED suite.c)

target_link_libraries(
  ${PROJECT_NAME}-args-suite
  PRIVATE
    ${PROJECT_NAME}-args
    ${PROJECT_NAME}-macros
    ${PROJECT_NAME}-test
)

add_executable(${PROJECT_NAME}-args-suite-exe main.c)

target_link_libraries(
  ${PROJECT_NAME}-args-suite-exe
  PRIVATE
    ${PROJECT_NAME}-args-suite
    ${PROJECT_NAME}-test
)

add_custom_target(${PROJECT_NAME}-args-check
  COMMAND ${PROJECT_NAME}-args-suite-exe
  COMMENT "Run test suite"
)
