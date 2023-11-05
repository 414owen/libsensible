// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#ifndef SENSIBLE_TIMING_SUITE_H
#define SENSIBLE_TIMING_SUITE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../sensible-test/src/sensible-test.h"
#include "../../sensible-macros/include/sensible-macros.h"

senmac_public void run_sensible_timing_suite(struct sentest_state *state);

#ifdef __cplusplus
}
#endif

#endif
