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

#ifndef SENARENA_DEFAULT_CHUNK_SIZE
# define SENARENA_DEFAULT_CHUNK_SIZE (4 * 1024 - sizeof(struct senarena_chunk_header))
#endif

#define SENARENA_MIN(a, b) ((a) <= (b) ? (a) : (b))

#define SENARENA_SIMPLE_ALIGNOF(t) (sizeof(t) <= 1 ? 1 : offsetof(struct { char c; t x; }, x))

// presumes alignment is a power of 2
#define SENARENA_ALIGN_DOWN(addr, alignment) ((addr) & -((uintptr_t) alignment))
#define SENARENA_ALIGNOF(t) SENARENA_MIN(sizeof(t), SENARENA_SIMPLE_ALIGNOF(t))

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

#if defined(SENARENA_NOINLINE) && !defined(SENARENA_IMPL)
void *senarena_alloc(struct senarena *arena, size_t byte_amount, size_t alignment);
#endif
void senarena_clear(struct senarena *arena);
void senarena_free(struct senarena arena);
void *senarena_alloc_more(struct senarena *arena, size_t amount, size_t alignment);

#define senarena_alloc_type(arena, type) senarena_alloc((arena), sizeof(type), SENARENA_ALIGNOF(type))
#define senarena_alloc_array_of(arena, type, amount) senarena_alloc(arena, sizeof(type) * amount, SENARENA_ALIGNOF(type))

#if defined(SENARENA_IMPL) || !defined(SENARENA_NOINLINE)

#include <assert.h>
#include <stdbool.h>

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
  return ptr & (alignment - 1);
}

#ifndef SENARENA_IMPL
static
#endif
void *senarena_alloc(struct senarena *arena, size_t amount, size_t alignment) {
  assert(alignment > 0);

#ifndef NDEBUG
  {
    const bool alignmentPowerOfTwo = !(alignment == 0) && !(alignment & (alignment - 1));
    assert(alignmentPowerOfTwo);
  }
#endif

  const size_t amount_and_padding = amount + senarena_extra_bytes_needed((uintptr_t) arena->top - amount, alignment);
  // the invariant that top > bottom is maintained by us
  const size_t free_space = arena->top - arena->bottom;
  if (amount_and_padding > free_space) {
    return senarena_alloc_more(arena, amount, alignment);
  }
  arena->top -= amount_and_padding;
  return arena->top;
}

#endif
#endif
