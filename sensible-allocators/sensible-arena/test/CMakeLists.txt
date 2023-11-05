# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0

add_executable(${PROJECT_NAME}-arena-example example.c)
target_link_libraries(
  ${PROJECT_NAME}-arena-example
  ${PROJECT_NAME}-arena
)

add_executable(${PROJECT_NAME}-arena-bench-exe bench.c)

target_link_libraries(
  ${PROJECT_NAME}-arena-bench-exe
  PRIVATE
    ${PROJECT_NAME}-arena
    ${PROJECT_NAME}-macros
    ${PROJECT_NAME}-timing
)

IF (NOT WIN32)
  target_link_libraries(${PROJECT_NAME}-arena-bench-exe PRIVATE m)
ENDIF()

add_custom_target(${PROJECT_NAME}-arena-bench
  COMMAND ${PROJECT_NAME}-arena-bench-exe
  COMMENT "Run benchmark suite"
)

add_library(${PROJECT_NAME}-arena-suite SHARED suite.c)
add_executable(${PROJECT_NAME}-arena-suite-exe main.c)
target_link_libraries(
  ${PROJECT_NAME}-arena-suite-exe
  PRIVATE
    ${PROJECT_NAME}-arena-suite
    ${PROJECT_NAME}-test
)
target_link_libraries(
  ${PROJECT_NAME}-arena-suite
  PRIVATE
    ${PROJECT_NAME}-arena
    ${PROJECT_NAME}-test
    ${PROJECT_NAME}-macros
)

IF (NOT WIN32)
  target_link_libraries(${PROJECT_NAME}-arena-suite PRIVATE m)
ENDIF()

add_custom_target(${PROJECT_NAME}-arena-check
  COMMAND ${PROJECT_NAME}-arena-suite-exe
  COMMENT "Run test suite"
)
