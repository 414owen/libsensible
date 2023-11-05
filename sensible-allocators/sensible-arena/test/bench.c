// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#define _XOPEN_SOURCE 500

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "../../../sensible-test/src/sensible-test.h"
#include "../../../sensible-timing/src/sensible-timing.h"
#include "../src/sensible-arena.h"

#define ROUNDS 20
#define VERBOSE true
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
double ns_to_s(double ns) {
  // three decimal places
  return (double) ns / 1e9;
}

static
uint64_t scale_allocations_up(uint64_t a) {
  return a + (a >> 1) + (a >> 2);
}

static
void clearln(void) {
  if (stdout_is_tty()) {
    printf("\33[2K\r");
  }
}

static
void clearln_nl(void) {
  if (stdout_is_tty()) {
    puts("\33[2K\r");
  }
}

// Find the number of allocations that makes sense per benchmark iteration
static
unsigned long determine_arena_alloc_amt(void) {
  if (stdout_is_tty()) {
    printf("Finding out how many arena allocations will take %.3fs\n", ns_to_s(time_threshold_nanos));
  }

  unsigned long num_allocations = 2 << 10; // 1024
  uint64_t nanos = 0;
  while (true) {
    if (stdout_is_tty()) {
      printf("\rIncreasing allocations: %lu", num_allocations);
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

  clearln();

  for (int j = 0; j < REDUCTION_STEPS; j++) {
    while (true) {
      const unsigned long m = num_allocations - num_allocations / (1 << (j + 1));
      if (stdout_is_tty()) {
        printf("\rReducing %d/%d %lu", j + 1, REDUCTION_STEPS, m);
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
    printf("\rDoing %lu arena_alloc()s per round.\n"
      "This number has been determined empirically for a round time ≈ %.3fs.\n\n",
      num_allocations, ns_to_s(nanos));
  }
  return num_allocations;
}

// Find the number of allocations that makes sense per benchmark iteration
static
unsigned long determine_standard_alloc_amt(void) {
  if (stdout_is_tty()) {
    printf("Finding out how many standard allocations will take %.3fs\n", ns_to_s(time_threshold_nanos));
  }

  unsigned long num_allocations = 2 << 10; // 1024
  volatile int **ptrs = malloc(sizeof(int*) * num_allocations);
  uint64_t nanos = 0;
  while (true) {
    if (stdout_is_tty()) {
      printf("\rIncreasing allocations: %lu", num_allocations);
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

  clearln();

  for (int j = 0; j < REDUCTION_STEPS; j++) {
    while (true) {
      const unsigned long m = num_allocations - num_allocations / (1 << (j + 1));
      if (stdout_is_tty()) {
        printf("\rReducing %d/%d %lu", j + 1, REDUCTION_STEPS, m);
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
    printf("\rDoing %lu malloc()s per round.\n"
      "This number has been determined empirically for a round time ≈ %.3fs.\n\n",
      num_allocations, ns_to_s(nanos));
  }
  free((void*) ptrs);
  return num_allocations;
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
double calc_stddev_d(double *values, size_t num_values) {
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
double minimum_d(double *values, size_t num_values) {
  double res = values[0];
  for (size_t i = 0; i < num_values; i++) {
    double value = values[i];
    if (value < res) {
      res = value;
    }
  }
  return res;
}

static
double maximum_d(double *values, size_t num_values) {
  double res = values[0];
  for (size_t i = 0; i < num_values; i++) {
    double value = values[i];
    if (value > res) {
      res = value;
    }
  }
  return res;
}

struct min_max_mean {
  double min;
  double max;
  double mean;
  double stddev;
};

struct aggregates {
  struct min_max_mean time_s;
  struct min_max_mean throughtput_us;
};

struct aggregates calc_aggregates(uint64_t num_iters, uint64_t *times_nanos, uint64_t num_values) {
  struct aggregates res;
  {
    double *times_s = malloc(sizeof(double) * num_values);
    times_s[0] = 0;
    for (size_t i = 0; i < num_values; i++) {
      times_s[i] = ns_to_s(times_nanos[i]);
    }
    struct min_max_mean time = {
      .min = minimum_d(times_s, num_values),
      .max = maximum_d(times_s, num_values),
      .mean = calc_mean_d(times_s, num_values),
      .stddev = calc_stddev_d(times_s, num_values)
    };
    res.time_s = time;
    free(times_s);
  }
  {
    double *throughputs = malloc(sizeof(double) * num_values);
    throughputs[0] = 0;
    for (size_t i = 0; i < num_values; i++) {
      throughputs[i] = 1000 * (double) num_iters / times_nanos[i];
    }
    struct min_max_mean throughput = {
      .min = minimum_d(throughputs, num_values),
      .max = maximum_d(throughputs, num_values),
      .mean = calc_mean_d(throughputs, num_values),
      .stddev = calc_stddev_d(throughputs, num_values)
    };
    res.throughtput_us = throughput;
    free(throughputs);
  }
  return res;
}

static
void print_aggregates(const char *name, struct aggregates aggs) {
  if (VERBOSE) printf("%s time:\n"
    "  min:    %.4fs\n"
    "  max:    %.4fs\n"
    "  mean:   %.4fs\n"
    "  stdenv: %.4f\n\n",
    name,
    aggs.time_s.min,
    aggs.time_s.max,
    aggs.time_s.mean,
    aggs.time_s.stddev);
}

int main(void) {
  const unsigned long num_arena_allocations = determine_arena_alloc_amt();
  const unsigned long num_standard_allocations = determine_standard_alloc_amt();

  struct aggregates arena_alloc_aggregates;
  struct aggregates arena_alloc_reused_aggregates;
  struct aggregates arena_free_aggregates;

  {
    uint64_t arena_alloc_times_nanos[ROUNDS];
    uint64_t arena_alloc_reused_times_nanos[ROUNDS];
    uint64_t arena_alloc_free_times_nanos[ROUNDS];

    if (stdout_is_tty()) {
      puts("# Benchmarking arena use");
    }
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
          clearln();
          puts("# Benchmarking arena reuse");
        }

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

    clearln_nl();

    arena_alloc_aggregates = calc_aggregates(num_arena_allocations, arena_alloc_times_nanos, ROUNDS);
    arena_alloc_reused_aggregates = calc_aggregates(num_arena_allocations, arena_alloc_reused_times_nanos, ROUNDS);
    arena_free_aggregates = calc_aggregates(num_arena_allocations, arena_alloc_free_times_nanos, ROUNDS);

    print_aggregates("Arena allocation", arena_alloc_aggregates);
    print_aggregates("Arena (reused) allocation", arena_alloc_reused_aggregates);
    print_aggregates("Arena free", arena_free_aggregates);

    printf("Arena allocation throughput:                  %.3f ops/μs\n", arena_alloc_aggregates.throughtput_us.mean);
    printf("Arena (reused) allocation throughput:         %.3f ops/μs\n", arena_alloc_reused_aggregates.throughtput_us.mean);
    printf("Arena free throughput:                        %.3f freed allocations/μs\n", arena_free_aggregates.throughtput_us.mean);
    putchar('\n');
  }

  if (stdout_is_tty()) {
    puts("# Benchmarking malloc use");
  }

  struct aggregates std_alloc_aggregates;
  struct aggregates std_free_aggregates;

  {
    uint64_t std_alloc_times_nanos[ROUNDS];
    uint64_t std_free_times_nanos[ROUNDS];

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
    clearln_nl();

    std_alloc_aggregates = calc_aggregates(num_standard_allocations, std_alloc_times_nanos, ROUNDS);
    std_free_aggregates = calc_aggregates(num_standard_allocations, std_free_times_nanos, ROUNDS);

    print_aggregates("Standard allocation", std_alloc_aggregates);
    print_aggregates("Standard free", std_free_aggregates);

    printf("stdlib malloc() throughput:                   %.3f ops/μs\n", std_alloc_aggregates.throughtput_us.mean);
    printf("stdlib free() throughput:                     %.3f ops/μs\n", std_free_aggregates.throughtput_us.mean);
    putchar('\n');
  }

  puts("# Minimums");
  printf("         arena_alloc() vs malloc() speedup:   %.3f\n", arena_alloc_aggregates.throughtput_us.min / std_alloc_aggregates.throughtput_us.min);
  printf("(reused) arena_alloc() vs malloc() speedup:   %.3f\n", arena_alloc_reused_aggregates.throughtput_us.min / std_alloc_aggregates.throughtput_us.min);
  printf("          arena_free() vs   free() speedup:   %.3f\n", arena_free_aggregates.throughtput_us.min / std_free_aggregates.throughtput_us.min);
  putchar('\n');

  puts("# Maximums");
  printf("         arena_alloc() vs malloc() speedup:   %.3f\n", arena_alloc_aggregates.throughtput_us.max / std_alloc_aggregates.throughtput_us.max);
  printf("(reused) arena_alloc() vs malloc() speedup:   %.3f\n", arena_alloc_reused_aggregates.throughtput_us.max / std_alloc_aggregates.throughtput_us.max);
  printf("          arena_free() vs   free() speedup:   %.3f\n", arena_free_aggregates.throughtput_us.max / std_free_aggregates.throughtput_us.max);
  putchar('\n');

  puts("# Means");
  printf("         arena_alloc() vs malloc() speedup:   %.3f\n", arena_alloc_aggregates.throughtput_us.mean / std_alloc_aggregates.throughtput_us.mean);
  printf("(reused) arena_alloc() vs malloc() speedup:   %.3f\n", arena_alloc_reused_aggregates.throughtput_us.mean / std_alloc_aggregates.throughtput_us.mean);
  printf("          arena_free() vs   free() speedup:   %.3f\n", arena_free_aggregates.throughtput_us.mean / std_free_aggregates.throughtput_us.mean);
}
