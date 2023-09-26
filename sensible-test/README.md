<!--
SPDX-FileCopyrightText: 2023 The libsensible Authors

SPDX-License-Identifier: Unlicense
-->

# sensible-test

Usage:

```C
int main(void) {
  struct sentest_state *state = sentest_start(config);
  sentest_group(state, "the number one") {
    sentest(state, "is number one") {
      sentest_assert_eq(state, 1, 1);
    }
    sentest(state, "isn't number two") {
      sentest_assert_neq(state, 1, 2);
    }
  }
  sentest_finish(state);
}
```
