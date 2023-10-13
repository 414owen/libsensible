// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdint.h>

#include "sensible-timing.h"

#if defined(WIN32)

#include <stdint.h>
#include <profileapi.h>

struct seninstant seninstant_now(void) {
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  struct seninstant res = {
    .value = now.QuadPart
  };
  return res;
}

// subtract beginning from end
uint64_t seninstant_subtract(struct seninstant end, struct seninstant begin) {
  uint64_t diff = end.value - start.value;

  // apparently no need to cache this, as internally it's just a read...
  LARGE_INTEGER freq;
  QueryPerformanceFrequency(&freq);

  {
    // optimization for most common case:
    // https://github.com/microsoft/STL/blob/785143a0c73f030238ef618890fd4d6ae2b3a3a0/stl/inc/chrono#L694-L701
    const uint64_t common_freq = 1e7;
    if (freq == common_freq) {
      // 1e9 is ns per s
      return diff * (1e9 / common_freq);
    }
  }

  const double nanoseconds_per_count = 1.0e9 / (double) freq;
  const uint64_t nanoseconds = (uint64_t) (diff * nanoseconds_per_count);
  return nanoseconds;
}

#else

// assume POSIX
#include <stdint.h>
#include <time.h>

#ifdef __linux__
# define MONOTONIC_CLOCK_TYPE CLOCK_BOOTTIME
#elif defined(__APPLE__)
# define MONOTONIC_CLOCK_TYPE CLOCK_UPTIME_RAW
#elseif defined(__FreeBSD__)
# define MONOTONIC_CLOCK_TYPE CLOCK_MONOTONIC_FAST
#else
# define MONOTONIC_CLOCK_TYPE CLOCK_MONOTONIC
#endif

struct seninstant seninstant_now(void) {
  struct seninstant res;
  if (clock_gettime(MONOTONIC_CLOCK_TYPE, &res.value) > 0) {
    perror("Couldn't get monotonic time");
  }
  return res;
}

uint64_t seninstant_subtract(struct seninstant end, struct seninstant begin) {
  // precondition: end.tv_nsec >= begin.tv_nsec
  const uint64_t secs = end.value.tv_sec -= begin.value.tv_sec;
  uint64_t res = secs * 1e9;
  // TODO get rid of this by using signed integer arithmetic?
  if (end.value.tv_nsec > begin.value.tv_nsec) {
    res += end.value.tv_nsec - begin.value.tv_nsec;
  } else {
    res -= begin.value.tv_nsec - end.value.tv_nsec;
  }
  return res;
}

#endif
