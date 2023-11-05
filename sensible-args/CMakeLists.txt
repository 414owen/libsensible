# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0


# Library

add_library(${PROJECT_NAME}-args SHARED src/sensible-args.c)
set_target_properties(${PROJECT_NAME}-args PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME}-args PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME}-args PROPERTIES PUBLIC_HEADER include/sensible-args.h)

install(TARGETS ${PROJECT_NAME}-args)

# Test suite

add_subdirectory(test EXCLUDE_FROM_ALL)
add_subdirectory(examples EXCLUDE_FROM_ALL)
