# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0

# Library
# add_definitions(-DBUILD_DLL)

add_library(${PROJECT_NAME}-test SHARED src/sensible-test.c)
set_target_properties(${PROJECT_NAME}-test PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME}-test PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME}-test PROPERTIES PUBLIC_HEADER include/sensible-test.h)

install(TARGETS ${PROJECT_NAME}-test)

# Test suite

add_subdirectory(test EXCLUDE_FROM_ALL)
add_subdirectory(examples EXCLUDE_FROM_ALL)
