// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "sensible-test.h"

int run_example_suite(bool has_failure, const char *out_path) {
  struct sentest_config config = {
    .output = fopen(out_path, "w"),
    .color = false,
    .filter_str = NULL,
    .junit = true,
  };
  struct sentest_state *state = sentest_start(config);
  sentest_group(state, "test-suite") {
    sentest_group(state, "trivial") {
      sentest(state, "assert true") {
        sentest_assert(state, true);
      }
    }
    sentest_group(state, "equality") {
      if (has_failure) {
        sentest(state, "one is two") {
          sentest_assert_eq(state, 1, 2);
        }
      } else {
        sentest(state, "three is three") {
          sentest_assert_eq(state, 3, 3);
        }
      }
      sentest(state, "two is two") {
        sentest_assert_eq(state, 2, 2);
      }
    }
  }
  int res = sentest_finish(state);
  fclose(config.output);
  return res;
}

static
char *read_entire_file(struct sentest_state *state, const char *path) {
  FILE *f = fopen(path, "rb");
  int seek_res = fseek(f, 0, SEEK_END);
  sentest_assert_eq(state, seek_res, 0);
  long fsize = ftell(f);
  rewind(f);
  char *string = malloc(fsize + 1);
  size_t read_amt = fread(string, 1, fsize, f);
  sentest_assert_eq(state, (long) read_amt, fsize);
  fclose(f);
  string[fsize] = 0;
  return string;
}

static
int count_substrings(char *a, char *b) {
  int res = 0;
  char *p = strstr(a, b);
  while (p != NULL) {
    res += 1;
    p = strstr(p + 1, b);
  }
  return res;
}

int main(void) {
  struct sentest_config config = {
    .output = stdout,
    .color = true,
    .filter_str = NULL,
    .junit = true,
  };
  struct sentest_state *state = sentest_start(config);

  sentest_group(state, "suite which passes") {
    const char *output_path = "out";
    run_example_suite(false, output_path);
    sentest_group(state, "output") {
      char *output = NULL;
      sentest(state, "writes the output file") {
        output = read_entire_file(state, output_path);
      }
      if (output == NULL) break;
      sentest(state, "printf ticks for passes") {
        sentest_assert_eq(state, count_substrings(output, "✓"), 3);
      }
      sentest(state, "doesn't print any crosses") {
        sentest_assert_eq(state, count_substrings(output, "x"), 0);
      }
      free(output);
    }
    remove(output_path);
    sentest_group(state, "junit output") {
      char *output = NULL;
      sentest(state, "writes the output file") {
        output = read_entire_file(state, output_path);
      }
      if (output == NULL) break;
      sentest(state, "printf ticks for passes") {
        sentest_assert_eq(state, count_substrings(output, "✓"), 3);
      }
      sentest(state, "doesn't print any crosses") {
        sentest_assert_eq(state, count_substrings(output, "x"), 0);
      }
      free(output);
    }
  }
  exit(sentest_finish(state));
}
