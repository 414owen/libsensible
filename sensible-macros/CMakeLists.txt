# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: CC0-1.0

add_library(${PROJECT_NAME}-macros INTERFACE)
target_sources(${PROJECT_NAME}-macros INTERFACE include/sensible-macros-hedley.h)
