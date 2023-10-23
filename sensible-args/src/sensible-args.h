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

typedef enum {
  ARG_FLAG,
  ARG_STRING,
  ARG_INT,
  ARG_SUBCOMMAND,
} arg_type;

struct argument;
typedef struct argument argument;

typedef struct {
  argument *args;
  unsigned amt;
  int subcommand_chosen;
} argument_bag;

struct argument {
  const arg_type tag;

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
      argument_bag subs;
      int value;
    } subcommand;
  } data;

  // Filled in by argument parser
  unsigned long_len;
};

typedef struct {
  argument_bag *root;
  const char *preamble;
} program_args;

void parse_args(program_args args, int argc, char **argv);
void print_help(program_args program_args, int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
