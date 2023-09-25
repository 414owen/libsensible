// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sensible-test.h"

// ANSI escape codes

#define TERM_ESCAPE "\x1B"
#define TERM_RED TERM_ESCAPE "[0;31m"
#define TERM_GREEN TERM_ESCAPE "[0;32m"
#define TERM_RESET TERM_ESCAPE "[0m"


// Useful types

enum sentest_action {
  GROUP_ENTER,
  TEST_ENTER,
  GROUP_LEAVE,
  TEST_LEAVE,
  TEST_FAIL,
};

struct test_aggregate {
  size_t tests;
  size_t failures;
};

struct sentest_string_arr {
  char **data;
  size_t length;
};


// Vectors

#define VEC_INITIAL_CAPACITY 8

struct sentest_vec_test_aggregates {
  struct test_aggregate *data;
  size_t length;
  size_t capacity;
};

struct sentest_vec_test_action {
  enum sentest_action *data;
  size_t length;
  size_t capacity;
};

struct sentest_vec_string {
  char **data;
  size_t length;
  size_t capacity;
};

struct sentest_failure {
  struct sentest_string_arr path;
  char *reason;
};

struct sentest_vec_failure {
  struct sentest_failure *data;
  size_t length;
  size_t capacity;
};


// Main state

struct sentest_state {
  struct sentest_config config;

  // Current path, eg. "ints/can be added"
  struct sentest_vec_string path;
  uint32_t tests_passed;
  uint32_t tests_run;
  clock_t start_time;
  clock_t end_time;
  // TODO name this better
  char *current_name;
  struct sentest_vec_failure failures;
  struct sentest_vec_test_action actions;
  uint8_t  current_failed : 1;
  uint8_t in_test : 1;
  // used in the implementation of the for loop that
  // test_group uses
  uint8_t should_exit_group : 1;
  struct sentest_vec_string strs;
  const char *filter_str;
};


static const uint8_t TEST_INDENT = 2;

static
struct sentest_vec_string sentest_vec_string_new(void) {
  struct sentest_vec_string res = {
    .data = malloc(sizeof(char*) * VEC_INITIAL_CAPACITY),
    .length = 0,
    .capacity = VEC_INITIAL_CAPACITY,
  };
  return res;
}

static
void sentest_vec_string_push(struct sentest_vec_string *vec, char *string) {
  if (vec->length == vec->capacity) {
    vec->capacity += vec->capacity >> 1;
    vec->data = realloc(vec->data, sizeof(char*) * vec->capacity);
  }
  vec->data[vec->length++] = string;
}

static
struct sentest_vec_test_aggregates sentest_vec_test_aggregate_new(void) {
  struct sentest_vec_test_aggregates res = {
    .data = malloc(sizeof(char*) * VEC_INITIAL_CAPACITY),
    .length = 0,
    .capacity = VEC_INITIAL_CAPACITY,
  };
  return res;
}

static
void sentest_vec_test_aggregate_push(struct sentest_vec_test_aggregates *vec, const struct test_aggregate test_aggregate) {
  if (vec->length == vec->capacity) {
    vec->capacity += vec->capacity >> 1;
    vec->data = realloc(vec->data, sizeof(char*) * vec->capacity);
  }
  vec->data[vec->length++] = test_aggregate;
}

static
struct sentest_vec_test_action sentest_vec_test_action_new(void) {
  struct sentest_vec_test_action res = {
    .data = malloc(sizeof(enum sentest_action) * VEC_INITIAL_CAPACITY),
    .length = 0,
    .capacity = VEC_INITIAL_CAPACITY,
  };
  return res;
}

static
void sentest_vec_test_action_push(struct sentest_vec_test_action *vec, enum sentest_action action) {
  if (vec->length == vec->capacity) {
    vec->capacity += vec->capacity >> 1;
    vec->data = realloc(vec->data, sizeof(char*) * vec->capacity);
  }
  vec->data[vec->length++] = action;
}

