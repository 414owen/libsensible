#include "sensible-test-golden.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define STATIC_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

static const char SEPARATOR[] = "\n--------\n";

// Returns the first line at which the strings weren't equal
static bool strs_eq(size_t *res, char *as, char *bs) {
  size_t line = 0;
  while (true) {
    char a = *as++;
    char b = *bs++;
    if (a == '\n') line++;
    if (a != b) {
      *res = line;
      return false;
    }
  }
  return true;
}

struct sentest_golden_res sentest_golden(char *input, char *transform(char*)) {
  char *sep_start = strstr(input, SEPARATOR);
  char backup = *sep_start;
  *sep_start = '\0';
  char *expected = sep_start + STATIC_LEN(SEPARATOR) - 1;
  size_t first_noneq_line;
  char *output = transform(input);
  bool success = strs_eq(&first_noneq_line, output, expected);
  if (success) {
    free(output);
  }
  struct sentest_golden_res res = {
    .passed = success,
    .result = output,
    .expected = expected,
    .first_different_line = first_noneq_line
  };
  *sep_start = backup;
  return res;
}
