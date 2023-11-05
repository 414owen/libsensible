// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: CC0-1.0

#include <stdio.h>
#include <ctype.h>

#include "../src/sensible-args.h"

#define STATIC_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

static void upcase(char *a) {
  size_t i = 0;
  while (a[i] != '\0') {
    a[i] = toupper(a[i]);
    i++;
  }
}

int main(int argc, char **argv) {
  struct senargs_argument target = {
    .tag = SENARG_STRING,
    .small = 't',
    .full = "target",
    .description = "Target of greeting",
  };

  struct senargs_argument shout = {
    .tag = SENARG_FLAG,
    .small = 's',
    .full = "shout",
    .description = "Target of greeting",
  };

  struct senargs_argument enthusiasm = {
    .tag = SENARG_INT,
    .small = 'e',
    .full = "enthusiasm",
    .description = "How enthusiastically to greet",
  };

  struct senargs_argument *root_args[] = {
    &target,
    &shout,
    &enthusiasm
  };

  struct senargs_argument_bag root = {
    .amt = STATIC_LEN(root_args),
    .args = root_args
  };

  struct senargs_description desc = {
    .root = &root,
    .preamble = NULL
  };

  senargs_parse(desc, argc, argv);

  if (target.data.string_value == NULL) {
    senargs_print_help(desc, argc, argv);
    exit(1);
  }

  if (shout.data.flag_value) {
    upcase(target.data.string_value);
    printf("HELLO %s ", target.data.string_value);
  } else {
    printf("hello %s ", target.data.string_value);
  }
  for (int i = 0; i < enthusiasm.data.int_value; i++) {
    putchar('!');
  }
  putchar('\n');
}
