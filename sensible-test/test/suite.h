// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: CC0-1.0

#ifndef SENSIBLE_TEST_SUITE_H
#define SENSIBLE_TEST_SUITE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sensible-macros.h"
#include "../include/sensible-test.h"

senmac_public void run_sensible_test_suite(struct sentest_state *state);

#ifdef __cplusplus
}
#endif

#endif
