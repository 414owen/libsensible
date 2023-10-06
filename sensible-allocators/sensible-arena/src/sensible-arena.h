// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

// Allocator optimized for small allocations.
// Frees everything at once.
// Can reuse space.

#define SENARENA_MIN(a, b) ((a) <= (b) ? (a) : (b))

#define SENARENA_SIMPLE_ALIGNOF(t) (sizeof(t) <= 1 ? 1 : offsetof(struct { char c; t x; }, x))

// presumes alignment is a power of 2
#define SENARENA_ALIGN_DOWN(addr, alignment) ((addr) & -((uintptr_t) alignment))
#define SENARENA_ALIGNOF(t) SENARENA_MIN(sizeof(t), SENARENA_SIMPLE_ALIGNOF(t))

// This has to be a power of two, so that our
// alignment calculations work. See 'extra_fresh_bytes'.
#define SENARENA_MIN_CHUNK_SIZE (4 * 1024 - sizeof(struct senarena_chunk_header))

struct senarena_chunk_header {
  struct senarena_chunk_header *ptr;
  uintptr_t capacity;
};

struct senarena {
  // capacity of the current chunk,
  size_t size;
  // pointer to the next reusable chunk
  struct senarena_chunk_header *fresh_chunks;
  // pointer to the current chunk (after the header)
  unsigned char *current;
};

struct senarena senarena_new();
void *senarena_alloc(struct senarena *arena, size_t byte_amount, size_t alignment);
void senarena_clear(struct senarena *arena);
void senarena_free(struct senarena arena);

#define senarena_alloc_type(arena, type) senarena_alloc((arena), sizeof(type), SENARENA_ALIGNOF(type))
#define senarena_alloc_array_of(arena, type, amount) senarena_alloc(arena, sizeof(type) * amount, SENARENA_ALIGNOF(type))

#endif
