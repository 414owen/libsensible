// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: CC0-1.0

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#include "../include/sensible-timing.h"

// minimum sleep found is around 14ms, so we use 50ms as a test
int main(void) {
  uint64_t min = 1e9;
  uint64_t max = 0;
  const uint64_t micros_to_sleep = 1000 * 1000;
  for (int i = 0; i < 50; i++) {
    struct seninstant begin = seninstant_now();
    sentiming_microsleep(micros_to_sleep);
    uint64_t nanos = seninstant_subtract(seninstant_now(), begin);
    if (nanos > max) { max = nanos; }
    if (nanos < min) { min = nanos; }
  }
  printf("target:  %" PRIu64 "\n", micros_to_sleep * 1000);
  printf("min:     %" PRIu64 "\n", min);
  printf("max:     %" PRIu64 "\n", max);
  printf("max-min: %" PRIu64 "\n", max - min);
}
