// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#include <stdio.h>
#include <time.h>

#include "../../../sensible-test/src/sensible-test.h"
#include "../src/sensible-arena.h"

/*
static
void print_llu_underscored(long long unsigned n) {
  unsigned long long length = 0;
  unsigned long long exp = 1;
  {
    unsigned long long m = n;
    while (m > 0) {
      length++;
      m /= 10;
      exp *= 10;
    }
  }

  if (length == 0) {
    length = 1;
  }

  exp /= 10;
  for (int i = length; i > 0; i--) {
    putchar('0' + (n / exp));
    n %= exp;
    exp /= 10;
    if (i > 1 && (i - 1) % 3 == 0) {
      putchar('_');
    }
  }
}
*/

struct minmax_d {
  double min;
  double max;
};

void minmax_d(struct minmax_d *out, double d) {
  if (d < out->min)
    out->min = d;
  if (d > out->max)
    out->max= d;
}

int main(void) {
  struct minmax_d arena_alloc_aggs = {0};
  struct minmax_d arena_free_aggs = {0};
  double arena_alloc_throughput = 0;
  double arena_free_throughput = 0;
  double time_threshold_seconds = 1.5; // rounds should take at least 1.5s

  {
    unsigned long n = 1;
    puts("Finding out how many arena allocations will take half a second");

    {
      double seconds = 0;
      while (true) {
        struct senarena a1 = senarena_new();
        const double start_time = (double)clock()/CLOCKS_PER_SEC;
        for (unsigned long i = 0; i < n; i++) {
          volatile int *ints = senarena_alloc_array_of(&a1, int, 2);
          ints[1] = 0;
        }
        const double end_time = (double)clock()/CLOCKS_PER_SEC;
        seconds = end_time - start_time;
        senarena_free(a1);
        if (seconds >= time_threshold_seconds) break;
        n *= 2;
      }
      printf("%lu arena allocations took %fs\n", n, seconds);
    }

    for (int j = 0; j < 10; j++) {
      printf("\r%d/10", j + 1);
      fflush(stdout);
      struct senarena arena = senarena_new();
      {
        const double start_time = (double)clock()/CLOCKS_PER_SEC;
        for (unsigned long i = 0; i < n; i++) {
          volatile int *ints = senarena_alloc_array_of(&arena, int, 2);
          ints[1] = 0;
        }
        const double end_time = (double)clock()/CLOCKS_PER_SEC;
        const double time = end_time - start_time;
        if (j == 0) {
          arena_alloc_aggs.min = time;
          arena_alloc_aggs.max = time;
        } else {
          minmax_d(&arena_alloc_aggs, time);
        }
      }

      {
        double start_time = (double)clock()/CLOCKS_PER_SEC;
        senarena_free(arena);
        double end_time = (double)clock()/CLOCKS_PER_SEC;
        const double time = end_time - start_time;
        if (j == 0) {
          arena_free_aggs.min = time;
          arena_free_aggs.max = time;
        } else {
          minmax_d(&arena_free_aggs, time);
        }
      }
    }
    putchar('\r');
    printf("Minimum arena allocation time: %fs\n", arena_alloc_aggs.min);
    printf("Maximum arena allocation time: %fs\n", arena_alloc_aggs.max);
    printf("Minimum arena free time: %fs\n", arena_free_aggs.min);
    printf("Maximum arena free time: %fs\n", arena_free_aggs.max);
    arena_alloc_throughput = n / (arena_alloc_aggs.max * 1000);
    printf("Maximum arena allocations per ms: %f\n", arena_alloc_throughput);
    arena_free_throughput = n / (arena_free_aggs.max * 1000);
    printf("Maximum arena allocation frees per ms: %f\n", arena_free_throughput);
  }

  putchar('\n');

  struct minmax_d std_alloc_aggs = {0};
  struct minmax_d std_free_aggs = {0};
  double std_alloc_throughput = 0;
  double std_free_throughput = 0;
  {
    unsigned long n = 1;
    puts("Finding out how many malloc allocations will take half a second");
    volatile int **ptrs = malloc(sizeof(int*) * n);

    {
      double seconds = 0;
      while (true) {
        double start_time = (double)clock()/CLOCKS_PER_SEC;
        for (unsigned long i = 0; i < n; i++) {
          volatile int *ints = malloc(sizeof(int) * 2);
          ptrs[i] = ints;
          ints[1] = 0;
        }
        double end_time = (double)clock()/CLOCKS_PER_SEC;
        for (unsigned long i = 0; i < n; i++) {
          free((void*) ptrs[i]);
        }
        seconds = end_time - start_time;
        if (seconds >= time_threshold_seconds) break;
        n *= 2;
        ptrs = realloc(ptrs, sizeof(int*) * n);
      }
      printf("%lu malloc allocations took %fs\n", n, seconds);
    }

    for (int j = 0; j < 10; j++) {
      printf("\r%d/10", j + 1);
      {
        double start_time = (double)clock()/CLOCKS_PER_SEC;
        for (unsigned long i = 0; i < n; i++) {
          volatile int *ints = malloc(sizeof(int) * 2);
          // This doesn't slow down the benchmark enough to make
          // it unfair.
          ptrs[i] = ints;
          ints[1] = 0;
        }
        double end_time = (double)clock()/CLOCKS_PER_SEC;
        const double time = end_time - start_time;
        if (j == 0) {
          std_alloc_aggs.min = time;
          std_alloc_aggs.max = time;
        } else {
          minmax_d(&std_alloc_aggs, time);
        }
      }

      {
        const double start_time = (double)clock()/CLOCKS_PER_SEC;
        for (unsigned long i = 0; i < n; i++) {
          free((void*)ptrs[i]);
        }
        const double end_time = (double)clock()/CLOCKS_PER_SEC;
        const double time = end_time - start_time;
        if (j == 0) {
          std_free_aggs.min = time;
          std_free_aggs.max = time;
        } else {
          minmax_d(&std_free_aggs, time);
        }
      }
    }
    free(ptrs);
    putchar('\r');
    printf("Minimum standard allocation time: %fs\n", std_alloc_aggs.min);
    printf("Maximum standard allocation time: %fs\n", std_alloc_aggs.max);
    printf("Minimum standard free time: %fs\n", std_free_aggs.min);
    printf("Maximum standard free time: %fs\n", std_free_aggs.max);
    std_alloc_throughput = n / (std_alloc_aggs.max * 1000);
    printf("Maximum standard allocations per ms: %f\n", std_alloc_throughput);
    std_free_throughput = n / (std_free_aggs.max * 1000);
    printf("Maximum standard allocation frees per ms: %f\n", std_free_throughput);
  }

  putchar('\n');
  printf("arena_alloc vs malloc speedup: %f\n", arena_alloc_throughput / std_alloc_throughput);
  printf("arena_free vs free() speedup: %f\n", arena_free_throughput / std_free_throughput);
}
