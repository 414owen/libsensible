# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0

add_library(${PROJECT_NAME}-bitvec-suite SHARED suite.c)

target_link_libraries(
  ${PROJECT_NAME}-bitvec-suite
  PRIVATE
    ${PROJECT_NAME}-bitvec
  PUBLIC
    ${PROJECT_NAME}-macros
    ${PROJECT_NAME}-test
)

add_executable(${PROJECT_NAME}-bitvec-suite-exe main.c)

target_link_libraries(
  ${PROJECT_NAME}-bitvec-suite-exe
  PRIVATE
    ${PROJECT_NAME}-bitvec-suite
    ${PROJECT_NAME}-test
)

add_custom_target(${PROJECT_NAME}-bitvec-check
  COMMAND ${PROJECT_NAME}-bitvec-suite-exe
  COMMENT "Run test suite"
)
