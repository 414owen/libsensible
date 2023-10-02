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

struct bitvec {
  BITVECTOR_CELL *restrict data;
  // in bits
  size_t length;
  // in bytes
  size_t capacity;
};

struct bitvec bitvec_new(size_t capacity_bits);

bool bitvec_get(struct bitvec bs, size_t n);

void bitvec_set_true(struct bitvec bs, size_t n);
void bitvec_set_false(struct bitvec bs, size_t n);
void bitvec_set(struct bitvec bs, bool value, size_t n);

void bitvec_push_true(struct bitvec *bs);
void bitvec_push_false(struct bitvec *bs);
void bitvec_push(struct bitvec *bs, bool value);

bool bitvec_pop(struct bitvec *bs);
void bitvec_free(struct bitvec *bs);
