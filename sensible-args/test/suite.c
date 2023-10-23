// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#include "../src/sensible-args.h"
#include "../../sensible-test/src/sensible-test.h"

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

void run_sensible_args_suite(struct sentest_state *state) {
  sentest_group(state, "sensible-args") {
    sentest(state, "test stub") {
    }
  }
}