static
struct sentest_vec_failure sentest_vec_failure_new(void) {
  struct sentest_vec_failure res = {
    .data = malloc(sizeof(struct sentest_failure) * VEC_INITIAL_CAPACITY),
    .length = 0,
    .capacity = VEC_INITIAL_CAPACITY,
  };
  return res;
}

static
void sentest_vec_failure_push(struct sentest_vec_failure *vec, struct sentest_failure failure) {
  if (vec->length == vec->capacity) {
    vec->capacity += vec->capacity >> 1;
    vec->data = realloc(vec->data, sizeof(char*) * vec->capacity);
  }
  vec->data[vec->length++] = failure;
}

static
void sentest_print_depth_indent(struct sentest_state *state) {
  for (uint8_t i = 0; i < state->path.length * TEST_INDENT; i++) {
    putc(' ', state->config.output);
  }
}

void sentest_group_start(struct sentest_state *state, char *name) {
  assert(!state->in_test);
  sentest_print_depth_indent(state);
  fprintf(state->config.output, "%s\n", name);
  sentest_vec_string_push(&state->path, name);
  state->should_exit_group = false;
  if (state->config.junit_output_path) {
    sentest_vec_test_action_push(&state->actions, GROUP_ENTER);
    sentest_vec_string_push(&state->strs, name);
  }
}

void sentest_group_end(struct sentest_state *state) {
  assert(!state->in_test);
  state->path.length--;
  state->should_exit_group = true;
  if (state->config.junit_output_path) {
    sentest_vec_test_action_push(&state->actions, GROUP_LEAVE);
  }
}

// Static to avoid someone failing with a non-alloced string
static
void sentest_fail_with(struct sentest_state *state, char *reason) {
  assert(state->in_test);
  state->current_failed = true;
  size_t path_part_amount = state->path.length + 1;
  struct sentest_failure f = {
    .path = {
      .length = path_part_amount,
      .data = malloc(sizeof(char*) * path_part_amount),
    },
    .reason = reason
  };
  for (size_t i = 0; i < state->path.length; i++) {
    f.path.data[i] = state->path.data[i];
  }
  f.path.data[path_part_amount - 1] = state->current_name;
  sentest_vec_failure_push(&state->failures, f);
  if (state->config.junit_output_path) {
    sentest_vec_test_action_push(&state->actions, TEST_FAIL);
  }
}

static
void sentest_vfailf_internal(struct sentest_state *state, const char *file, size_t line, const char *fmt, va_list ap) {
  const char *fmt1 = "In %s line %zu: ";
  int size = snprintf(NULL, 0, fmt1, file, line);
  size += vsnprintf(NULL, 0, fmt, ap);
  char *buf = malloc(size + 1);
  char *pos = buf;
  pos += sprintf(buf, fmt1, file, line);
  pos += vsprintf(pos, fmt, ap);
  *pos = '\0';
  sentest_fail_with(state, buf);
}

void sentest_failf_internal(struct sentest_state *state, const char *file, size_t line, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  sentest_vfailf_internal(state, file, line, fmt, ap);
  va_end(ap);
}

bool sentest_assertf_internal(struct sentest_state *state, bool cond, const char *file, size_t line, const char *fmt, ...) {
  if (cond) { return false; }
  va_list ap;
  va_start(ap, fmt);
  sentest_vfailf_internal(state, file, line, fmt, ap);
  va_end(ap);
  return true;
}

bool sentest_assert_eq_internal(struct sentest_state *state, bool is_equal, const char *file, size_t line, char *a, char *b) {
  if (is_equal) return false;
  sentest_failf_internal(state, file, line, "Assert failed: '%s' == '%s'", a, b);
  return true;
}

bool sentest_assert_neq_internal(struct sentest_state *state, bool is_equal, const char *file, size_t line, char *a, char *b) {
  if (!is_equal) return false;
  sentest_failf_internal(state, file, line, "Assert failed: '%s' != '%s'", a, b);
  return true;
}

