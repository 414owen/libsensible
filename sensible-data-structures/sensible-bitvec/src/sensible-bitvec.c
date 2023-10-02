// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "sensible-bitvec.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static
void bitvec_reserve_one(struct bitvec *bv) {
  const size_t required_slots = BITNSLOTS(bv->length + 1);
  if (required_slots > bv->capacity) {
    bv->capacity += bv->capacity >> 1;
    bv->data = realloc(bv->data, bv->capacity);
  }
}

struct bitvec bitvec_new(size_t capacity_bits) {
  size_t num_slots = BITNSLOTS(capacity_bits);
  num_slots = MAX(num_slots, 2);
  struct bitvec res = {
    .data = malloc(sizeof(BITVECTOR_CELL) * num_slots),
    .length = 0,
    .capacity = num_slots,
  };
  return res;
}

bool bitvec_get(struct bitvec bv, size_t n) {
  assert(n < bv.length);
  return BITTEST(bv.data, n);
}

void bitvec_set_true(struct bitvec bv, size_t n) {
  assert(n < bv.length);
  BITSET(bv.data, n);
}

void bitvec_set_false(struct bitvec bv, size_t n) {
  assert(n < bv.length);
  BITCLEAR(bv.data, n);
}

void bitvec_set(struct bitvec bv, bool value, size_t n) {
  assert(n < bv.length);
  if (value) {
    bitvec_set_true(bv, n);
  } else {
    bitvec_set_false(bv, n);
  }
}

void bitvec_push_true(struct bitvec *bv) {
  bitvec_reserve_one(bv);
  BITSET(bv->data, bv->length);
  bv->length++;
}

void bitvec_push_false(struct bitvec *bv) {
  bitvec_reserve_one(bv);
  BITCLEAR(bv->data, bv->length);
  bv->length++;
}

void bitvec_push(struct bitvec *bv, bool value) {
  if (value) {
    bitvec_push_true(bv);
  } else {
    bitvec_push_false(bv);
  }
}

bool bitvec_pop(struct bitvec *bv) {
  assert(bv->length > 0);
  bv->length--;
  return BITTEST(bv->data, bv->length);
}

void bitvec_free(struct bitvec *bv) {
#ifndef NDEBUG
  bv->capacity = 0;
  bv->length = 0;
#endif
  free(bv->data);
}
