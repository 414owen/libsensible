// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SENSIBLE_ARGS_H
#define SENSIBLE_ARGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdlib.h>

enum arg_type {
  ARG_FLAG,
  ARG_STRING,
  ARG_INT,
  ARG_SUBCOMMAND
};

struct argument;
typedef struct argument argument;

struct argument_bag {
  argument *args;
  unsigned amt;
  int subcommand_chosen;
};

struct argument {
  const enum arg_type tag;

  // when '\0', argument has no short name
  const char short_name;

  union {
    const char *long_name;
    const char *subcommand_name;
  } names;

  const char *description;

  union {
    bool *flag_val;
    char **string_val;
    int *int_val;
    struct {
      struct argument_bag subs;
      int value;
    } subcommand;
  } data;

  // Filled in by argument parser
  unsigned long_len;
};

struct program_args {
  struct argument_bag *root;
  const char *preamble;
};

void parse_args(struct program_args args, int argc, char **argv);
void print_help(struct program_args program_args, int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
