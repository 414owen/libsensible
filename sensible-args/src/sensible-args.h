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
  const unsigned amt;
  int subcommand_chosen;
};

struct senargs_argument {
  const enum senargs_argument_type tag;

  // in C, `long` and `short` are keywords

  // use NULL or "" to disable
  const char *full;
  // when '\0', argument has no short name
  const char small;

  const char *description;

  union data {
    bool flag_value;
    char *string_value;
    int int_value;
    struct subcommand {
      struct senargs_argument_bag subs;
      int value;
    } subcommand;
  } data;
};

struct senargs_description {
  struct senargs_argument_bag *root;
  const char *preamble;
  const bool do_not_exit;
};

struct senargs_result {
  bool success;
};

void senargs_parse(struct senargs_description args, int argc, char **argv);
void senargs_print_help(struct senargs_description program_args, int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
