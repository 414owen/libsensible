// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#define _XOPEN_SOURCE 500

#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "../../../sensible-test/src/sensible-test.h"
#include "../../../sensible-timing/src/sensible-timing.h"
#include "../src/sensible-arena.h"

static const double time_threshold_nanos = 5e8; // rounds should take at least 0.2s
static const int num_iters = 10;
static const int ints_to_allocate = 2;

#ifdef WIN32
bool stdout_is_tty(void) {
  return _isatty(_fileno(stdout));
}
#else
bool stdout_is_tty(void) {
  return isatty(fileno(stdout));
}
#endif

struct minmax_d {
  uint64_t min;
  uint64_t max;
};

static
void minmax_d(struct minmax_d *out, uint64_t nanos) {
  if (nanos < out->min)
    out->min = nanos;
  if (nanos > out->max)
    out->max = nanos;
}

static
double ns_to_s(uint64_t ns) {
  // three decimal places
  return (double) ns / 1e9;
}

// Find the number of allocations that makes sense per benchmark iteration
static
unsigned long determine_arena_alloc_amt(void) {
  printf("Finding out how many arena allocations will take %.3fs\n", ns_to_s(time_threshold_nanos));

  unsigned long num_allocations = 2 << 10; // 1024
  uint64_t nanos = 0;
  while (true) {
    if (stdout_is_tty()) {
      printf("\rincreasing allocations: %lu", num_allocations);
      fflush(stdout);
    }
    struct senarena a1 = senarena_new();
    const struct seninstant begin = seninstant_now();
    for (unsigned long i = 0; i < num_allocations; i++) {
      volatile int *ints = senarena_alloc_array_of(&a1, int, ints_to_allocate);
      ints[1] = 0;
    }
    nanos = seninstant_subtract(seninstant_now(), begin);
    senarena_free(a1);
    if (nanos >= time_threshold_nanos) break;
    num_allocations *= 2;
  }

  if (stdout_is_tty()) {
    putchar('\n');
  }

  const int reduction_steps = 5;
  for (int j = 0; j < reduction_steps; j++) {
    while (true) {
      const unsigned long m = num_allocations - num_allocations / (1 << (j + 1));
      if (stdout_is_tty()) {
        printf("\rreducing %d/%d %lu", j + 1, reduction_steps, m);
        fflush(stdout);
      }
      struct senarena a1 = senarena_new();
      const struct seninstant begin = seninstant_now();
      for (unsigned long i = 0; i < m; i++) {
        volatile int *ints = senarena_alloc_array_of(&a1, int, ints_to_allocate);
        ints[1] = 0;
      }
      const uint64_t red_nanos = seninstant_subtract(seninstant_now(), begin);
      senarena_free(a1);
      if (red_nanos < time_threshold_nanos) {
        break;
      }
      num_allocations = m;
      nanos = red_nanos;
    }
  }
  if (stdout_is_tty()) {
    putchar('\n');
  }
  printf("Doing %lu arena_alloc()s per round.\n"
    "This number has been determined empirically for a round time >= %.3fs.\n",
    num_allocations, ns_to_s(nanos));
  return num_allocations;
}

