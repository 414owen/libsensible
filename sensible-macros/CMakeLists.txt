# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

add_library(${PROJECT_NAME}-macros INTERFACE)
target_sources(${PROJECT_NAME}-macros INTERFACE include/sensible-macros-hedley.h)
