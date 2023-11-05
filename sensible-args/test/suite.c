// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: Unlicense

#include <stdbool.h>
#include <stdint.h>

#include "../src/sensible-args.h"
#include "../../sensible-test/src/sensible-test.h"
#include "../../sensible-macros/include/sensible-macros.h"

#define STATIC_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

senmac_public
void run_sensible_args_suite(struct sentest_state *state) {
  sentest_group(state, "sensible-args") {
    sentest(state, "minimal invocation") {

      struct senargs_argument_bag root = {
        .args = NULL,
        .amt = 0
      };

      struct senargs_description args = {
        .root = &root,
        .preamble = NULL,
        .do_not_panic = true
      };

      senargs_parse(args, 0, NULL);
    }

    sentest_group(state, "flags") {
      struct senargs_argument y_flag = {
        .tag = SENARG_FLAG,
        .small = 'y',
        .full = "yes",
        .description = "is yes the answer?",
        .data.flag_value = false
      };

      struct senargs_argument *root_args[] = { &y_flag };

      struct senargs_argument_bag root = {
        .amt = STATIC_LEN(root_args),
        .args = root_args
      };

      struct senargs_description desc = {
        .root = &root,
        .preamble = NULL,
        .do_not_panic = true
      };

      sentest_group(state, "when not present") {
        sentest(state, "doesn't touch default") {
          senargs_parse(desc, 0, NULL);
          sentest_assert_eq(state, y_flag.data.flag_value, false);
          y_flag.data.flag_value = true;
          senargs_parse(desc, 0, NULL);
          sentest_assert_eq(state, y_flag.data.flag_value, true);
        }
      }

      sentest_group(state, "when present") {
        sentest(state, "is set to true") {
          y_flag.data.flag_value = false;
          static char *argv[] = {
            "./test",
            "-y"
          };
          senargs_parse(desc, STATIC_LEN(argv), argv);
          sentest_assert_eq(state, y_flag.data.flag_value, true);
        }
      }

      sentest_group(state, "when different") {
        sentest(state, "is set to false") {
          y_flag.data.flag_value = false;
          static char *argv[] = {
            "./test",
            "-n"
          };
          senargs_parse(desc, STATIC_LEN(argv), argv);
          sentest_assert_eq(state, y_flag.data.flag_value, false);
        }
      }
    }
  }
}
