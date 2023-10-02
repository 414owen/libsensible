// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../src/sensible-data-structures.h"
#include "../../sensible-test/src/sensible-test.h"

#define STATIC_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

void run_sensible_data_structure_suite(struct sentest_state *state) {
  sentest_group(state, "bitvecs") {
    sentest(state, "can be allocated and freed") {
      struct bitvec bs = bitvec_new(10);
      bitvec_free(&bs);
    }
    sentest_group(state, "allocates the right amount of cells for") {
      sentest(state, "a multiple of BITVECTOR_CELL_BITS") {
        for (size_t i = 2; i < 50; i++) {
          size_t length = BITVECTOR_CELL_BITS * i;
          struct bitvec bs = bitvec_new(length);
          sentest_assert_eq_fmt(state, bs.length, 0, "%zu");
          sentest_assert_eq_fmt(state, bs.capacity, i, "%zu");
          bitvec_free(&bs);
        }
      }
    }

    sentest(state, "pops what is pushed (hardcoded)") {
      static const bool hardcoded_bools[] = {true, false, true, true, true, false, false};
      const size_t bool_amount = STATIC_LEN(hardcoded_bools);
      struct bitvec bs = bitvec_new(0);
      for (size_t i = 0; i < bool_amount; i++) {
        bitvec_push(&bs, hardcoded_bools[i]);
      }
      for (size_t i = 0; i < bool_amount; i++) {
        bool got = bitvec_pop(&bs);
        bool expected = hardcoded_bools[bool_amount - 1 - i];
        sentest_assert_eq_fmt(state, got, expected, "%d");
      }
      bitvec_free(&bs);
    }

    sentest(state, "pops what is pushed (random)") {
      struct bitvec bs = bitvec_new(0);
      BITVECTOR_CELL cells[100];
      for (size_t i = 0; i < 100; i++) {
        cells[i] = rand();
      }
      const size_t bool_amount = 100 * BITVECTOR_CELL_BITS;
      for (size_t i = 0; i < bool_amount; i++) {
        bitvec_push(&bs, BITTEST(cells, i));
      }
      for (size_t i = 0; i < bool_amount; i++) {
        bool got = bitvec_pop(&bs);
        bool expected = BITTEST(cells, bool_amount - 1 - i);
        sentest_assert_eq_fmt(state, got, expected, "%d");
      }
      bitvec_free(&bs);
    }
  }
}
