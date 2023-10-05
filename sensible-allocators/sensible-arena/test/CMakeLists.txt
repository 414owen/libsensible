# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

add_library(${PROJECT_NAME}-arena-suite SHARED suite.c)
add_executable(${PROJECT_NAME}-arena-suite-exe main.c)
target_link_libraries(
  ${PROJECT_NAME}-arena-suite-exe
  ${PROJECT_NAME}-arena-suite
  ${PROJECT_NAME}-test
)
target_link_libraries(
  ${PROJECT_NAME}-arena-suite
  ${PROJECT_NAME}-test
  ${PROJECT_NAME}-arena
)

add_custom_target(${PROJECT_NAME}-arena-check
  COMMAND ${PROJECT_NAME}-arena-suite-exe
  COMMENT "Run test suite"
)
