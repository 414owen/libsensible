// Copyright 2023 The piq Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#define test_assert(state, b)                                                  \
  if (!(b))                                                                    \
    test_fail_eq(state, #b, "true");

#define test_assert_eq(state, a, b)                                            \
  if ((a) != (b))                                                              \
    test_fail_eq(state, #a, #b);

#define test_assert_eq_fmt(state, a, b, fmt)                                   \
  if ((a) != (b)) {                                                            \
    failf(state, #a " != " #b "\nExpected: '" fmt "', got: '" fmt "'", a, b);  \
  }

#define test_assert_eq_fmt_f(state, a, b, fmt, transform)                      \
  if ((a) != (b)) {                                                            \
    failf(state,                                                               \
          #a " != " #b "\nExpected: '" fmt "', got: '" fmt "'",                \
          (transform)(a),                                                      \
          (transform)(b));                                                     \
  }

#define test_assert_eq_fmt_a(state, a, b, fmt, transform)                      \
  if ((a) != (b)) {                                                            \
    failf(state,                                                               \
          #a " != " #b "\nExpected: '" fmt "', got: '" fmt "'",                \
          (transform)[a],                                                      \
          (transform)[b]);                                                     \
  }

#define test_assert_neq(state, a, b)                                           \
  if ((a) == (b))                                                              \
    test_fail_eq(state, #a, #b);

#define failf(state, fmt, ...) failf_(state, "In %s on line %zu: " fmt, __FILE__, __LINE__, __VA_ARGS__)

struct test_config {
  bool junit;
  char *filter_str;
};

struct vec_string {
  char **data;
  size_t length;
  size_t capacity;
};

struct string_arr {
  char **data;
  size_t length;
};

struct failure {
  struct string_arr path;
  char *reason;
};

struct vec_failure {
  struct failure *data;
  size_t length;
  size_t capacity;
};

// TODO prefix these
enum test_action {
  GROUP_ENTER,
  TEST_ENTER,
  GROUP_LEAVE,
  TEST_LEAVE,
  TEST_FAIL,
};

struct vec_test_action {
  enum test_action *data;
  size_t length;
  size_t capacity;
};

struct test_state {
  // Settings
  struct test_config config;

  // Current path, eg. "ints/can be added"
  struct vec_string path;
  uint32_t tests_passed;
  uint32_t tests_run;
  clock_t start_time;
  clock_t end_time;
  // TODO name this better
  char *current_name;
  struct vec_failure failures;
  struct vec_test_action actions;
  uint8_t  current_failed : 1;
  uint8_t in_test : 1;
  struct vec_string strs;
  const char *filter_str;
};

#define test_start(state, desc)                                              \
  for (test_start_internal(state, __test_name); state->in_test; test_end_internal(state)) \

struct test_state test_state_new(struct test_config config);
void test_state_finalize(struct test_state *state);

void test_group_end(struct test_state *state);
void test_group_start(struct test_state *state, char *name);

void test_end_internal(struct test_state *state);
void test_start_internal(struct test_state *state, char *name);

void test_fail_eq(struct test_state *state, char *a, char *b);

void print_failures(struct test_state *state);

void write_test_results(struct test_state *state);
bool test_matches(struct test_state *state, char *test_name);
