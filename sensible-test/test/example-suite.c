// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#include <stdbool.h>
#include <stdlib.h>

#include "sensible-test.h"

int main(void) {
  struct sentest_config config = {
    .filter_str = NULL,
    .junit = true,
  };
  struct sentest_state *state = sentest_start(config);
  sentest_group(state, "test-suite") {
    sentest_group(state, "trivial") {
      sentest(state, "assert true") {
        sentest_assert(state, true);
      }
    }
    sentest_group(state, "equality") {
      sentest(state, "one is two") {
        sentest_assert_eq(state, 1, 2);
      }
      sentest(state, "two is two") {
        sentest_assert_eq(state, 2, 2);
      }
    }
  }
  exit(sentest_finish(state));
}
