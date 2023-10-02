// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SENSIBLE_BITVEC_H
#define SENSIBLE_BITVEC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>

#define SENSIBLE_BITVECTOR_CELL unsigned char
#define SENSIBLE_BITVECTOR_CELL_BITS CHAR_BIT

#ifndef SENSIBLE_BITSET_H

// Assuming a uint8_t *
#define SENSIBLE_BITMASK(b) (1 << ((b) % SENSIBLE_BITVECTOR_CELL_BITS))
#define SENSIBLE_BITSLOT(b) ((b) / SENSIBLE_BITVECTOR_CELL_BITS)
#define SENSIBLE_BITSET(a, b) ((a)[SENSIBLE_BITSLOT(b)] |= SENSIBLE_BITMASK(b))
#define SENSIBLE_BITCLEAR(a, b) ((a)[SENSIBLE_BITSLOT(b)] &= ~SENSIBLE_BITMASK(b))
#define SENSIBLE_BITTEST(a, b) ((a)[SENSIBLE_BITSLOT(b)] & SENSIBLE_BITMASK(b))
#define SENSIBLE_BITNSLOTS(nb) ((nb + SENSIBLE_BITVECTOR_CELL_BITS - 1) / SENSIBLE_BITVECTOR_CELL_BITS)

#endif

struct senbitvec {
  SENSIBLE_BITVECTOR_CELL *restrict data;
  // in bits
  size_t length;
  // in bytes
  size_t capacity;
};

struct senbitvec senbitvec_new(size_t capacity_bits);

bool senbitvec_get(struct senbitvec bv, size_t n);

void senbitvec_set_true(struct senbitvec bv, size_t n);
void senbitvec_set_false(struct senbitvec bv, size_t n);
void senbitvec_set(struct senbitvec bv, bool value, size_t n);

void senbitvec_push_true(struct senbitvec *bv);
void senbitvec_push_false(struct senbitvec *bv);
void senbitvec_push(struct senbitvec *bv, bool value);

bool senbitvec_pop(struct senbitvec *bv);
void senbitvec_free(struct senbitvec *bv);

#ifdef __cplusplus
}
#endif

#endif
