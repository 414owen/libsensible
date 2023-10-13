// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "../sensible-test/src/sensible-test.h"
#include "../sensible-test/test/suite.h"
#include "../sensible-data-structures/sensible-bitvec/test/suite.h"
#include "../sensible-allocators/sensible-arena/test/suite.h"

// This is the combined test suite for all sensible
// libraries. It uses shared libraries built in all
// testable subprojects.

int main(int argc, char **argv) {
  if (argc > 1) {
    int seed = atoi(argv[1]);
    printf("Using given seed: %d\n", seed);
    srand(seed);
  } else {
    clock_t now = time(NULL) + clock();
    now += time(NULL) + clock();
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
  run_sensible_bitvec_suite(state);
  run_sensible_arena_suite(state);
  return sentest_finish(state);
}
