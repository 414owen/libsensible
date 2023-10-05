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

#define MIN(a, b) ((a) <= (b) ? (a) : (b))

#define SIMPLE_ALIGNOF(t) (sizeof(t) <= 1 ? 1 : offsetof(struct { char c; t x; }, x))

// presumes alignment is a power of 2
#define ALIGN_DOWN(addr, alignment) ((addr) & -((uintptr_t) alignment))
#define ALIGNOF(t) MIN(sizeof(t), SIMPLE_ALIGNOF(t))

// This has to be a power of two, so that our
// alignment calculations work. See 'extra_fresh_bytes'.
#define ARENA_MIN_CHUNK_SIZE (4 * 1024 - sizeof(struct chunk_header))

struct chunk_header {
  struct chunk_header *ptr;
  uintptr_t capacity;
};

struct arena {
  // capacity of the current chunk,
  size_t size;
  // pointer to the next reusable chunk
  struct chunk_header *fresh_chunks;
  // pointer to the current chunk (after the header)
  unsigned char *current;
};

struct arena arena_new();
void *arena_alloc(struct arena *arena, size_t byte_amount, size_t alignment);
void arena_clear(struct arena *arena);
void arena_free(struct arena arena);

#define arena_alloc_type(arena, type) arena_alloc((arena), sizeof(type), ALIGNOF(type))
#define arena_alloc_array_of(arena, type, amount) arena_alloc(arena, sizeof(type) * amount, ALIGNOF(type))

#endif
