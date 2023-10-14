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
  // pointer to the first unfree byte
  unsigned char *top;
  // pointer to the current chunk (after the header)
  unsigned char *bottom;
  // pointer to the next reusable chunk
  struct senarena_chunk_header *fresh_chunks;
};

struct senarena senarena_new();

#if !defined(SENARENA_INLINE)
void *senarena_alloc(struct senarena *arena, size_t byte_amount, size_t alignment);
#endif
void senarena_clear(struct senarena *arena);
void senarena_free(struct senarena arena);
void *senarena_alloc_more(struct senarena *arena, size_t amount, size_t alignment);

#define senarena_alloc_type(arena, type) senarena_alloc((arena), sizeof(type), SENARENA_ALIGNOF(type))
#define senarena_alloc_array_of(arena, type, amount) senarena_alloc(arena, sizeof(type) * amount, SENARENA_ALIGNOF(type))

#ifdef SENARENA_INLINE

#include <assert.h>

static
uintptr_t senarena_extra_bytes_needed(uintptr_t ptr, uintptr_t alignment) {
  // ptr           = 11010
  // 8             = 01000
  // 8 - 1         = 00111
  // ptr & (8 - 1) = 00010

  // Also works if alignment > ptr, because 0 is always aligned:

  // ptr           = 00010
  // 8             = 01000
  // 8 - 1         = 00111
  // ptr & (8 - 1) = 00010
  alignment -= 1;
  ptr &= alignment;
  return ptr;
}

static const size_t senarena_chunk_header_size = sizeof(struct senarena_chunk_header);

// this is only called when a chunk is being allocated specifically for one
// allocation.
static
size_t senarena_extra_fresh_bytes_needed(uintptr_t alignment) {
  uintptr_t start = alignment > senarena_chunk_header_size ? alignment - senarena_chunk_header_size : senarena_chunk_header_size;
  return senarena_extra_bytes_needed(start, alignment);
}

#ifndef SENARENA_IMPL
static
#endif
void *senarena_alloc(struct senarena *arena, size_t amount, size_t alignment) {
  assert(alignment > 0);

#ifndef NDEBUG
  bool alignmentPowerOfTwo = !(alignment == 0) && !(alignment & (alignment - 1));
  assert(alignmentPowerOfTwo);
#endif

  intptr_t amount_and_padding = amount + senarena_extra_bytes_needed((intptr_t) arena->top - amount, alignment);
  const intptr_t free_space = arena->top - arena->bottom;
  if (amount_and_padding > free_space) {
    return senarena_alloc_more(arena, amount, alignment);
  }
  arena->top -= amount_and_padding;
  return arena->top;
}

#endif

#endif
