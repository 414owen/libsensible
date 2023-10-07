// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "sensible-arena.h"

// This basically acts like a zipper, where `new` chunks are
// reusable, and `old` chunks are full
//
// When chunk is in use:
// * Pointer refers to previously used chunk (at start of chunk_header)
// * Capacity contains the current size
//
// When chunk is reusable:
// * Pointer refers to next reusable chunk (at start of chunk_header)
// * Capacity contains the size of the chunk (this doesn't include the chunk_header)
//
// When chunk is current:
// * Pointer refers to previously used chunk (at start f chunk_header)

/*
 *   NULL   -------------
 *     ^    | bytes     |
 *     |    |-----------|
 *     \--- | header    |
 *   ptr    ------------- <--\
 *                           |
 *          -------------    | ptr
 *          | bytes     |    |
 *          |-----------|    |
 *          | header    | ---/
 *     /--> -------------
 *     |
 * ptr |    -------------
 *     |    | bytes     |
 *     |    |-----------| <---\
 *     \--- | header:   |     |
 *          -------------     |
 *                            | current
 *  ---------------------     |
 *  |                   |     |
 *  |                   | ----/
 *  |  arena            |
 *  |                   | ----\
 *  |                   |     |
 *  ---------------------     |
 *                            | fresh_chunks
 *  -------  ptr  -------     |
 *  |     | <---- |     | <---/
 *  -------       -------
 *    |
 *    \--> NULL
 *
 */

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

static
size_t senarena_extra_fresh_bytes_needed(uintptr_t amount, uintptr_t alignment) {
  size_t res = senarena_extra_bytes_needed(sizeof(struct senarena_chunk_header), alignment);
  if (amount + res < SENARENA_MIN_CHUNK_SIZE) return 0;
  return res;
}

// Returns a pointer to *after* the chunk_header
static
unsigned char *senarena_chunk_new(uintptr_t size, struct senarena_chunk_header *ptr) {
  // with size 7 and alignment 8 you'll need 1 more byte if you align up or down
  struct senarena_chunk_header *chunk = (struct senarena_chunk_header*) malloc(size + sizeof(struct senarena_chunk_header));
  chunk->ptr = ptr;
  chunk->capacity = size;
  return (unsigned char*) chunk + sizeof(struct senarena_chunk_header);
}

struct senarena senarena_new() {
  struct senarena res = {
    .size = SENARENA_MIN_CHUNK_SIZE,
    .current = senarena_chunk_new(SENARENA_MIN_CHUNK_SIZE, NULL),
    .fresh_chunks = NULL,
  };
  return res;
}

// O(min(n, m)) where n, m are the sizes of the linked chunk lists a and b
static
struct senarena_chunk_header *senarena_join_chunk_chains(struct senarena_chunk_header *a, struct senarena_chunk_header *b) {

  if (a == NULL) return b;
  if (b == NULL) return a;

  struct senarena_chunk_header *curr_a = a;
  struct senarena_chunk_header *curr_b = b;

  struct senarena_chunk_header *next_a = a->ptr;
  struct senarena_chunk_header *next_b = b->ptr;

  while (next_a != NULL && next_b != NULL) {
    curr_a = next_a;
    curr_b = next_b;
    next_a = curr_a->ptr;
    next_b = curr_b->ptr;
  }

  if (next_a == NULL) {
    curr_a->ptr = b;
    return a;
  } else {
    curr_b->ptr = a;
    return b;
  }
}

void senarena_clear(struct senarena *arena) {
  struct senarena_chunk_header *current = (struct senarena_chunk_header*) (arena->current - sizeof(struct senarena_chunk_header));
  arena->size = current->capacity;
  arena->fresh_chunks = senarena_join_chunk_chains(arena->fresh_chunks, current->ptr);
  current->ptr = NULL;
}

void *senarena_alloc(struct senarena *arena, size_t amount, size_t alignment) {
  assert(alignment > 0);
  size_t amount_and_padding = amount + senarena_extra_bytes_needed((uintptr_t) arena->current + amount, alignment);
  if (amount_and_padding > arena->size) {
    struct senarena_chunk_header *current_header = (struct senarena_chunk_header*) (arena->current - sizeof(struct senarena_chunk_header));
    // extra bytes needed on a fresh chunk
    size_t extra_fresh_bytes = senarena_extra_fresh_bytes_needed(amount, alignment);
    if (amount + extra_fresh_bytes >= SENARENA_MIN_CHUNK_SIZE >> 2) {
      // Makes it possible to allocate large objects here.
      // Really, you just shouldn't...
      unsigned char *res = senarena_chunk_new(amount + extra_fresh_bytes, current_header->ptr);
      current_header->ptr = (struct senarena_chunk_header*) (res - sizeof(struct senarena_chunk_header));
      return res + extra_fresh_bytes;
    } else if (arena->fresh_chunks != NULL) {
      struct senarena_chunk_header *next = arena->fresh_chunks;
      arena->fresh_chunks = next->ptr;
      next->ptr = (struct senarena_chunk_header*) (arena->current - sizeof(struct senarena_chunk_header));
      arena->size = next->capacity;
      arena->current = (unsigned char*) next + sizeof(struct senarena_chunk_header);
    } else {
      arena->current = senarena_chunk_new(SENARENA_MIN_CHUNK_SIZE, current_header);
      arena->size = SENARENA_MIN_CHUNK_SIZE;
    }
  }
  arena->size -= amount_and_padding;
  return arena->current + arena->size;
}

static
void senarena_free_chunk_chain(struct senarena_chunk_header *current) {
  while (current) {
    struct senarena_chunk_header *previous = current->ptr;
    free(current);
    current = previous;
  }
}

void senarena_free(struct senarena arena) {
  struct senarena_chunk_header *current = (struct senarena_chunk_header *) (arena.current - sizeof(struct senarena_chunk_header));
  senarena_free_chunk_chain(current);
  senarena_free_chunk_chain(arena.fresh_chunks);
}