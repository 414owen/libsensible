// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SENSIBLE_MACROS_H
#define SENSIBLE_MACROS_H

#include "sensible-macros-hedley.h"

#ifdef __cplusplus
extern "C" {
#endif

#define senmac_public HEDLEY_PUBLIC

#define restrict HEDLEY_RESTRICT

#define STATIC_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#ifdef __cplusplus
}
#endif

#endif
