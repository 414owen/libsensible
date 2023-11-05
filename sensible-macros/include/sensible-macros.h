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

// cl.exe doesn't seem to recognise 'restrict', even though
// it's standard C
#ifdef _WIN32
# define restrict HEDLEY_RESTRICT
#endif

#ifdef __cplusplus
}
#endif

#endif
