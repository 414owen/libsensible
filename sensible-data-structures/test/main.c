// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../src/sensible-data-structures.h"
#include "../../sensible-test/src/sensible-test.h"

int main(void) {
  struct sentest_config config = {
    .output = stdout,
    .color = true,
    .filter_str = NULL,
    .junit_output_path = NULL,
  };
  struct sentest_state *state = sentest_start(config);

  sentest_group(state, "bitsets") {
    sentest(state, "can be allocated and freed") {
      struct bitset bs = bitset_new(10);
      bitset_free(&bs);
    }
    sentest_group(state, "allocated the right amount of cells for") {
      sentest(state, "a multiple of BITVECTOR_CELL_BITS") {
        for (size_t i = 2; i < 50; i++) {
          size_t length = BITVECTOR_CELL_BITS * i;
          struct bitset bs = bitset_new(length);
          sentest_assert_eq_fmt(state, bs.length, 0, "%zu");
          sentest_assert_eq_fmt(state, bs.capacity, i, "%zu");
          bitset_free(&bs);
        }
      }
    }
  }
  return sentest_finish(state);
}
