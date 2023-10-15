// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ARENA_H
#define ARENA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

// Allocator optimized for small allocations.
// Frees everything at once.
// Can reuse space.

#if defined(__GNUC__) || defined(__clang__)
#define senarena_unlikely(x)     (__builtin_expect(!!(x),false))
#define senarena_likely(x)       (__builtin_expect(!!(x),true))
#elif (defined(__cplusplus) && (__cplusplus >= 202002L)) || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
#define senarena_unlikely(x)     (x) [[unlikely]]
#define senarena_likely(x)       (x) [[likely]]
#else
#define senarena_unlikely(x)     (x)
#define senarena_likely(x)       (x)
#endif

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

#ifdef SENARENA_IMPL
 static
#else
 extern
#endif
inline
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
extern inline
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
  if senarena_unlikely(amount_and_padding > free_space) {
    return senarena_alloc_more(arena, amount, alignment);
  }
  arena->top -= amount_and_padding;
  return arena->top;
}

#endif
#endif

#ifdef __cplusplus
}
#endif
