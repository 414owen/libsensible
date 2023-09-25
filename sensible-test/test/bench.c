// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#include <time.h>

#include "sensible-test.h"

#ifdef _WIN32
  #define DEVNULL "nul"
#else
  #define DEVNULL "/dev/null"
#endif

static
void print_llu_underscored(long long unsigned n) {
  // printf("(%llu)", n);
  int length = 0;
  int exp = 1;
  {
    int m = n;
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
  struct sentest_config config = {
    .output = fopen(DEVNULL, "w"),
    .color = true,
    .filter_str = NULL,
    .junit_output_path = DEVNULL,
  };
  int n = 10000000;

  {
    double start_time = (double)clock()/CLOCKS_PER_SEC;
    struct sentest_state *state = sentest_start(config);
    for (int i = 0; i < n; i++) {
      sentest_group(state, "suite which passes") {
      }
    }
    sentest_finish(state);
    double end_time = (double)clock()/CLOCKS_PER_SEC;
    fputs("Groups per second: ", stdout);
    print_llu_underscored(n / (end_time - start_time));
    putchar('\n');
  }

  {
    double start_time = (double)clock()/CLOCKS_PER_SEC;
    struct sentest_state *state = sentest_start(config);
    sentest_group(state, "suite which passes") {
      for (int i = 0; i < n; i++) {
        sentest(state, "contains balances <testsuite>s") {
        }
      }
    }
    sentest_finish(state);
    double end_time = (double)clock()/CLOCKS_PER_SEC;
    fputs("Tests per second: ", stdout);
    print_llu_underscored(n / (end_time - start_time));
    putchar('\n');
  }

  {
    double start_time = (double)clock()/CLOCKS_PER_SEC;
    struct sentest_state *state = sentest_start(config);
    sentest_group(state, "suite which passes") {
      sentest(state, "contains balances <testsuite>s") {
        for (int i = 0; i < n; i++) {
          sentest_assert_eq(state, 1, 1);
        }
      }
    }
    sentest_finish(state);
    double end_time = (double)clock()/CLOCKS_PER_SEC;
    fputs("Asserts per second: ", stdout);
    print_llu_underscored(n / (end_time - start_time));
    putchar('\n');
  }
  fclose(config.output);
}
