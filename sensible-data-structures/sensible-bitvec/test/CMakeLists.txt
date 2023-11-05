# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0

add_library(${PROJECT_NAME}-bitvec-suite SHARED suite.c)
add_executable(${PROJECT_NAME}-bitvec-suite-exe main.c)
target_link_libraries(
  ${PROJECT_NAME}-bitvec-suite-exe
  ${PROJECT_NAME}-bitvec-suite
  ${PROJECT_NAME}-test
)
target_link_libraries(
  ${PROJECT_NAME}-bitvec-suite
  ${PROJECT_NAME}-test
  ${PROJECT_NAME}-bitvec
)

add_custom_target(${PROJECT_NAME}-bitvec-check
  COMMAND ${PROJECT_NAME}-bitvec-suite-exe
  COMMENT "Run test suite"
)
