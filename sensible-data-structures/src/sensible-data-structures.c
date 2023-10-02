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
void bitset_reserve_one(struct bitset *bs) {
  const size_t required_slots = BITNSLOTS(bs->length + 1);
  if (required_slots > bs->capacity) {
    bs->capacity += bs->capacity >> 1;
    bs->data = realloc(bs->data, bs->capacity);
  }
}

struct bitset bitset_new(size_t capacity_bits) {
  size_t num_slots = BITNSLOTS(capacity_bits);
  num_slots = MAX(num_slots, 2);
  struct bitset res = {
    .data = malloc(sizeof(BITVECTOR_CELL) * num_slots),
    .length = 0,
    .capacity = num_slots,
  };
  return res;
}

bool bitset_get(struct bitset bs, size_t n) {
  assert(n < bs.length);
  return BITTEST(bs.data, n);
}

void bitset_set_true(struct bitset bs, size_t n) {
  assert(n < bs.length);
  BITSET(bs.data, n);
}

void bitset_set_false(struct bitset bs, size_t n) {
  assert(n < bs.length);
  BITCLEAR(bs.data, n);
}

void bitset_set(struct bitset bs, bool value, size_t n) {
  assert(n < bs.length);
  if (value) {
    bitset_set_true(bs, n);
  } else {
    bitset_set_false(bs, n);
  }
}

void bitset_push_true(struct bitset *bs) {
  bitset_reserve_one(bs);
  BITSET(bs->data, bs->length);
  bs->length++;
}

void bitset_push_false(struct bitset *bs) {
  bitset_reserve_one(bs);
  BITCLEAR(bs->data, bs->length);
  bs->length++;
}

void bitset_push(struct bitset *bs, bool value) {
  if (value) {
    bitset_push_true(bs);
  } else {
    bitset_push_false(bs);
  }
}

bool bitset_pop(struct bitset *bs) {
  assert(bs->length > 0);
  bs->length--;
  return BITTEST(bs->data, bs->length);
}

void bitset_free(struct bitset *bs) {
#ifndef NDEBUG
  bs->capacity = 0;
  bs->length = 0;
#endif
  free(bs->data);
}
