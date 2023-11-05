// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../src/sensible-bitvec.h"
#include "../../../sensible-test/src/sensible-test.h"
#include "../../../sensible-macros/include/sensible-macros.h"

#define STATIC_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

senmac_public
void run_sensible_bitvec_suite(struct sentest_state *state) {
  sentest_group(state, "sensible-bitvec") {
    sentest(state, "can be allocated and freed") {
      struct senbitvec bs = senbitvec_new(10);
      senbitvec_free(&bs);
    }
    sentest_group(state, "allocates the right amount of cells for") {
      sentest(state, "a multiple of BITVECTOR_CELL_BITS") {
        for (size_t i = 2; i < 50; i++) {
          size_t length = SENSIBLE_BITVECTOR_CELL_BITS * i;
          struct senbitvec bs = senbitvec_new(length);
          sentest_assert_eq_fmt(state, "zu", bs.length, 0);
          sentest_assert_eq_fmt(state, "zu", bs.capacity, i);
          senbitvec_free(&bs);
        }
      }
    }

    sentest(state, "pops what is pushed (hardcoded)") {
      static const bool hardcoded_bools[] = {true, false, true, true, true, false, false};
      const size_t bool_amount = STATIC_LEN(hardcoded_bools);
      struct senbitvec bs = senbitvec_new(0);
      for (size_t i = 0; i < bool_amount; i++) {
        senbitvec_push(&bs, hardcoded_bools[i]);
      }
      for (size_t i = 0; i < bool_amount; i++) {
        bool got = senbitvec_pop(&bs);
        bool expected = hardcoded_bools[bool_amount - 1 - i];
        sentest_assert_eq_fmt(state, "d", got, expected);
      }
      senbitvec_free(&bs);
    }

    sentest(state, "pops what is pushed (random)") {
      struct senbitvec bs = senbitvec_new(0);
      SENSIBLE_BITVECTOR_CELL cells[100];
      for (size_t i = 0; i < 100; i++) {
        cells[i] = rand();
      }
      const size_t bool_amount = 100 * SENSIBLE_BITVECTOR_CELL_BITS;
      for (size_t i = 0; i < bool_amount; i++) {
        senbitvec_push(&bs, SENSIBLE_BITTEST(cells, i));
      }
      for (size_t i = 0; i < bool_amount; i++) {
        bool got = senbitvec_pop(&bs);
        bool expected = SENSIBLE_BITTEST(cells, bool_amount - 1 - i);
        sentest_assert_eq_fmt(state, "d", got, expected);
      }
      senbitvec_free(&bs);
    }
  }
}