// Find the number of allocations that makes sense per benchmark iteration
static
unsigned long determine_standard_alloc_amt(void) {
  printf("Finding out how many standard allocations will take %.3fs\n", ns_to_s(time_threshold_nanos));

  unsigned long num_allocations = 2 << 10; // 1024
  volatile int **ptrs = malloc(sizeof(int*) * num_allocations);
  uint64_t nanos = 0;
  while (true) {
    if (stdout_is_tty()) {
      printf("\rincreasing allocations: %lu", num_allocations);
      fflush(stdout);
    }
    const struct seninstant begin = seninstant_now();
    for (unsigned long i = 0; i < num_allocations; i++) {
      volatile int *ints = malloc(sizeof(int) * ints_to_allocate);
      ptrs[i] = ints;
      ints[1] = 0;
    }
    const struct seninstant end = seninstant_now();
    for (unsigned long i = 0; i < num_allocations; i++) {
      free((void*) ptrs[i]);
    }
    nanos = seninstant_subtract(end, begin);
    if (nanos >= time_threshold_nanos) break;
    num_allocations *= 2;
    ptrs = realloc(ptrs, sizeof(int*) * num_allocations);
  }

  if (stdout_is_tty()) {
    putchar('\n');
  }

  const int reduction_steps = 5;
  for (int j = 0; j < reduction_steps; j++) {
    while (true) {
      const unsigned long m = num_allocations - num_allocations / (1 << (j + 1));
      if (stdout_is_tty()) {
        printf("\rreducing %d/%d %lu", j + 1, reduction_steps, m);
        fflush(stdout);
      }
      const struct seninstant begin = seninstant_now();
      for (unsigned long i = 0; i < m; i++) {
        volatile int *ints = malloc(sizeof(int) * ints_to_allocate);
        ptrs[i] = ints;
        ints[1] = 0;
      }
      const uint64_t red_nanos = seninstant_subtract(seninstant_now(), begin);
      for (unsigned long i = 0; i < m; i++) {
        free((void*) ptrs[i]);
      }
      if (red_nanos < time_threshold_nanos) {
        break;
      }
      num_allocations = m;
      nanos = red_nanos;
    }
  }
  if (stdout_is_tty()) {
    putchar('\n');
  }
  printf("Doing %lu malloc()s per round.\n"
    "This number has been determined empirically for a round time >= %.3fs.\n",
    num_allocations, ns_to_s(nanos));
  free((void*) ptrs);
  return num_allocations;
}

static
void clearln(void) {
  if (stdout_is_tty()) {
    puts("\33[2K");
  }
}

