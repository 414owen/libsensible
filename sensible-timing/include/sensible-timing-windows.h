// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SENSIBLE_TIMING_WINDOWS_H
#define SENSIBLE_TIMING_WINDOWS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <stdint.h>
#include <profileapi.h>

#include "../../sensible-macros/include/sensible-macros.h"

struct seninstant {
  uint64_t value;
};

#ifdef __cplusplus
}
#endif

#endif
