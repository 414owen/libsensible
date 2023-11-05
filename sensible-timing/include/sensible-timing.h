// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SENSIBLE_TIMING_H
#define SENSIBLE_TIMING_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
# include "sensible-timing-windows.h"
#else
# include "sensible-timing-posix.h"
#endif

#include <stdbool.h>
#include <stdint.h>

#include "sensible-macros.h"

senmac_public struct seninstant seninstant_now(void);
senmac_public uint64_t seninstant_subtract(struct seninstant end, struct seninstant begin);

senmac_public bool sentiming_microsleep(uint64_t micros);

#ifdef __cplusplus
}
#endif

#endif
