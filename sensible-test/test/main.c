// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: CC0-1.0

#include <stdbool.h>
#include <stdio.h>

#include "suite.h"
#include "../src/sensible-test.h"

int main(void) {
  struct sentest_config config = {
    .output = stdout,
    .color = true,
    .filter_str = NULL,
    .junit_output_path = NULL,
  };
  struct sentest_state *state = sentest_start(config);
  run_sensible_test_suite(state);
  return sentest_finish(state);
}
