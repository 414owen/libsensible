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
void senbitvec_reserve_one(struct senbitvec *bv) {
  const size_t required_slots = SENSIBLE_BITNSLOTS(bv->length + 1);
  if (required_slots > bv->capacity) {
    bv->capacity += bv->capacity >> 1;
    bv->data = realloc(bv->data, bv->capacity);
  }
}

struct senbitvec senbitvec_new(size_t capacity_bits) {
  size_t num_slots = SENSIBLE_BITNSLOTS(capacity_bits);
  num_slots = MAX(num_slots, 2);
  struct senbitvec res = {
    .data = malloc(sizeof(SENSIBLE_BITVECTOR_CELL) * num_slots),
    .length = 0,
    .capacity = num_slots,
  };
  return res;
}

bool senbitvec_get(struct senbitvec bv, size_t n) {
  assert(n < bv.length);
  return SENSIBLE_BITTEST(bv.data, n);
}

void senbitvec_set_true(struct senbitvec bv, size_t n) {
  assert(n < bv.length);
  SENSIBLE_BITSET(bv.data, n);
}

void senbitvec_set_false(struct senbitvec bv, size_t n) {
  assert(n < bv.length);
  SENSIBLE_BITCLEAR(bv.data, n);
}

void senbitvec_set(struct senbitvec bv, bool value, size_t n) {
  assert(n < bv.length);
  if (value) {
    senbitvec_set_true(bv, n);
  } else {
    senbitvec_set_false(bv, n);
  }
}

void senbitvec_push_true(struct senbitvec *bv) {
  senbitvec_reserve_one(bv);
  SENSIBLE_BITSET(bv->data, bv->length);
  bv->length++;
}

void senbitvec_push_false(struct senbitvec *bv) {
  senbitvec_reserve_one(bv);
  SENSIBLE_BITCLEAR(bv->data, bv->length);
  bv->length++;
}

void senbitvec_push(struct senbitvec *bv, bool value) {
  if (value) {
    senbitvec_push_true(bv);
  } else {
    senbitvec_push_false(bv);
  }
}

bool senbitvec_pop(struct senbitvec *bv) {
  assert(bv->length > 0);
  bv->length--;
  return SENSIBLE_BITTEST(bv->data, bv->length);
}

void senbitvec_free(struct senbitvec *bv) {
#ifndef NDEBUG
  bv->capacity = 0;
  bv->length = 0;
#endif
  free(bv->data);
}
