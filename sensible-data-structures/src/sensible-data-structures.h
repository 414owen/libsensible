// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>

#define BITVECTOR_CELL unsigned char
#define BITVECTOR_CELL_BITS CHAR_BIT

// Assuming a uint8_t *
#define BITMASK(b) (1 << ((b) % BITVECTOR_CELL_BITS))
#define BITSLOT(b) ((b) / BITVECTOR_CELL_BITS)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + BITVECTOR_CELL_BITS - 1) / BITVECTOR_CELL_BITS)

struct bitset {
  BITVECTOR_CELL *restrict data;
  // in bits
  size_t length;
  // in bytes
  size_t capacity;
};

struct bitset bitset_new(size_t capacity_bits);

bool bitset_get(struct bitset bs, size_t n);

void bitset_set_true(struct bitset bs, size_t n);
void bitset_set_false(struct bitset bs, size_t n);
void bitset_set(struct bitset bs, bool value, size_t n);

void bitset_push_true(struct bitset *bs);
void bitset_push_false(struct bitset *bs);
void bitset_push(struct bitset *bs, bool value);

bool bitset_pop(struct bitset *bs);
void bitset_free(struct bitset *bs);
