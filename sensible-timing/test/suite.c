// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: CC0-1.0

#include "../include/sensible-timing.h"
#include "../../sensible-test/include/sensible-test.h"
#include "../../sensible-macros/include/sensible-macros.h"

#include <stdbool.h>
#include <inttypes.h>
#include <stdint.h>

senmac_public
void run_sensible_timing_suite(struct sentest_state *state) {
  sentest_group(state, "sensible-timing") {
    sentest(state, "Measures time including sleep") {
      struct seninstant begin = seninstant_now();
      const uint64_t micros_to_sleep = 1000 * 1000; // 1 second
      sentiming_microsleep(micros_to_sleep);
      uint64_t nanos = seninstant_subtract(seninstant_now(), begin);
      sentest_assert(state, nanos > micros_to_sleep * 1000);
      if (nanos <= micros_to_sleep * 1000) {
        printf("%" PRIu64 " %" PRIu64 "\n", nanos, micros_to_sleep * 1000);
      }
    }
  }
}
