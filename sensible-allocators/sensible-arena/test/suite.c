// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../src/sensible-arena.h"
#include "../../../sensible-test/src/sensible-test.h"

#define STATIC_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

int one_in(int n) {
  return rand() % n == 0;
}

void run_sensible_arena_suite(struct sentest_state *state) {
  sentest_group(state, "arena") {
    sentest(state, "can be constructed and freed") {
      struct senarena arena = senarena_new();
      senarena_free(arena);
    }
    sentest(state, "can allocate an int") {
      struct senarena arena = senarena_new();
      volatile int *a = senarena_alloc_type(&arena, int);
      *a = 42;
      sentest_assert_eq(state, *a, 42);
      senarena_free(arena);
    }
    sentest(state, "has a null next chunk by default") {
      struct senarena arena = senarena_new();
      sentest_assert_eq(state, arena.fresh_chunks, NULL);
      senarena_free(arena);
    }
    sentest(state, "can allocate more than a chunk's worth of data at once") {
      struct senarena arena = senarena_new();
      const size_t size = 1024 * 1024;
      // 1MiB
      volatile unsigned char *area = senarena_alloc(&arena, size, 1);
      for (size_t i = 0; i < size; i++) {
        area[i] = 1;
      }
      senarena_free(arena);
    }
    sentest(state, "can allocate more than a chunk's worth of data in steps") {
      struct senarena arena = senarena_new();
      for (int i = 0; i < 10000; i++) {
        size_t bytes = 100;
        volatile unsigned char *area = senarena_alloc(&arena, bytes, 1);
        for (size_t j = 0; j < bytes; j++) {
          area[j] = 50;
        }
      }
      senarena_free(arena);
    }
    sentest_group(state, "when allocating large chunks") {
      sentest(state, "leaves the current chunk available") {
        static const size_t sizes[] = {SENARENA_MIN_CHUNK_SIZE, 1 + (SENARENA_MIN_CHUNK_SIZE >> 2)};
        for (size_t i = 0; i < STATIC_LEN(sizes); i++) {
          const size_t allocation_size = sizes[i];
          struct senarena arena = senarena_new();
          const size_t capacity = arena.size;
          const size_t first_alloc_size = SENARENA_MIN_CHUNK_SIZE - 1000;
          unsigned char *start_buffer = arena.current;
          {
            volatile unsigned char *area = senarena_alloc(&arena, first_alloc_size, 1);
            area[50] = 42;
          }
          sentest_assert_eq_fmt(state, "zu", arena.size, capacity - first_alloc_size);
          {
            volatile unsigned char *area = senarena_alloc(&arena, allocation_size, 1);
            area[50] = 50;
          }
          sentest_assert_eq_fmt(state, "zu", arena.size, capacity - first_alloc_size);
          sentest_assert_eq_fmt(state, "p", arena.current, start_buffer);
          senarena_free(arena);
        }
      }
    }
    sentest_group(state, "when clearing an arena") {
      sentest(state, "reuses the chunk chain") {
        struct senarena arena = senarena_new();
        sentest_assert_eq(state, arena.fresh_chunks, NULL);
        unsigned char *chunk_data = arena.current;
        const size_t size = 1024 * 1024;
        // 1MiB
        volatile unsigned char *area = senarena_alloc(&arena, size, 1);
        area[50] = 42;
        sentest_assert_eq_fmt(state, "zu", arena.size, SENARENA_MIN_CHUNK_SIZE);
        sentest_assert_eq(state, arena.current, chunk_data);
        sentest_assert_eq(state, arena.fresh_chunks, NULL);
        senarena_clear(&arena);
        sentest_assert_eq(state, arena.current, chunk_data);
        sentest_assert_eq_fmt(state, "p", (unsigned char*) arena.fresh_chunks + sizeof(struct senarena_chunk_header), area);
        senarena_free(arena);
      }
    }
    sentest_group(state, "fuzz tests") {
      const int n = 1;

      unsigned char *numbers = malloc(n * sizeof(unsigned char));
      size_t *sizes = calloc(n, sizeof(size_t));
      int *alignments = calloc(n, sizeof(int));
      volatile unsigned char **pointers = calloc(n, sizeof(int*));

      struct senarena arena = senarena_new();
      sentest(state, "can write allocated memory") {
        for (int i = 0; i < n; i++) {
          const size_t mod = one_in(10) ? SENARENA_MIN_CHUNK_SIZE * 3 : 257;
          const size_t size = rand() % mod;
          sizes[i] = size;

          const size_t alignment = pow(2, rand() % 7); // max 64
          alignments[i] = alignment;

          volatile unsigned char *buf = senarena_alloc(&arena, size, alignment);
          pointers[i] = buf;

          // printf("size: %zu, align: %zu\n", size, alignment);
          const unsigned char number = rand();
          numbers[i] = number;
          memset((void*) buf, number, size);
        }
      }
      sentest(state, "memory is correctly aligned") {
        for (int i = 0; i < n; i++) {
          volatile unsigned char *pointer = pointers[i];
          const size_t size = sizes[i];
          const int alignment = alignments[i];
          if (size > 0) {
            const uintptr_t mod = (uintptr_t) pointer % alignment;
            if (mod != 0) {
              sentest_failf(state, "ptr %p of size %zu isn't aligned to %d boundary. Off by %p", pointer, size, alignment, mod);
            }
          }
        }
      }
      sentest(state, "can read allocated memory") {
        for (int i = 0; i < n; i++) {
          const unsigned char number = numbers[i];
          const size_t size = sizes[i];
          volatile unsigned char *pointer = pointers[i];
          bool equal = true;
          size_t j;
          for (j = 0; j < size; j++) {
            // printf("%zu\n", j);
            if (pointer[j] != number) {
              equal = false;
              break;
            }
          }
          if (!equal) {
            sentest_failf(state, "expected byte %zu to be '0x%x', but found '0x%x'", j, number, pointer[j]);
          }
        }
      }
      free(numbers);
      free(sizes);
      free(alignments);
      free(pointers);
      senarena_free(arena);
    }
  }
}
