#!/usr/bin/env bash

set -euo pipefail

for dir in libsensible-*; do
  pushd "$dir"
  cmake -B build
  cmake --build build -t check
  popd
done
