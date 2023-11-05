// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SENSIBLE_TIMING_POSIX_H
#define SENSIBLE_TIMING_POSIX_H

#ifdef __cplusplus
extern "C" {
#endif

#define _XOPEN_SOURCE 500

#include <stdint.h>
#include <time.h>

struct seninstant {
  struct timespec value;
};

#ifdef __cplusplus
}
#endif

#endif
