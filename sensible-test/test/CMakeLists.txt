# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0

add_executable(${PROJECT_NAME}-test-bench-exe bench.c)

target_link_libraries(${PROJECT_NAME}-test-bench-exe
  PRIVATE
    ${PROJECT_NAME}-test
)

add_custom_target(${PROJECT_NAME}-test-bench
  COMMAND ${PROJECT_NAME}-test-bench-exe
  COMMENT "Run benchmark suite"
)

add_library(${PROJECT_NAME}-test-suite SHARED suite.c)

target_link_libraries(
  ${PROJECT_NAME}-test-suite
  PRIVATE
    ${PROJECT_NAME}-macros
    ${PROJECT_NAME}-test
)

add_executable(${PROJECT_NAME}-test-suite-exe main.c)

target_link_libraries(
  ${PROJECT_NAME}-test-suite-exe
  ${PROJECT_NAME}-test
  ${PROJECT_NAME}-test-suite
)

add_custom_target(${PROJECT_NAME}-test-check
  COMMAND ${PROJECT_NAME}-test-suite-exe
  COMMENT "Run test suite"
)
