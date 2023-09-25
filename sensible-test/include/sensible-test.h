// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stdbool.h>
#include <stdio.h>

struct sentest_config {
  unsigned char junit : 1;
  unsigned char color : 1;
  FILE *output;
  char *filter_str;
};

#define sentest(state, desc)                                                   \
  for (sentest_start_internal(state, desc); sentest_test_should_continue(state); sentest_end_internal(state))

#define sentest_group(state, desc)                                             \
  for (sentest_group_start(state, desc); sentest_group_should_continue(state); sentest_group_end(state))

#define sentest_assert(state, b)                                               \
  if (!(b))                                                                    \
    sentest_fail_eq(state, #b, "true");

#define sentest_assert_eq(state, a, b)                                         \
  if ((a) != (b))                                                              \
    sentest_fail_eq(state, #a, #b);

#define sentest_assert_eq_fmt(state, a, b, fmt)                                \
  if ((a) != (b)) {                                                            \
    sentest_failf(state, #a " != " #b "\nExpected: '" fmt "', got: '" fmt "'", a, b);  \
  }

#define sentest_assert_neq(state, a, b)                                        \
  if ((a) == (b))                                                              \
    sentest_fail_eq(state, #a, #b);

#define sentest_failf(state, fmt, ...) failf_(state, "In %s on line %zu: " fmt, __FILE__, __LINE__, __VA_ARGS__)

struct sentest_state;

struct sentest_state *sentest_start(struct sentest_config config);

void sentest_group_end(struct sentest_state *state);
void sentest_group_start(struct sentest_state *state, char *name);

void sentest_end_internal(struct sentest_state *state);
void sentest_start_internal(struct sentest_state *state, char *name);

void sentest_fail_eq(struct sentest_state *state, char *a, char *b);

bool sentest_matches(struct sentest_state *state, char *test_name);

bool sentest_group_should_continue(struct sentest_state *state);
bool sentest_test_should_continue(struct sentest_state *state);

int sentest_finish(struct sentest_state *state);
