// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: CC0-1.0

#include "../src/sensible-test.h"

int main(void) {
  struct sentest_config config = {
    .output = stdout,
    .color = true,
    .filter_str = NULL,
    .junit_output_path = NULL,
  };

  struct sentest_state *state = sentest_start(config);

  sentest_group(state, "the number one") {
    sentest(state, "is number one") {
      sentest_assert_eq(state, 1, 1);
    }
    sentest(state, "isn't number two") {
      sentest_assert_neq(state, 1, 2);
    }
  }
  sentest_finish(state);
}
