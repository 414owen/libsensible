# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

add_executable(${PROJECT_NAME}-data-structures-suite main.c)
target_link_libraries(${PROJECT_NAME}-data-structures-suite sensible-test)
target_link_libraries(${PROJECT_NAME}-data-structures-suite ${PROJECT_NAME}-data-structures)
link_directories(../../sensible-test)
target_include_directories(${PROJECT_NAME}-data-structures-suite PRIVATE ../include)

add_custom_target(data-structures-check
  COMMAND ${PROJECT_NAME}-data-structures-suite
  COMMENT "Run test suite"
)
