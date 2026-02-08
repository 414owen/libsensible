// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SENSIBLE_TEST_GOLDEN_H
#define SENSIBLE_TEST_GOLDEN_H

#include <stdbool.h>
#include <stddef.h>

struct sentest_golden_res {
  bool passed;
  // The following is only set on error

  char *expected;
  // Needs to be freed
  char *result;
  size_t first_different_line;
};

struct sentest_golden_res sentest_golden(char *input, char *transform(char*));

#endif