void sentest_start_internal(struct sentest_state *state, char *name) {
  assert(!state->in_test);
  assert(state->path.length > 0);
  sentest_vec_string_push(&state->path, name);
  state->in_test = true;
  sentest_print_depth_indent(state);
  fputs(name, state->config.output);
  putc(' ', state->config.output);
  // we want to know what test is being run when we crash
  fflush(state->config.output);
  state->current_name = name;
  state->current_failed = false;
  state->in_test = true;
  if (state->config.junit_output_path) {
    sentest_vec_test_action_push(&state->actions, TEST_ENTER);
    sentest_vec_string_push(&state->strs, name);
  }
}

static
const char *sentest_color(struct sentest_state *state, char *color) {
  return state->config.color ? color : "";
}

void sentest_end_internal(struct sentest_state *state) {
  fprintf(state->config.output, "%s%s%s\n",
    sentest_color(state, state->current_failed ? TERM_RED : TERM_GREEN),
    state->current_failed ? "✕" : "✓",
    sentest_color(state, TERM_RESET));
  if (!state->current_failed)
    state->tests_passed++;
  state->path.length--;
  state->tests_run++;
  state->in_test = false;
  if (state->config.junit_output_path) {
    sentest_vec_test_action_push(&state->actions, TEST_LEAVE);
  }
}

struct sentest_state *sentest_start(struct sentest_config config) {
  struct sentest_state *res = (struct sentest_state*) malloc(sizeof(struct sentest_state));
  struct sentest_state state = {
    .config = config,
    .path = sentest_vec_string_new(),
    .tests_passed = 0,
    .tests_run = 0,
    .start_time = clock(),
    .end_time = 0,
    .current_name = NULL,
    .failures = sentest_vec_failure_new(),
    .actions = sentest_vec_test_action_new(),
    .current_failed = false,
    .in_test = false,
    .strs = sentest_vec_string_new(),
    .filter_str = config.filter_str,
  };
  if (state.config.output == NULL) {
    state.config.output = stdout;
  }
  *res = state;
  return res;
}

static
void sentest_print_test_path(struct sentest_string_arr v, FILE *out) {
  if (v.length == 0)
    return;
  fputs(v.data[0], out);
  for (size_t i = 1; i < v.length; i++) {
    putc('/', out);
    fputs(v.data[i], out);
  }
}

static
char *sentest_print_test_path_string(struct sentest_vec_string v) {
  // 1 for the null terminator
  size_t length = 1;

  if (v.length == 0) {
    char *res = calloc(1, 1);
    return res;  }

  length += strlen(v.data[0]);

  for (size_t i = 1; i < v.length; i++) {
    length += 1 + strlen(v.data[i]);
  }

  char *res = malloc(length);

  size_t ind = 0;
  strcpy(&res[ind], v.data[0]);
  ind += strlen(v.data[0]);
  for (size_t i = 1; i < v.length; i++) {
    res[ind++] = '/';
    strcpy(&res[ind], v.data[i]);
    ind += strlen(v.data[i]);
  }

  return res;
}

static
void sentest_print_failure(struct sentest_state *state, struct sentest_failure f) {
  fprintf(state->config.output,
    "\n%sFAILED%s: ",
    sentest_color(state, TERM_RED),
    sentest_color(state, TERM_RESET));
  sentest_print_test_path(f.path, state->config.output);
  putc('\n', state->config.output);
  fputs(f.reason, state->config.output);
  putc('\n', state->config.output);
  fflush(state->config.output);
}

static
void sentest_print_failures(struct sentest_state *state) {
  for (size_t i = 0; i < state->failures.length; i++) {
    sentest_print_failure(state, state->failures.data[i]);
  }
}

