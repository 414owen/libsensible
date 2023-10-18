// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#define _XOPEN_SOURCE 500

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "../../../sensible-test/src/sensible-test.h"
#include "../../../sensible-timing/src/sensible-timing.h"
#include "../src/sensible-arena.h"

#define ROUNDS 10
#define VERBOSE false
#define REDUCTION_STEPS 10

static const double time_threshold_nanos = 5e8; // rounds should take at least 0.5s
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
double ns_to_s(uint64_t ns) {
  // three decimal places
  return (double) ns / 1e9;
}

static
uint64_t scale_allocations_up(uint64_t a) {
  return a + (a >> 1) + (a >> 2);
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
    num_allocations = scale_allocations_up(num_allocations);
  }

  if (stdout_is_tty()) {
    putchar('\n');
  }

  for (int j = 0; j < REDUCTION_STEPS; j++) {
    while (true) {
      const unsigned long m = num_allocations - num_allocations / (1 << (j + 1));
      if (stdout_is_tty()) {
        printf("\rreducing %d/%d %lu", j + 1, REDUCTION_STEPS, m);
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
    "This number has been determined empirically for a round time ≈ %.3fs.\n\n",
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
    num_allocations = scale_allocations_up(num_allocations);
    ptrs = realloc(ptrs, sizeof(int*) * num_allocations);
  }

  if (stdout_is_tty()) {
    putchar('\n');
  }

  for (int j = 0; j < REDUCTION_STEPS; j++) {
    while (true) {
      const unsigned long m = num_allocations - num_allocations / (1 << (j + 1));
      if (stdout_is_tty()) {
        printf("\rreducing %d/%d %lu", j + 1, REDUCTION_STEPS, m);
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
    "This number has been determined empirically for a round time ≈ %.3fs.\n\n",
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

static
double calc_sum_d(double *values, size_t num_values) {
  double sum = 0;
  for (size_t i = 0; i < num_values; i++) {
    sum += values[i];
  }
  return sum;
}

static
double calc_mean_d(double *values, size_t num_values) {
  double sum = calc_sum_d(values, num_values);
  return sum / num_values;
}

static
double calc_stddev(double *values, size_t num_values) {
  double res = 0;
  double mean = calc_mean_d(values, num_values);
  for (size_t i = 0; i < num_values; i++) {
    double diff = values[i] - mean;
    res += diff * diff;
  }
  res /= num_values;
  return sqrt(res);
}

static
uint64_t minimum(uint64_t *values, size_t num_values) {
  uint64_t res = values[0];
  for (size_t i = 0; i < num_values; i++) {
    uint64_t value = values[i];
    if (value < res) {
      res = value;
    }
  }
  return res;
}

static
uint64_t maximum(uint64_t *values, size_t num_values) {
  uint64_t res = values[0];
  for (size_t i = 0; i < num_values; i++) {
    uint64_t value = values[i];
    if (value > res) {
      res = value;
    }
  }
  return res;
}

static
void nanos_to_seconds(double *seconds, uint64_t *nanos, size_t num_values) {
  for (size_t i = 0; i < num_values; i++) {
    seconds[i] = ns_to_s(nanos[i]);
  }
}

static
double calc_sum(uint64_t *values, size_t num_values) {
  uint64_t res = 0;
  for (size_t i = 0; i < num_values; i++) {
    res += values[i];
  }
  return res;
}

static
double calc_mean(uint64_t *values, size_t num_values) {
  return (double) calc_sum(values, num_values) / (double) num_values;
}

int main(void) {
  uint64_t arena_alloc_times_nanos[ROUNDS];
  uint64_t arena_alloc_reused_times_nanos[ROUNDS];
  uint64_t arena_alloc_free_times_nanos[ROUNDS];

  const unsigned long num_arena_allocations = determine_arena_alloc_amt();
  const unsigned long num_standard_allocations = determine_standard_alloc_amt();

  {
    puts("# Benchmarking arena use");
    struct senarena arena;

    for (int j = 0; j < ROUNDS; j++) {
      if (stdout_is_tty()) {
        printf("\r%d/%d (a)", j + 1, ROUNDS);
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
        arena_alloc_times_nanos[j] = nanos;
      }

      if (j == ROUNDS - 1) {
        if (stdout_is_tty()) {
          putchar('\r');
        }
        puts("# Benchmarking arena reuse");

        for (int k = 0; k < ROUNDS; k++) {
          if (stdout_is_tty()) {
            printf("\r%d/%d (r)", k + 1, ROUNDS);
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
            arena_alloc_reused_times_nanos[k] = nanos;
          }
        }
      }

      if (stdout_is_tty()) {
        printf("\r%d/%d (f)", j + 1, ROUNDS);
        fflush(stdout);
      }

      const struct seninstant begin = seninstant_now();
      senarena_free(arena);
      const struct seninstant end = seninstant_now();
      const uint64_t nanos = seninstant_subtract(end, begin);
      arena_alloc_free_times_nanos[j] = nanos;
    }
  }

  puts("\r# Benchmarking malloc use");
  uint64_t std_alloc_times_nanos[ROUNDS];
  uint64_t std_free_times_nanos[ROUNDS];

  {
    volatile int **ptrs = malloc(sizeof(int*) * num_standard_allocations);

    for (int j = 0; j < ROUNDS; j++) {
      if (stdout_is_tty()) {
        printf("\r%d/%d (a)", j + 1, ROUNDS);
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
        std_alloc_times_nanos[j] = nanos;
      }

      if (stdout_is_tty()) {
        printf("\r%d/%d (f)", j + 1, ROUNDS);
        fflush(stdout);
      }

      {
        const struct seninstant begin = seninstant_now();
        for (unsigned long i = 0; i < num_standard_allocations; i++) {
          free((void*)ptrs[i]);
        }
        const struct seninstant end = seninstant_now();
        const uint64_t nanos = seninstant_subtract(end, begin);
        std_free_times_nanos[j] = nanos;
      }
    }
    free(ptrs);
    clearln();
  }

  if (VERBOSE) {
    double min_time = ns_to_s(minimum(arena_alloc_times_nanos, ROUNDS));
    double max_time = ns_to_s(maximum(arena_alloc_times_nanos, ROUNDS));
    double alloc_times_in_seconds[ROUNDS];
    nanos_to_seconds(alloc_times_in_seconds, arena_alloc_times_nanos, ROUNDS);
    double stddev_time = calc_stddev(alloc_times_in_seconds, ROUNDS);
    printf("Arena allocation time:\n"
      "  min:    %.3fs\n"
      "  max:    %.3fs\n"
      "  stdenv: %.3f\n\n", min_time, max_time, stddev_time);
  }
  if (VERBOSE) {
    double min_time = ns_to_s(minimum(arena_alloc_reused_times_nanos, ROUNDS));
    double max_time = ns_to_s(maximum(arena_alloc_reused_times_nanos, ROUNDS));
    double alloc_times_in_seconds[ROUNDS];
    nanos_to_seconds(alloc_times_in_seconds, arena_alloc_reused_times_nanos, ROUNDS);
    double stddev_time = calc_stddev(alloc_times_in_seconds, ROUNDS);
    printf("Arena (reused) allocation time:\n"
      "  min:    %.3fs\n"
      "  max:    %.3fs\n"
      "  stddev: %.3f\n\n", min_time, max_time, stddev_time);
  }
  if (VERBOSE) {
    double min_time = ns_to_s(minimum(arena_alloc_free_times_nanos, ROUNDS));
    double max_time = ns_to_s(maximum(arena_alloc_free_times_nanos, ROUNDS));
    double free_times_in_seconds[ROUNDS];
    nanos_to_seconds(free_times_in_seconds, arena_alloc_free_times_nanos, ROUNDS);
    double stddev_time = calc_stddev(free_times_in_seconds, ROUNDS);
    printf("Arena free time:\n"
      "  min:    %.3fs\n"
      "  max:    %.3fs\n"
      "  stddev: %.3f\n\n", min_time, max_time, stddev_time);
  }
  double arena_alloc_throughput = num_arena_allocations / (calc_mean(arena_alloc_times_nanos, ROUNDS) / 1000);
  printf("Arena allocation throughput:                  %.3f ops/μs\n", arena_alloc_throughput);
  double arena_alloc_reused_throughput = num_arena_allocations / (calc_mean(arena_alloc_reused_times_nanos, ROUNDS)/ 1000);
  printf("Arena (reused) allocation throughput:         %.3f ops/μs\n", arena_alloc_reused_throughput);
  double arena_free_throughput = num_arena_allocations / (calc_mean(arena_alloc_free_times_nanos, ROUNDS) / 1000);
  printf("Arena free throughput:                        %.3f freed allocations/μs\n", arena_free_throughput);
  putchar('\n');

  if (VERBOSE) {
    double min_time = ns_to_s(minimum(std_alloc_times_nanos, ROUNDS));
    double max_time = ns_to_s(maximum(std_alloc_times_nanos, ROUNDS));
    double alloc_times_in_seconds[ROUNDS];
    nanos_to_seconds(alloc_times_in_seconds, std_alloc_times_nanos, ROUNDS);
    double stddev_time = calc_stddev(alloc_times_in_seconds, ROUNDS);
    printf("Standard allocation time:\n"
      "  min: %.3fs\n"
      "  max: %.3fs\n"
      "  stddev: %.3f\n\n", min_time, max_time, stddev_time);
  }
  if (VERBOSE) {
    double min_time = ns_to_s(minimum(std_free_times_nanos, ROUNDS));
    double max_time = ns_to_s(maximum(std_free_times_nanos, ROUNDS));
    double free_times_in_seconds[ROUNDS];
    nanos_to_seconds(free_times_in_seconds, std_free_times_nanos, ROUNDS);
    double stddev_time = calc_stddev(free_times_in_seconds, ROUNDS);
    printf("Standard free time:\n"
      "  min: %.3fs\n"
      "  max: %.3fs\n"
      "  stdenv: %.3f\n\n", min_time, max_time, stddev_time);
  }
  double std_alloc_throughput = num_standard_allocations / (calc_mean(std_alloc_times_nanos, ROUNDS) / 1000);
  printf("stdlib malloc() throughput:                   %.3f ops/μs\n", std_alloc_throughput);
  double std_free_throughput = num_standard_allocations / (calc_mean(std_alloc_times_nanos, ROUNDS) / 1000);
  printf("stdlib free() throughput:                     %.3f ops/μs\n", std_free_throughput);

  putchar('\n');
  printf("         arena_alloc() vs malloc() speedup:   %.3f\n", arena_alloc_throughput / std_alloc_throughput);
  printf("(reused) arena_alloc() vs malloc() speedup:   %.3f\n", arena_alloc_reused_throughput / std_alloc_throughput);
  printf("          arena_free() vs   free() speedup:   %.3f\n", arena_free_throughput / std_free_throughput);
}
