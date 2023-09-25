#!/usr/bin/env bash

set -euo pipefail

for dir in sensible-*; do
  pushd "$dir"
  cmake -B build
  cmake --build build -t check
  popd
done
