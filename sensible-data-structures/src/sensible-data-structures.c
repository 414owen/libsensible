// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "sensible-data-structures.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static
void bitvec_reserve_one(struct bitvec *bs) {
  const size_t required_slots = BITNSLOTS(bs->length + 1);
  if (required_slots > bs->capacity) {
    bs->capacity += bs->capacity >> 1;
    bs->data = realloc(bs->data, bs->capacity);
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

bool bitvec_get(struct bitvec bs, size_t n) {
  assert(n < bs.length);
  return BITTEST(bs.data, n);
}

void bitvec_set_true(struct bitvec bs, size_t n) {
  assert(n < bs.length);
  BITSET(bs.data, n);
}

void bitvec_set_false(struct bitvec bs, size_t n) {
  assert(n < bs.length);
  BITCLEAR(bs.data, n);
}

void bitvec_set(struct bitvec bs, bool value, size_t n) {
  assert(n < bs.length);
  if (value) {
    bitvec_set_true(bs, n);
  } else {
    bitvec_set_false(bs, n);
  }
}

void bitvec_push_true(struct bitvec *bs) {
  bitvec_reserve_one(bs);
  BITSET(bs->data, bs->length);
  bs->length++;
}

void bitvec_push_false(struct bitvec *bs) {
  bitvec_reserve_one(bs);
  BITCLEAR(bs->data, bs->length);
  bs->length++;
}

void bitvec_push(struct bitvec *bs, bool value) {
  if (value) {
    bitvec_push_true(bs);
  } else {
    bitvec_push_false(bs);
  }
}

bool bitvec_pop(struct bitvec *bs) {
  assert(bs->length > 0);
  bs->length--;
  return BITTEST(bs->data, bs->length);
}

void bitvec_free(struct bitvec *bs) {
#ifndef NDEBUG
  bs->capacity = 0;
  bs->length = 0;
#endif
  free(bs->data);
}
