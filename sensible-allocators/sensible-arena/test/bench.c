// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#include <stdio.h>
#include <time.h>

#include "../../../sensible-test/src/sensible-test.h"
#include "../src/sensible-arena.h"

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



int main(void) {
  {
    const unsigned long n = 1000000000;
    struct senarena arena = senarena_new();

    {
      double start_time = (double)clock()/CLOCKS_PER_SEC;
      for (unsigned long i = 0; i < n; i++) {
        volatile int *ints = senarena_alloc_array_of(&arena, int, 2);
        ints[1] = 0;
      }
      double end_time = (double)clock()/CLOCKS_PER_SEC;
      fputs("Small allocations per second: ", stdout);
      print_llu_underscored(n / (end_time - start_time));
      putchar('\n');
    }

    {
      double start_time = (double)clock()/CLOCKS_PER_SEC;
      senarena_free(arena);
      double end_time = (double)clock()/CLOCKS_PER_SEC;
      fputs("Time to free: ", stdout);
      print_llu_underscored(end_time * 1000000 - start_time * 1000000);
      puts("ns");
    }
  }

  {
    const unsigned long n = 100000000;
    volatile int **ptrs = malloc(sizeof(void*) * n);

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
      fputs("Small *standard malloc* allocations per second: ", stdout);
      print_llu_underscored(n / (end_time - start_time));
      putchar('\n');
    }

    {
      double start_time = (double)clock()/CLOCKS_PER_SEC;
      for (unsigned long i = 0; i < n; i++) {
        free((void*)ptrs[i]);
      }
      double end_time = (double)clock()/CLOCKS_PER_SEC;
      fputs("Time to free all *standard malloc* ptrs: ", stdout);
      print_llu_underscored(end_time * 1000000 - start_time * 1000000);
      puts("ns");
    }
    free(ptrs);
  }
}
