#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2023 The libsensible Authors
#
# SPDX-License-Identifier: Unlicense

set -euo pipefail

for dir in sensible-*; do
  pushd "$dir"
  cmake -B build
  cmake --build build -t check
  popd
done
