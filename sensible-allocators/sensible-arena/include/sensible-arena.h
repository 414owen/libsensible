// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SENSIBLE_ARENA_H
#define SENSIBLE_ARENA_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#include "../../../sensible-macros/include/sensible-macros.h"

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

#define senarena_malloc
#define senarena_always_inline inline

#if defined(__has_attribute)
# if __has_attribute(malloc)
#  undef senarena_malloc
#  define senarena_malloc __attribute__((__malloc__))
# endif
# if __has_attribute(always_inline)
#  undef senarena_always_inline
#  define senarena_always_inline inline __attribute__((__always_inline__))
# endif
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
  uintptr_t top;
  // pointer to the current chunk (after the header)
  uintptr_t bottom;
  // pointer to the next reusable chunk
  struct senarena_chunk_header *fresh_chunks;
};


senmac_public struct senarena senarena_new();

#if defined(SENARENA_NOINLINE) && !defined(SENARENA_IMPL)
senmac_public void *senarena_alloc(struct senarena *restrict arena, size_t byte_amount, size_t alignment) senarena_malloc;
#endif
senmac_public void senarena_clear(struct senarena *restrict arena);
senmac_public void senarena_free(struct senarena arena);
senmac_public uintptr_t senarena_alloc_more(struct senarena *restrict arena, size_t amount, size_t alignment);

#define senarena_alloc_type(arena, type) senarena_alloc((arena), sizeof(type), SENARENA_ALIGNOF(type))
#define senarena_alloc_array_of(arena, type, amount) senarena_alloc(arena, sizeof(type) * amount, SENARENA_ALIGNOF(type))

#if defined(SENARENA_IMPL) || !defined(SENARENA_NOINLINE)

#include <assert.h>
#include <stdbool.h>

static senarena_always_inline
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
extern senarena_always_inline
#endif
senarena_malloc
void *senarena_alloc(struct senarena *restrict arena, size_t amount, size_t alignment) {
  assert(alignment > 0);
  {
    const bool alignmentPowerOfTwo = !(alignment == 0) && !(alignment & (alignment - 1));
    (void) alignmentPowerOfTwo;
    assert(alignmentPowerOfTwo);
  }
  uintptr_t top = arena->top;
  const size_t amount_and_padding = amount + senarena_extra_bytes_needed((uintptr_t) top - amount, alignment);

  // the invariant that top > bottom is maintained by us
  const size_t free_space = arena->top - arena->bottom;
  if senarena_unlikely(amount_and_padding > free_space) {
    return (void*) senarena_alloc_more(arena, amount, alignment);
  }
  arena->top -= amount_and_padding;
  return (void*) arena->top;
}

#endif // defined(SENARENA_IMPL) || !defined(SENARENA_NOINLINE)

#ifdef __cplusplus
}
#endif

#endif // ifndef SENSIBLE_ARENA_H
