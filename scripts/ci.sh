#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

set -euo pipefail

cmake -DCMAKE_BUILD_TYPE=Debug -B debug
cmake --build debug -t check
