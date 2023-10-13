// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#if defined(WIN32)

#include <stdint.h>
#include <profileapi.h>

struct seninstant {
  uint64_t value;
};

#else

#define _XOPEN_SOURCE 500
// assume POSIX
#include <stdint.h>
#include <time.h>

struct seninstant {
  struct timespec value;
};

#endif

struct seninstant seninstant_now(void);
uint64_t seninstant_subtract(struct seninstant end, struct seninstant begin);
