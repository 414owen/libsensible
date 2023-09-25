# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

add_executable(${PROJECT_NAME}-bench bench.c)
target_link_libraries(${PROJECT_NAME}-bench ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}-bench PRIVATE ../include)

add_custom_target(bench
  COMMAND ${PROJECT_NAME}-bench
  COMMENT "Run benchmark suite"
)

add_executable(${PROJECT_NAME}-suite main.c)
target_link_libraries(${PROJECT_NAME}-suite ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}-suite PRIVATE ../include)

add_custom_target(check
  COMMAND ${PROJECT_NAME}-suite
  COMMENT "Run test suite"
)