void sentest_write_results(struct sentest_state *state) {
  if (!state->config.junit_output_path) { return; }
  FILE *f = fopen(state->config.junit_output_path, "w");

  bool failed = false;
  struct sentest_vec_test_aggregates agg_stack = sentest_vec_test_aggregate_new();
  struct sentest_vec_test_aggregates aggs = sentest_vec_test_aggregate_new();
  struct test_aggregate current = {.tests = 0, .failures = 0};

  for (size_t i = 0; i < state->actions.length; i++) {
    enum sentest_action action = state->actions.data[state->actions.length - 1 - i];
    switch (action) {
      case GROUP_LEAVE:
        sentest_vec_test_aggregate_push(&agg_stack, current);
        current.tests = 0;
        current.failures = 0;
        break;
      case GROUP_ENTER: {
        struct test_aggregate inner;
        inner = agg_stack.data[--agg_stack.length];
        sentest_vec_test_aggregate_push(&aggs, current);
        current.tests += inner.tests;
        current.failures += inner.failures;
        break;
      }
      case TEST_LEAVE:
        current.tests++;
        failed = false;
        break;
      case TEST_FAIL:
        failed = true;
        break;
      case TEST_ENTER:
        if (failed)
          current.failures++;
        failed = false;
        break;
    }
  }

  free(agg_stack.data);
  struct sentest_vec_string class_path = sentest_vec_string_new();

  clock_t elapsed = state->end_time - state->start_time;

  fprintf(f,
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          "<testsuites name=\"libsensible tests\" tests=\"%zu\" disabled=\"0\" "
          "errors=\"0\" failures=\"%zu\" time=\"%ld\">\n",
          current.tests,
          current.failures,
          // TODO fix this
          elapsed);

  size_t agg_ind = aggs.length - 1;
  size_t str_ind = 0;
  size_t fail_ind = 0;
  size_t depth = 1;

  for (size_t i = 0; i < state->actions.length; i++) {
    enum sentest_action action = state->actions.data[i];
    switch (action) {
      case GROUP_LEAVE:
        // TODO investigate: do we need depth?
        depth--;
        class_path.length--;
        break;
      default:
        break;
    }

    for (size_t j = 0; j < depth; j++) {
      fputs("  ", f);
    }

    switch (action) {
      case GROUP_ENTER: {
        struct test_aggregate agg = aggs.data[agg_ind];
        agg_ind--;
        char *str = state->strs.data[str_ind];
        str_ind++;
        fprintf(f,
                "<testsuite name=\"%s\" tests=\"%zu\" failures=\"%zu\">\n",
                str,
                agg.tests,
                agg.failures);
        sentest_vec_string_push(&class_path, str);
        depth++;
        break;
      }
      case TEST_ENTER:
        fprintf(f,
                "<testcase name=\"%s\" classname=\"",
                state->strs.data[str_ind]);
        str_ind++;
        for (size_t i = 0; i < class_path.length; i++) {
          if (i > 0)
            putc('/', f);
          fputs(class_path.data[i], f);
        }
        fputs("\">\n", f);
        break;
      case TEST_LEAVE:
        fputs("</testcase>\n", f);
        break;
      case GROUP_LEAVE:
        fputs("</testsuite>\n", f);
        break;
      case TEST_FAIL:
        fprintf(f,
                "<failure message=\"Assertion failure\">%s</failure>\n",
                state->failures.data[fail_ind].reason);
        fail_ind--;
        break;
    }
  }
  fputs("</testsuites>\n", f);

  fclose(f);
  free(aggs.data);
  free(class_path.data);
}

static
bool sentest_matches(struct sentest_state *restrict state) {
  if (state->filter_str == NULL) {
    return true;
  }
  // TODO remove this, by treating the current path as a stack allocator
  // or alternatively, match on the vec_string directly
  char *path_string = sentest_print_test_path_string(state->path);
  state->path.length--;
  bool res = strstr(path_string, state->filter_str) != NULL;
  free(path_string);
  return res;
}

bool sentest_test_should_continue(struct sentest_state *restrict state) {
  return state->in_test && sentest_matches(state);
}

bool sentest_group_should_continue(struct sentest_state *restrict state) {
  bool res = state->should_exit_group;
  if (res) {
    state->should_exit_group = false;
  }
  return !res;
}

int sentest_finish(struct sentest_state *state) {
  state->end_time = clock();
  sentest_print_failures(state);
  sentest_write_results(state);
  int res = state->failures.length > 0 ? 1 : 0;
  fflush(state->config.output);
  free(state);
  return res;
}
