// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../sensible-test/src/sensible-test.h"
#include "../sensible-test/test/suite.h"
#include "../sensible-data-structures/test/suite.h"

// This is the combined test suite for all sensible
// libraries. It uses shared libraries built in all
// testable subprojects.

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
    .junit_output_path = "test-results.xml",
  };
  struct sentest_state *state = sentest_start(config);
  run_sensible_test_suite(state);
  run_sensible_data_structure_suite(state);
  return sentest_finish(state);
}
