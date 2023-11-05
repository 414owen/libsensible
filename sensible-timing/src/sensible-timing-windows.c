// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#include <windows.h>
#include <stdint.h>
#include <synchapi.h>

#include "sensible-timing.h"
#include "sensible-macros.h"

senmac_public
struct seninstant seninstant_now(void) {
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  struct seninstant res = {
    .value = now.QuadPart
  };
  return res;
}

// subtract beginning from end
senmac_public
uint64_t seninstant_subtract(struct seninstant end, struct seninstant begin) {
  uint64_t diff = end.value - begin.value;

  // apparently no need to cache this, as internally it's just a read...
  LARGE_INTEGER freq;
  QueryPerformanceFrequency(&freq);

  {
    // optimization for most common case:
    // https://github.com/microsoft/STL/blob/785143a0c73f030238ef618890fd4d6ae2b3a3a0/stl/inc/chrono#L694-L701
    const uint64_t common_freq = 1e7;
    if (freq.QuadPart == common_freq) {
      // 1e9 is ns per s
      return diff * (1e9 / common_freq);
    }
  }

  const double nanoseconds_per_count = 1.0e9 / (double)freq.QuadPart;
  const uint64_t nanoseconds = (uint64_t)(diff * nanoseconds_per_count);
  return nanoseconds;
}

/*
senmac_public
bool sentiming_microsleep(uint64_t micros) {
  HANDLE timer;
  LARGE_INTEGER li;
  if (!(timer = CreateWaitableTimer(NULL, TRUE, NULL)))
    return false;
  // units are 100ns
  li.QuadPart = -micros * 10;
  if (!SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE)) {
    CloseHandle(timer);
    return false;
  }
  WaitForSingleObject(timer, INFINITE);
  CloseHandle(timer);
  return true;
}
*/

// This is hideously inaccurate, but the real-world accuracy is about the same as the above
// except that the above sometimes undersleeps, so we use this.
senmac_public
bool sentiming_microsleep(uint64_t micros) {
  Sleep(micros / 1000);
}
