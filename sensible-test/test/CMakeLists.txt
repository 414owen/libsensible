# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

add_executable(${PROJECT_NAME}-test-bench bench.c)
target_link_libraries(${PROJECT_NAME}-test-bench ${PROJECT_NAME}-test)
target_include_directories(${PROJECT_NAME}-test-bench PRIVATE ../include)

add_custom_target(test-bench
  COMMAND ${PROJECT_NAME}-test-bench
  COMMENT "Run benchmark suite"
)

add_executable(${PROJECT_NAME}-test-suite main.c)
target_link_libraries(${PROJECT_NAME}-test-suite ${PROJECT_NAME}-test)
target_include_directories(${PROJECT_NAME}-test-suite PRIVATE ../include)

add_custom_target(test-check
  COMMAND ${PROJECT_NAME}-test-suite
  COMMENT "Run test suite"
)
