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

#define MAX(a, b) ((a) > (b) ? (a) : (b))

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

struct sentest_aggregate {
  size_t tests;
  size_t failures;
};

struct sentest_string_arr {
  char *data;
  size_t length;
};


// Vectors

#define VEC_INITIAL_CAPACITY 8

struct sentest_vec_test_aggregates {
  struct sentest_aggregate *data;
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

struct sentest_vec_char {
  char *data;
  size_t length;
  size_t capacity;
};

struct sentest_vec_size_t {
  size_t *data;
  size_t length;
  size_t capacity;
};


// Main state

struct sentest_state {
  struct sentest_config config;

  // Current path, eg. "ints/can be added"
  struct sentest_vec_char path;

  struct sentest_vec_size_t path_seg_lengths;
  uint32_t tests_passed;
  uint32_t tests_run;
  clock_t start_time;
  clock_t end_time;
  // TODO name this better
  char *current_name;
  struct sentest_vec_test_action actions;
  uint8_t  current_failed : 1;
  uint8_t in_test : 1;
  // used in the implementation of the for loop that
  // test_group uses
  uint8_t should_exit_group : 1;
  struct sentest_vec_char strs;
  struct sentest_vec_size_t str_starts;
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
struct sentest_vec_size_t sentest_vec_size_t_new(void) {
  struct sentest_vec_size_t res = {
    .data = malloc(sizeof(size_t) * VEC_INITIAL_CAPACITY),
    .length = 0,
    .capacity = VEC_INITIAL_CAPACITY,
  };
  return res;
}

static
void sentest_vec_size_t_push(struct sentest_vec_size_t *vec, size_t elem) {
  if (vec->length == vec->capacity) {
    vec->capacity += vec->capacity >> 1;
    vec->data = realloc(vec->data, sizeof(size_t) * vec->capacity);
  }
  vec->data[vec->length++] = elem;
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
    vec->data = realloc(vec->data, sizeof(enum sentest_action) * vec->capacity);
  }
  vec->data[vec->length++] = action;
}

static
struct sentest_vec_char sentest_vec_char_new(void) {
  struct sentest_vec_char res = {
    .data = malloc(VEC_INITIAL_CAPACITY),
    .length = 0,
    .capacity = VEC_INITIAL_CAPACITY,
  };
  res.data[res.length++] = '\0';
  return res;
}

// Create a path by composing existing path with a new segment
static
void sentest_vec_char_push_pathseg(struct sentest_vec_char *vec, const char *str, size_t len) {
  if (vec->length + len + 1 >= vec->capacity) {
    vec->capacity = MAX(vec->capacity + (vec->capacity >> 1), vec->length + len + 1);
    vec->data = realloc(vec->data, vec->capacity);
  }
  // Replace null-terminator with '/'
  vec->data[vec->length - 1] = '/';
  strcpy(&vec->data[vec->length], str);
  vec->length += 1 + len;
}

// Push a string into a buffer of strings
static
void sentest_vec_char_push_string(struct sentest_vec_char *vec, const char *str, size_t length) {
  if (vec->length + length >= vec->capacity) {
    vec->capacity = MAX(vec->capacity + (vec->capacity >> 1), vec->length + length + 1);
    vec->data = realloc(vec->data, vec->capacity);
  }
  strcpy(&vec->data[vec->length], str);
  vec->length += 1 + length;
}

static
size_t sentest_depth(struct sentest_state *state) {
  return state->path_seg_lengths.length;
}

static
void sentest_print_depth_indent(struct sentest_state *state) {
  for (uint32_t i = 0; i < sentest_depth(state) * TEST_INDENT; i++) {
    putc(' ', state->config.output);
  }
}

static
void sentest_push_path(struct sentest_state *state, const char *segment, size_t length) {
  sentest_vec_char_push_pathseg(&state->path, segment, length);
  sentest_vec_size_t_push(&state->path_seg_lengths, length);
}

static
void sentest_push_string(struct sentest_state *state, const char *str, size_t length) {
  sentest_vec_size_t_push(&state->str_starts, state->strs.length);
  sentest_vec_char_push_string(&state->strs, str, length);
}

static
void sentest_pop_path(struct sentest_state *state) {
  assert(state->path_seg_lengths.length > 0);
  size_t seg_length = state->path_seg_lengths.data[--state->path_seg_lengths.length];
  state->path.length -= seg_length + 1;
  state->path.data[state->path.length - 1] = '\0';
}

void sentest_group_start(struct sentest_state *state, char *name) {
  assert(!state->in_test);
  sentest_print_depth_indent(state);
  size_t length = fprintf(state->config.output, "%s\n", name) - 1;
  fflush(state->config.output);
  sentest_push_path(state, name, length);
  state->should_exit_group = false;
  sentest_vec_test_action_push(&state->actions, GROUP_ENTER);
  sentest_push_string(state, name, length);
}

void sentest_group_end(struct sentest_state *state) {
  assert(!state->in_test);
  sentest_pop_path(state);
  state->should_exit_group = true;
  sentest_vec_test_action_push(&state->actions, GROUP_LEAVE);
}

// Static to avoid someone failing with a non-alloced string
static
void sentest_fail_with(struct sentest_state *state, char *reason, size_t length) {
  assert(state->in_test);
  state->current_failed = true;
  sentest_push_string(state, reason, length);
  sentest_vec_test_action_push(&state->actions, TEST_FAIL);
}

static
void sentest_vfailf_internal(struct sentest_state *state, const char *file, size_t line, const char *fmt, va_list ap) {
  const char *fmt1 = "In %s line %zu: ";
  size_t size = snprintf(NULL, 0, fmt1, file, line);
  {
    va_list ap1;
    va_copy(ap1, ap);
    size += vsnprintf(NULL, 0, fmt, ap1);
  }
#define MAX_STACK_BUF 256
  char stack_buf[MAX_STACK_BUF];
  char *buf = size + 1 > MAX_STACK_BUF ? malloc(size + 1) : stack_buf;
  {
    char *pos = buf;
    pos += sprintf(buf, fmt1, file, line);
    pos += vsprintf(pos, fmt, ap);
    *pos = '\0';
  }
  sentest_fail_with(state, buf, size);
  if (size + 1 > MAX_STACK_BUF) free(buf);

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
  assert(sentest_depth(state) > 0);
  sentest_print_depth_indent(state);
  size_t length = fprintf(state->config.output, "%s ", name) - 1;
  sentest_push_path(state, name, length);
  state->in_test = true;
  // we want to know what test is being run when we crash
  fflush(state->config.output);
  state->current_name = name;
  state->current_failed = false;
  state->in_test = true;
  sentest_vec_test_action_push(&state->actions, TEST_ENTER);
  sentest_push_string(state, name, length);
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
  sentest_pop_path(state);
  state->tests_run++;
  state->in_test = false;
  sentest_vec_test_action_push(&state->actions, TEST_LEAVE);
}

struct sentest_state *sentest_start(struct sentest_config config) {
  struct sentest_state *res = (struct sentest_state*) malloc(sizeof(struct sentest_state));
  struct sentest_state state = {
    .config = config,
    .path = sentest_vec_char_new(),
    .path_seg_lengths = sentest_vec_size_t_new(),
    .tests_passed = 0,
    .tests_run = 0,
    .start_time = clock(),
    .end_time = 0,
    .current_name = NULL,
    .actions = sentest_vec_test_action_new(),
    .current_failed = false,
    .in_test = false,
    .strs = sentest_vec_char_new(),
    .str_starts = sentest_vec_size_t_new(),
    .filter_str = config.filter_str,
  };
  if (state.config.output == NULL) {
    state.config.output = stdout;
  }
  *res = state;
  return res;
}

static
bool sentest_print_failures(struct sentest_state *state) {
  bool had_failure = false;
  size_t str_ind = 0;
  for (size_t i = 0; i < state->actions.length; i++) {
    enum sentest_action action = state->actions.data[i];
    switch (action) {
      case TEST_LEAVE:
      case GROUP_LEAVE:
        sentest_pop_path(state);
        break;
      case GROUP_ENTER:
      case TEST_ENTER: {
        char *segment = &state->strs.data[state->str_starts.data[str_ind++]];
        sentest_push_path(state, segment, strlen(segment));
        break;
      }
      case TEST_FAIL: {
        had_failure = true;
        char *reason = &state->strs.data[state->str_starts.data[str_ind++]];
        fprintf(state->config.output,
          "\n%sFAILED%s: ",
          sentest_color(state, TERM_RED),
          sentest_color(state, TERM_RESET));
        fputs(state->path.data, state->config.output);
        putc('\n', state->config.output);
        fputs(reason, state->config.output);
        putc('\n', state->config.output);
        fflush(state->config.output);
        break;
      }
    }
  }
  return had_failure;
}

void sentest_write_results(struct sentest_state *state) {
  if (!state->config.junit_output_path) { return; }
  FILE *f = fopen(state->config.junit_output_path, "w");

  size_t max_depth = 0;
  size_t num_groups = 0;
  {
    size_t depth = 0;
    for (size_t i = 0; i < state->actions.length; i++) {
      enum sentest_action action = state->actions.data[i];
      switch (action) {
        case GROUP_LEAVE:
          max_depth = MAX(max_depth, depth);
          depth--;
          break;
        case GROUP_ENTER:
          num_groups++;
          depth++;
          break;
        default:
          break;
      }
    }
  }

  // Traverse actions in reverse, calculating aggregates
  struct sentest_aggregate *aggs = malloc(sizeof(struct sentest_aggregate) * num_groups);
  struct sentest_aggregate current = {.tests = 0, .failures = 0};
  {
    struct sentest_aggregate *agg_stack = malloc(sizeof(struct sentest_aggregate) * max_depth);
    size_t agg_stack_ind = 0;
    size_t aggs_ind = 0;
    bool failed = false;

    for (size_t i = 0; i < state->actions.length; i++) {
      enum sentest_action action = state->actions.data[state->actions.length - 1 - i];
      switch (action) {
        case GROUP_LEAVE:
          agg_stack[agg_stack_ind++] = current;
          current.tests = 0;
          current.failures = 0;
          break;
        case GROUP_ENTER: {
          struct sentest_aggregate inner;
          inner = agg_stack[--agg_stack_ind];
          aggs[aggs_ind++] = current;
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
    free(agg_stack);
  }

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

  size_t agg_ind = num_groups - 1;
  size_t str_ind = 0;
  size_t fail_ind = 0;
  size_t depth = 1;
  char *all_depths = malloc((1 + max_depth) * TEST_INDENT + 1);
  for (size_t i = 0; i < (1 + max_depth) * TEST_INDENT; i++) {
    all_depths[i] = ' ';
  }
  all_depths[(1 + max_depth) * TEST_INDENT] = '\0';

  for (size_t i = 0; i < state->actions.length; i++) {
    enum sentest_action action = state->actions.data[i];
    if (action == GROUP_LEAVE) {
        // TODO investigate: do we need depth?
        depth--;
        class_path.length--;
    }

    fwrite(all_depths, 1, TEST_INDENT * depth, f);

    switch (action) {
      case GROUP_ENTER: {
        struct sentest_aggregate agg = aggs[agg_ind--];
        char *str = &state->strs.data[state->str_starts.data[str_ind++]];
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
                &state->strs.data[state->str_starts.data[str_ind++]]);
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
                &state->strs.data[state->str_starts.data[str_ind++]]);
        fail_ind--;
        break;
    }
  }
  fputs("</testsuites>\n", f);

  fclose(f);
  free(aggs);
  free(class_path.data);
  free(all_depths);
}

static
bool sentest_matches(struct sentest_state *restrict state) {
  if (state->filter_str == NULL) {
    return true;
  }
  // TODO remove this, by treating the current path as a stack allocator
  // or alternatively, match on the vec_string directly
  bool res = strstr(state->path.data, state->filter_str) != NULL;
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
  bool had_failure = sentest_print_failures(state);
  sentest_write_results(state);
  free(state->actions.data);
  free(state->path.data);
  free(state->path_seg_lengths.data);
  free(state->strs.data);
  free(state->str_starts.data);
  int res = had_failure > 0 ? 1 : 0;
  fflush(state->config.output);
  free(state);
  return res;
}
