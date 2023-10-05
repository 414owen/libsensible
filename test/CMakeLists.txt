# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

add_executable(${PROJECT_NAME}-suite-exe main.c)
target_link_libraries(
  ${PROJECT_NAME}-suite-exe
  ${PROJECT_NAME}-test
  ${PROJECT_NAME}-test-suite
  ${PROJECT_NAME}-bitvec-suite
  ${PROJECT_NAME}-arena-suite
)

add_custom_target(check
  COMMAND ${PROJECT_NAME}-suite-exe
  COMMENT "Run test suite"
)
