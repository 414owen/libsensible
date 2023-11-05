// SPDX-FileCopyrightText: 2023 The lisensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#define _XOPEN_SOURCE 500

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "sensible-timing.h"

#define MONOTONIC_CLOCK_TYPE CLOCK_MONOTONIC

#if defined(CLOCK_MONOTONIC_RAW)
# undef MONOTONIC_CLOCK_TYPE
# define MONOTONIC_CLOCK_TYPE CLOCK_MONOTONIC_RAW
#endif

#ifdef __FreeBSD__
# if defined(CLOCK_MONOTONIC_FAST)
#  undef MONOTONIC_CLOCK_TYPE
#  define MONOTONIC_CLOCK_TYPE CLOCK_MONOTONIC_FAST
# endif
#endif

// Linux has CLOCK_BOOTTIME
// OSX has CLOCK_UPTIME_RAW
// WE're not bothering with these.

senmac_public
struct seninstant seninstant_now(void) {
  struct seninstant res;
  if (clock_gettime(MONOTONIC_CLOCK_TYPE, &res.value) > 0) {
    perror("Couldn't get monotonic time");
  }
  return res;
}

senmac_public
uint64_t seninstant_subtract(struct seninstant end, struct seninstant begin) {
  // precondition: end >= begin
  // precondition: begin.tv_nsec < 1e9
  // precondition: end.tv_nsec < 1e9
  const uint64_t secs = end.value.tv_sec -= begin.value.tv_sec;
  uint64_t res = secs * 1e9;
  // TODO get rid of this by using signed integer arithmetic?
  if (end.value.tv_nsec > begin.value.tv_nsec) {
    res += end.value.tv_nsec - begin.value.tv_nsec;
  }
  else {
    res -= begin.value.tv_nsec - end.value.tv_nsec;
  }
  return res;
}

static
struct timespec ns_to_timespec(uint64_t nanos) {
  struct timespec res = {
    .tv_sec = nanos / (uint64_t) 1e9,
    .tv_nsec = nanos % (uint64_t) 1e9
  };
  return res;
}

senmac_public
bool sentiming_microsleep(uint64_t micros) {
  uint64_t ns = micros * 1000;
  struct timespec rem = ns_to_timespec(ns);
  while (nanosleep(&rem, &rem)) {
    if (errno != EINTR) {
      return false;
    }
  }
  return true;
}
