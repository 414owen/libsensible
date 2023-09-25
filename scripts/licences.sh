#!/usr/bin/env bash

set -euo pipefail

script_path="$(dirname "$0")"

LICENCE="// Copyright (c) 2023 The libsensible Authors All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file."

LINES=$(wc -l <(echo "$LICENCE"))

function check() {
  first_block="true"
  for file in $("$script_path"/source_files.sh); do
    if ! cmp --silent <(head -n 3 < "$file") <(echo "$LICENCE") > /dev/null; then
      if [ "$first_block" != "true" ]; then
        echo ""
        echo ""
      fi
      echo "${file} doesn't contain the desired licence."
      echo "Please insert this at the top:"
      echo ""
      echo "${LICENCE}"
      first_block=false
    fi
  done
}

function print_usage() {
  echo "Usage: $0 <command>
Valid commands:
  * check
  * update"
}

case "${1-""}" in
  check)
    check
    ;;
  *) 
    print_usage
    ;;
esac