int main(void) {
  double arena_alloc_throughput = 0;
  double arena_free_throughput = 0;
  double arena_alloc_reused_throughput = 0;
  const unsigned long num_arena_allocations = determine_arena_alloc_amt();
  const unsigned long num_standard_allocations = determine_standard_alloc_amt();

  {
    struct minmax_d arena_alloc_aggs = {0};
    struct minmax_d arena_alloc_reused_aggs = {0};
    struct minmax_d arena_free_aggs = {0};

    puts("\n# Benchmarking arena use");
    struct senarena arena;

    for (int j = 0; j < num_iters; j++) {
      if (stdout_is_tty()) {
        printf("\r%d/%d (a)", j + 1, num_iters);
        fflush(stdout);
      }
      arena = senarena_new();
      {
        const struct seninstant begin = seninstant_now();
        for (unsigned long i = 0; i < num_arena_allocations; i++) {
          volatile int *ints = senarena_alloc_array_of(&arena, int, ints_to_allocate);
          ints[1] = 42;
        }
        const struct seninstant end = seninstant_now();
        const uint64_t nanos = seninstant_subtract(end, begin);
        if (j == 0) {
          arena_alloc_aggs.min = nanos;
          arena_alloc_aggs.max = nanos;
        } else {
          minmax_d(&arena_alloc_aggs, nanos);
        }
      }

      if (j == num_iters - 1) {
        if (stdout_is_tty()) {
          putchar('\r');
        }
        puts("# Benchmarking arena reuse");

        for (int k = 0; k < num_iters; k++) {
          if (stdout_is_tty()) {
            printf("\r%d/%d (r)", k + 1, num_iters);
            fflush(stdout);
          }
          senarena_clear(&arena);
          {
            const struct seninstant begin = seninstant_now();
            for (unsigned long i = 0; i < num_arena_allocations; i++) {
              volatile int *ints = senarena_alloc_array_of(&arena, int, ints_to_allocate);
              ints[1] = 42;
            }
            const struct seninstant end = seninstant_now();
            const uint64_t nanos = seninstant_subtract(end, begin);
            if (k == 0) {
              arena_alloc_reused_aggs.min = nanos;
              arena_alloc_reused_aggs.max = nanos;
            } else {
              minmax_d(&arena_alloc_reused_aggs, nanos);
            }
          }
        }
      }

      if (stdout_is_tty()) {
        printf("\r%d/%d (f)", j + 1, num_iters);
        fflush(stdout);
      }

      const struct seninstant begin = seninstant_now();
      senarena_free(arena);
      const struct seninstant end = seninstant_now();
      const uint64_t nanos = seninstant_subtract(end, begin);
      if (j == 0) {
        arena_free_aggs.min = nanos;
        arena_free_aggs.max = nanos;
      } else {
        minmax_d(&arena_free_aggs, nanos);
      }
    }

    clearln();
    {
      double min_time = ns_to_s(arena_alloc_aggs.min);
      double max_time = ns_to_s(arena_alloc_aggs.max);
      printf("Arena allocation time:             %.3fs (min), %.3fs (max)\n", min_time, max_time);
    }
    {
      double min_time = ns_to_s(arena_alloc_reused_aggs.min);
      double max_time = ns_to_s(arena_alloc_reused_aggs.max);
      printf("Arena (reused) allocation time:    %.3fs (min), %.3fs (max)\n", min_time, max_time);
    }
    {
      double min_time = ns_to_s(arena_free_aggs.min);
      double max_time = ns_to_s(arena_free_aggs.max);
      printf("Arena free time:                   %.3fs (min), %.3fs (max)\n", min_time, max_time);
    }
    arena_alloc_throughput = num_arena_allocations / ((double) arena_alloc_aggs.max / 1000);
    printf("Arena allocations per us:          %.3f\n", arena_alloc_throughput);
    arena_alloc_reused_throughput = num_arena_allocations / ((double) arena_alloc_reused_aggs.max / 1000);
    printf("Arena (reused) allocations per us: %.3f\n", arena_alloc_reused_throughput);
    arena_free_throughput = num_arena_allocations / ((double) arena_free_aggs.max / 1000);
    printf("Arena allocation frees per us:     %.3f\n", arena_free_throughput);
  }

  clearln();
  puts("# Benchmarking malloc use");

  double std_alloc_throughput = 0;
  double std_free_throughput = 0;

  {
    struct minmax_d std_alloc_aggs = {0};
    struct minmax_d std_free_aggs = {0};

    volatile int **ptrs = malloc(sizeof(int*) * num_standard_allocations);

    for (int j = 0; j < num_iters; j++) {
      if (stdout_is_tty()) {
        printf("\r%d/%d (a)", j + 1, num_iters);
        fflush(stdout);
      }
      {
        const struct seninstant begin = seninstant_now();
        for (unsigned long i = 0; i < num_standard_allocations; i++) {
          volatile int *ints = malloc(sizeof(int) * ints_to_allocate);
          // This doesn't slow down the benchmark enough to make
          // it unfair.
          ptrs[i] = ints;
          ints[1] = 0;
        }
        const struct seninstant end = seninstant_now();
        const uint64_t nanos = seninstant_subtract(end, begin);
        if (j == 0) {
          std_alloc_aggs.min = nanos;
          std_alloc_aggs.max = nanos;
        } else {
          minmax_d(&std_alloc_aggs, nanos);
        }
      }

      if (stdout_is_tty()) {
        printf("\r%d/%d (f)", j + 1, num_iters);
        fflush(stdout);
      }

      {
        const struct seninstant begin = seninstant_now();
        for (unsigned long i = 0; i < num_standard_allocations; i++) {
          free((void*)ptrs[i]);
        }
        const struct seninstant end = seninstant_now();
        const uint64_t nanos = seninstant_subtract(end, begin);
        if (j == 0) {
          std_free_aggs.min = nanos;
          std_free_aggs.max = nanos;
        } else {
          minmax_d(&std_free_aggs, nanos);
        }
      }
    }
    free(ptrs);
    clearln();
    {
      double min_time = ns_to_s(std_alloc_aggs.min);
      double max_time = ns_to_s(std_alloc_aggs.max);
      printf("Standard allocation time:         %.3fs (min), %.3fs (max)\n", min_time, max_time);
    }
    {
      double min_time = ns_to_s(std_free_aggs.min);
      double max_time = ns_to_s(std_free_aggs.max);
      printf("Standard free time:               %.3fs (min), %.3fs (max)\n", min_time, max_time);
    }
    std_alloc_throughput = num_standard_allocations / ((double) std_alloc_aggs.max / 1000);
    printf("Standard allocations per us:      %.3f\n", std_alloc_throughput);
    std_free_throughput = num_standard_allocations / ((double) std_free_aggs.max / 1000);
    printf("Standard allocation frees per us: %.3f\n", std_free_throughput);
  }

  putchar('\n');
  printf("         arena_alloc() vs malloc() speedup: %.3f\n", arena_alloc_throughput / std_alloc_throughput);
  printf("(reused) arena_alloc() vs malloc() speedup: %.3f\n", arena_alloc_reused_throughput / std_alloc_throughput);
  printf("          arena_free() vs   free() speedup: %.3f\n", arena_free_throughput / std_free_throughput);
}
