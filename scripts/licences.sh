#!/usr/bin/env bash

# Copyright (c) 2023 The libsensible Authors. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

set -euo pipefail

script_path="$(dirname "$0")"

TERM_ESCAPE="\x1B"
TERM_RED="${TERM_ESCAPE}[0;31m"
TERM_GREEN="${TERM_ESCAPE}[0;32m"
TERM_RESET="${TERM_ESCAPE}[0m"

# head of the license, so we can test for different years
LICENSE_1="// Copyright (c) YEAR The libsensible Authors. All rights reserved."
LICENSE_2="// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file."

FIRST_YEAR="2023"
YEAR="$(date +%Y)"

function file_has_license() {
  file="$1"
  head_matched=false
  for year in $(seq "$FIRST_YEAR" "$YEAR"); do
    if cmp --silent <(head -n 1 < "$file") <(echo "$LICENSE_1" | sed "s/YEAR/$year/g") > /dev/null; then
      head_matched=true
      break
    fi
  done
  if [ "$head_matched" == "false" ] || ! cmp --silent <(tail -n +2 < "$file" | head -n 2) <(echo "$LICENSE_2") > /dev/null; then
    return 1
  else
    return 0
  fi
}

function check() {
  failed="false"
  first_block="true"
  for file in $("$script_path"/source_files.sh); do
    printf "%s" "$file"
    if file_has_license "$file"; then
      printf " - ${TERM_GREEN}PASSED${TERM_RESET}\n"
    else
      failed="true"
      printf " - ${TERM_RED}FAILED${TERM_RESET}\n"
    fi
  done

  if [ "$failed" == "true" ]; then
    echo ""
    echo "Some files were missing the license header."
    echo "Please insert this at the top:"
    echo ""
    echo "${LICENSE_1}" | sed "s/YEAR/${YEAR}/g"
    echo "${LICENSE_2}"
    exit 1
  fi
}

function print_usage() {
  echo "Usage: $0 <command>
Valid commands:
  * check
  * update"
}

function update_files() {
  for file in $("$script_path"/source_files.sh); do
    printf "%s" "$file"
    if file_has_license "$file"; then
      printf " - PASSED\n"
    else
      ( echo "${LICENSE_1}" | sed "s/YEAR/${YEAR}/g"; printf "%s\n\n" "${LICENSE_2}"; cat "$file" ) > "${file}.tmp"
      mv "${file}.tmp" "$file"
      printf " - ${TERM_GREEN}UPDATED${TERM_RESET}\n"
    fi
  done
}

case "${1-""}" in
  check)
    check
    ;;
  update)
    update_files
    ;;
  *) 
    print_usage
    ;;
esac
