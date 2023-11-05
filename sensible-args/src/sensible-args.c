// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define STATIC_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#include "sensible-args.h"
#include "sensible-bitvec.h"

struct senargs_vec_string {
  const char **data;
  size_t length;
  size_t capacity;
};

struct parse_state {
  char **argv;
  int arg_cursor;
  int argc;
  struct senargs_description program_args;
  struct senargs_argument_bag *arguments;
  struct senargs_vec_string subcommands;
};

enum prefix_type {
  PREFIX_LONG,
  PREFIX_SHORT,
  PREFIX_NONE,
};

static
char *type_strs[] = {
  [SENARG_FLAG] = "",
  [SENARG_STRING] = "STRING",
  [SENARG_INT] = "INT",
};

static
const struct senargs_argument help_arg = {
  .tag = SENARG_FLAG,
  .small = 'h',
  .full = "help",
  .description = "list available commands and arguments",
};

void senargs_vec_string_push(struct senargs_vec_string *vec, const char *str) {
  if (vec->length == vec->capacity) {
    vec->capacity += vec->capacity >> 1;
    vec->data = realloc(vec->data, sizeof(char*) * vec->capacity);
  }
  vec->data[vec->length++] = str;
}

static void senargs_parse_rec(struct parse_state *state);

#ifndef NDEBUG
static bool valid_short_flag(char c) { return c >= 'a' && c <= 'z'; }
#endif

static
size_t senargs_safe_strlen(const char *str) {
  if (str == NULL) return 0;
  return strlen(str);
}

static
void print_argument(struct senargs_argument a, int max_long_len, int max_type_len) {
  const char *type_str = type_strs[a.tag];
  printf("  -%c%*s--%s%*s  %s\n",
         a.small == '\0' ? ' ' : a.small,
         2 + max_long_len - (int)senargs_safe_strlen(a.full),
         "",
         a.full,
         2 + max_type_len,
         type_str,
         a.description);
}

static
void print_help_internal(struct parse_state *state) {
  if (state->program_args.preamble != NULL) {
    puts(state->program_args.preamble);
  }
  unsigned num_subcommands = 0;

  for (unsigned i = 0; i < state->arguments->amt; i++) {
    struct senargs_argument *a = state->arguments->args[i];
    switch (a->tag) {
      case SENARG_SUBCOMMAND:
        num_subcommands++;
        break;
      default:
        break;
    }
  }

  printf("Usage: %s", state->argv[0]);
  for (unsigned i = 0; i < state->subcommands.length; i++) {
    printf(" %s", state->subcommands.data[i]);
  }

  fputs(" [options]", stdout);
  if (num_subcommands > 0) {
    fputs(" <subcommand>", stdout);
    fputs(" [options]", stdout);
  }

  puts("\n");
  unsigned max_long_len = senargs_safe_strlen(help_arg.full);
  for (unsigned i = 0; i < state->arguments->amt; i++) {
    struct senargs_argument *a = state->arguments->args[i];
    max_long_len = MAX(max_long_len, senargs_safe_strlen(a->full));
  }

  int max_type_len = 0;
  for (size_t i = 0; i < STATIC_LEN(type_strs); i++) {
    max_type_len = MAX(max_type_len, (int)senargs_safe_strlen(type_strs[i]));
  }

  if (num_subcommands > 0) {
    puts("subcommands:");
    for (unsigned i = 0; i < state->arguments->amt; i++) {
      struct senargs_argument *a = state->arguments->args[i];
      if (a->tag == SENARG_SUBCOMMAND) {
        printf("%*s %*s %s\n",
               8 + max_long_len,
               a->full,
               2 + max_type_len,
               "",
               a->description);
      }
    }
    putc('\n', stdout);
  }

  puts("options:");
  for (unsigned i = 0; i < state->arguments->amt; i++) {
    struct senargs_argument a = *state->arguments->args[i];
    if (a.tag == SENARG_SUBCOMMAND) {
      continue;
    }
    print_argument(a, max_long_len, max_type_len);
  }
  print_argument(help_arg, max_long_len, max_type_len);
}

// TODO HEDLEY_NO_RETURN
static
void unknown_arg(struct parse_state *restrict state,
                        const char *restrict arg_str) {
  if (!state->program_args.do_not_panic) {
    fprintf(stderr, "Unknown argument: '%s'.\n", arg_str);
    print_help_internal(state);
    exit(1);
  }
}

// TODO HEDLEY_NO_RETURN
static
void expected_argument(struct parse_state *restrict state,
                              const char *restrict arg_name) {
  fprintf(
    stderr, "Argument '%s' expected a value, but none given.\n", arg_name);
  print_help_internal(state);
  exit(1);
}

static
void assign_data(struct senargs_argument *restrict a, char *restrict str) {
  // convert from string to type
  switch (a->tag) {
    case SENARG_INT:
      a->data.int_value = atoi(str);
      break;
    case SENARG_STRING:
      a->data.string_value = str;
      break;
    case SENARG_FLAG:
      break;
    case SENARG_SUBCOMMAND:
      // TODO?
      break;
  }
}

