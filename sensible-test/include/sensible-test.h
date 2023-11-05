// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SENSIBLE_TEST_H
#define SENSIBLE_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
# define SENTEST_TICK "OK"
# define SENTEST_CROSS "FAIL"
#else
# define SENTEST_TICK "✓"
# define SENTEST_CROSS "✕"
#endif

#include <stdbool.h>
#include <stdio.h>

#include "sensible-macros.h"

struct sentest_config {
  // Enable color output
  unsigned char color : 1;
  // Output file (usually stdout)
  FILE *output;
  // Only run tests with paths that contain this string
  const char *filter_str;
  // NULL for no junit xml output
  const char *junit_output_path;
};

#define sentest(state, desc)                                                   \
  for (sentest_start_internal(state, desc); sentest_test_should_continue(state); sentest_end_internal(state))

#define sentest_group(state, desc)                                             \
  for (sentest_group_start(state, desc); sentest_group_should_continue(state); sentest_group_end(state))

#define sentest_assert(state, b)                                               \
  sentest_assert_eq_internal(state, (b), __FILE__, __LINE__, #b, "true")

#define sentest_assert_eq(state, a, b)                                         \
  sentest_assert_eq_internal(state, (a) == (b), __FILE__, __LINE__, #a, #b)

#define sentest_assert_eq_fmt(state, fmt, a, b)                                \
  sentest_assertf(state, ((a) == (b)), #a " == " #b "\nGot: '%" fmt "', expected: '%" fmt "'", a, b)

#define sentest_assert_neq(state, a, b)                                        \
  sentest_assert_neq_internal(state, ((a) == (b)), __FILE__, __LINE__, #a, #b)

#define sentest_failf(state, fmt, ...) sentest_failf_internal(state, __FILE__, __LINE__, fmt, __VA_ARGS__)

#define sentest_assertf(state, assertion, fmt, ...) sentest_assertf_internal(state, assertion, __FILE__, __LINE__, fmt, __VA_ARGS__)

struct sentest_state;

senmac_public struct sentest_state *sentest_start(struct sentest_config config);

senmac_public void sentest_group_start(struct sentest_state *state, char *name);
senmac_public bool sentest_group_should_continue(struct sentest_state *state);
senmac_public void sentest_group_end(struct sentest_state *state);

senmac_public void sentest_end_internal(struct sentest_state *state);
senmac_public bool sentest_test_should_continue(struct sentest_state *state);
senmac_public void sentest_start_internal(struct sentest_state *state, char *name);

// Returns true on assertion failue
senmac_public bool sentest_assertf_internal(struct sentest_state *state, bool cond, const char *file, size_t line, const char *fmt, ...);
senmac_public void sentest_failf_internal(struct sentest_state *state, const char *file, size_t line, const char *fmt, ...);

// Returns true if equal, false if not equal
senmac_public bool sentest_assert_eq_internal(struct sentest_state *state, bool is_eq, const char *file, size_t line, char *a, char *b);
senmac_public bool sentest_assert_neq_internal(struct sentest_state *state, bool is_eq, const char *file, size_t line, char *a, char *b);

senmac_public int sentest_finish(struct sentest_state *state);

#ifdef __cplusplus
}
#endif

#endif
