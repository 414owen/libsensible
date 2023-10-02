// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../src/sensible-test.h"

// This is the test suite we're testing the output of.
static
int run_example_suite(bool has_failure, const char *out_path, const char *junit_out_path) {
  struct sentest_config config = {
    .output = fopen(out_path, "w"),
    .color = false,
    .filter_str = NULL,
    .junit_output_path = junit_out_path,
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
  if (sentest_assert_eq(state, seek_res, 0)) return NULL;
  long fsize = ftell(f);
  if (sentest_assert_neq(state, fsize, -1)) return NULL;
  rewind(f);
  char *string = malloc(fsize + 1);
  if (sentest_assert_neq(state, string, NULL)) return NULL;
  size_t read_amt = fread(string, 1, fsize, f);
  if (sentest_assert_eq(state, (long) read_amt, fsize)) return NULL;
  int close_res = fclose(f);
  if (sentest_assert_eq(state, close_res, 0)) return NULL;
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

// Known caveat: Also matches strings that have 'tag_name' as a prefix
static
void assert_tags_match(struct sentest_state *state, char *xml, char *tag_name) {
  size_t len = strlen(tag_name);
  char *start_tag_str = malloc(len * 2 + 6);
  char *end_tag_str = start_tag_str + sprintf(start_tag_str, "<%s", tag_name) + 1;
  sprintf(end_tag_str, "</%s", tag_name);
  int starts = count_substrings(xml, start_tag_str);
  int ends = count_substrings(xml, end_tag_str);
  if (starts != ends) {
    sentest_failf(state, "Different number of start and end tags for tag %s: %d vs %d", tag_name, starts, ends);
  }
  free(start_tag_str);
}

void run_sensible_test_suite(struct sentest_state *state) {
  sentest_group(state, "suite which passes") {
    const char *output_path = "pass-suite.out";
    const char *junit_out_path = "pass-junit.xml";
    run_example_suite(false, output_path, junit_out_path);
    sentest_group(state, "output") {
      char *output = NULL;
      sentest(state, "writes the output file") {
        output = read_entire_file(state, output_path);
      }
      if (output == NULL) break;
      sentest(state, "prints ticks for passes") {
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
        output = read_entire_file(state, junit_out_path);
      }
      if (output == NULL) break;
      sentest(state, "contains balanced <testsuites>") {
        assert_tags_match(state, output, "testsuites");
      }
      sentest(state, "contains balanced <testsuite>s") {
        assert_tags_match(state, output, "testsuite");
      }
      sentest(state, "contains balanced <testcase>s") {
        assert_tags_match(state, output, "testcase");
      }
      free(output);
    }
    remove(junit_out_path);
  }

  sentest_group(state, "suite which fails") {
    const char *output_path = "fail-suite.stdout";
    const char *junit_out_path = "fail-junit.xml";
    run_example_suite(true, output_path, junit_out_path);
    sentest_group(state, "output") {
      char *output = NULL;
      sentest(state, "writes the output file") {
        output = read_entire_file(state, output_path);
      }
      if (output == NULL) break;
      sentest(state, "prints ticks for passes") {
        sentest_assert_eq(state, count_substrings(output, "✓"), 2);
      }
      sentest(state, "prints crosses for failures") {
        sentest_assert_eq(state, count_substrings(output, "✕"), 1);
      }
      free(output);
    }
    sentest_group(state, "junit output") {
      char *output = NULL;
      sentest(state, "writes the output file") {
        output = read_entire_file(state, junit_out_path);
      }
      if (output == NULL) break;
      sentest(state, "contains balanced <testsuites>") {
        assert_tags_match(state, output, "testsuites");
      }
      sentest(state, "contains balanced <testsuite>s") {
        assert_tags_match(state, output, "testsuite");
      }
      sentest(state, "contains balanced <testcase>s") {
        assert_tags_match(state, output, "testcase");
      }
      free(output);
    }
    remove(junit_out_path);
  }
}
