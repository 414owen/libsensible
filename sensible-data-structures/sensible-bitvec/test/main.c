// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: CC0-1.0

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../../../sensible-test/include/sensible-test.h"
#include "suite.h"

int main(void) {
  {
    time_t now = time(NULL);
    printf("Using random seed: %ld\n", now);
    srand(now);
  }
  struct sentest_config config = {
    .output = stdout,
    .color = true,
    .filter_str = NULL,
    .junit_output_path = NULL,
  };
  struct sentest_state *state = sentest_start(config);
  run_sensible_bitvec_suite(state);
  return sentest_finish(state);
}