static
void parse_long(struct parse_state *restrict state, char *restrict arg_str) {
  size_t arg_str_len = senargs_safe_strlen(arg_str);
  if (strcmp(arg_str, "help") == 0) {
    print_help_internal(state);
    exit(0);
  }
  for (unsigned i = 0; i < state->arguments->amt; i++) {
    struct senargs_argument *a = state->arguments->args[i];
    bool prefix_matches = arg_str_len >= senargs_safe_strlen(a->full) &&
                          strncmp(arg_str, a->full, senargs_safe_strlen(a->full)) == 0;
    bool matches = prefix_matches && arg_str_len == senargs_safe_strlen(a->full);
    bool matched = false;
    switch (a->tag) {
      case SENARG_SUBCOMMAND:
        break;
      case SENARG_FLAG:
        if (matches) {
          a->data.flag_value = true;
          matched = true;
        }
        break;
      case SENARG_INT:
      case SENARG_STRING:
        if (matches) {
          if (state->arg_cursor == state->argc - 1) {
            expected_argument(state, a->full);
          }
          state->arg_cursor++;
          char *str = state->argv[state->arg_cursor];
          assign_data(a, str);
          matched = true;
        } else if (prefix_matches && arg_str[senargs_safe_strlen(a->full)] == '=') {
          char *str = &arg_str[senargs_safe_strlen(a->full) + 1];
          assign_data(a, str);
          matched = true;
        }
        break;
    }
    if (matched) {
      state->arg_cursor++;
      return;
    }
  }
  unknown_arg(state, arg_str);
}

static
void parse_short(struct parse_state *state, const char *restrict arg_str) {
  size_t arg_str_len = senargs_safe_strlen(arg_str);
  int cursor = state->arg_cursor;
  if (strchr(arg_str, 'h')) {
    print_help_internal(state);
    exit(0);
  }

  // just ignore "-"
  if (arg_str_len == 0) {
    return;
  }
  bool first_unmatched = true;
  for (size_t i = 0; i < arg_str_len; i++) {
    char c = arg_str[i];
    bool matched = false;
    for (unsigned j = 0; !matched && j < state->arguments->amt; j++) {
      struct senargs_argument *a = state->arguments->args[j];
      matched |= c == a->small;
      switch (a->tag) {
        case SENARG_SUBCOMMAND:
          break;
        case SENARG_FLAG:
          if (matched) {
            a->data.flag_value = true;
          }
          break;

        case SENARG_INT:
        case SENARG_STRING:
          if (matched) {
            if (arg_str_len > 1) {
              fprintf(stderr,
                      "Short argument '%c' takes a parameter, so can't be used "
                      "with other short arguments.\n",
                      a->small);
              print_help_internal(state);
              exit(1);
            }
            if (cursor == state->argc - 1) {
              char s[2] = {a->small, '\0'};
              expected_argument(state, s);
            }
            cursor++;
            char *str = state->argv[cursor];
            assign_data(a, str);
          }
          break;
      }
    }
    if (!matched) {
      if (first_unmatched) {
        first_unmatched = false;
      }
      if (!state->program_args.do_not_panic) {
        fprintf(stderr, "Unknown short argument: '%c'\n", c);
      }
    }
  }
  if (!first_unmatched) {
    if (!state->program_args.do_not_panic) {
      exit(1);
    }
  }
  state->arg_cursor = cursor + 1;
}

static
void parse_noprefix(struct parse_state *state, const char *restrict arg_str) {
  bool subcommand_taken = false;
  for (unsigned i = 0; i < state->arguments->amt; i++) {
    struct senargs_argument *a = state->arguments->args[i];
    if (a->tag != SENARG_SUBCOMMAND) {
      continue;
    }
    if (strcmp(arg_str, a->full) == 0) {
      subcommand_taken = true;
      state->arg_cursor++;
      senargs_vec_string_push(&state->subcommands, a->full);
      state->arguments->subcommand_chosen = a->data.subcommand.value;
      struct senargs_argument_bag *tmp = state->arguments;
      state->arguments = &a->data.subcommand.subs;
      senargs_parse_rec(state);
      state->arguments = tmp;
      break;
    }
  }
  if (!subcommand_taken) {
    fprintf(stderr, "Unknown subcommand: %s\n", arg_str);
    print_help_internal(state);
    exit(1);
  }
  // TODO support positional args here
}

// recursive, but should be fine
static
void preprocess_and_validate_args(struct senargs_argument_bag bag) {
  for (unsigned i = 0; i < bag.amt; i++) {
    struct senargs_argument a = *bag.args[i];
    if (a.tag == SENARG_SUBCOMMAND) {
      preprocess_and_validate_args(a.data.subcommand.subs);
    }
    assert(a.full == 0 || valid_short_flag(a.small));
  }
}

static void senargs_parse_rec(struct parse_state *state) {
  while (state->arg_cursor < state->argc) {
    char *arg_str = state->argv[state->arg_cursor];
    enum prefix_type prefix_type = PREFIX_NONE;

    if (arg_str[0] == '-') {
      prefix_type = PREFIX_SHORT;
      arg_str++;
      if (arg_str[0] == '-') {
        prefix_type = PREFIX_LONG;
        arg_str++;
      }
    }

    switch (prefix_type) {
      case PREFIX_LONG:
        parse_long(state, arg_str);
        break;
      case PREFIX_SHORT:
        parse_short(state, arg_str);
        break;
      case PREFIX_NONE:
        parse_noprefix(state, arg_str);
        break;
    }
  }
}

static
struct parse_state new_state(struct senargs_description program_args, int argc,
                             char **argv) {
  struct parse_state res = {
    // assume the first parameter is the program name
    .arg_cursor = 1,
    .program_args = program_args,
    .arguments = program_args.root,
    .argc = argc,
    .argv = argv,
    .subcommands = {
      .data = malloc(sizeof(char*) * 4),
    },
  };
  return res;
}

void senargs_print_help(struct senargs_description program_args, int argc, char **restrict argv) {
  struct parse_state state = new_state(program_args, argc, argv);
  print_help_internal(&state);
  free(state.subcommands.data);
}

void senargs_parse(struct senargs_description program_args, int argc, char **restrict argv) {
  preprocess_and_validate_args(*program_args.root);
  struct parse_state state = new_state(program_args, argc, argv);
  senargs_parse_rec(&state);
  free(state.subcommands.data);
}
