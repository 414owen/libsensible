# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

add_library(${PROJECT_NAME}-timing-suite SHARED suite.c)
add_executable(${PROJECT_NAME}-timing-suite-exe main.c)
target_link_libraries(
  ${PROJECT_NAME}-timing-suite-exe
  ${PROJECT_NAME}-timing-suite
  ${PROJECT_NAME}-test
)
target_link_libraries(
  ${PROJECT_NAME}-timing-suite
  ${PROJECT_NAME}-test
  ${PROJECT_NAME}-timing
)

add_custom_target(${PROJECT_NAME}-timing-check
  COMMAND ${PROJECT_NAME}-timing-suite-exe
  COMMENT "Run test suite"
)

add_executable(${PROJECT_NAME}-timing-measure-sleep-variation measure-sleep-variation.c)

target_link_libraries(
  ${PROJECT_NAME}-timing-measure-sleep-variation
  ${PROJECT_NAME}-timing
)
