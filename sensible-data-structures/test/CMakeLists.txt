# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

add_library(${PROJECT_NAME}-data-structures-suite SHARED suite.c)
add_executable(${PROJECT_NAME}-data-structures-suite-exe main.c)
target_link_libraries(
  ${PROJECT_NAME}-data-structures-suite-exe
  ${PROJECT_NAME}-data-structures-suite
  ${PROJECT_NAME}-test
)
target_link_libraries(
  ${PROJECT_NAME}-data-structures-suite
  ${PROJECT_NAME}-test
  ${PROJECT_NAME}-data-structures
)

add_custom_target(${PROJECT_NAME}-data-structures-check
  COMMAND ${PROJECT_NAME}-data-structures-suite-exe
  COMMENT "Run test suite"
)
