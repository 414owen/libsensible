// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#include "../src/sensible-timing.h"
#include "../../sensible-test/src/sensible-test.h"

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

void run_sensible_timing_suite(struct sentest_state *state) {
  sentest_group(state, "sensible-timing") {
    sentest(state, "Measures time including sleep") {
      struct seninstant begin = seninstant_now();
      sleep(1);
      uint64_t nanos = seninstant_subtract(seninstant_now(), begin);
      sentest_assert(state, nanos > 1e9);
      sentest_assert(state, nanos < 1e9 + 1e8);
    }
  }
}
