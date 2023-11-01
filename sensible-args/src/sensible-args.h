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

enum senargs_argument_type {
  SENARG_FLAG,
  SENARG_STRING,
  SENARG_INT,
  SENARG_SUBCOMMAND
};

struct senargs_argument;

struct senargs_argument_bag {
  struct senargs_argument **args;
  unsigned amt;
  int subcommand_chosen;
};

struct senargs_argument {
  const enum senargs_argument_type tag;

  // when '\0', argument has no short name
  const char short_name;

  union {
    const char *long_name;
    const char *subcommand_name;
  } names;

  const char *description;

  union {
    bool flag_value;
    const char *string_value;
    int int_value;
    struct {
      struct senargs_argument_bag subs;
      int value;
    } subcommand;
  } data;

  // Filled in by argument parser
  unsigned long_len;
};

struct senargs_description {
  struct senargs_argument_bag *root;
  const char *preamble;
};

void senargs_parse(struct senargs_description args, int argc, const char **argv);
void senargs_print_help(struct senargs_description program_args, int argc, const char **argv);

#ifdef __cplusplus
}
#endif

#endif
