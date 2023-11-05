<!--
SPDX-FileCopyrightText: 2023 The libsensible Authors

SPDX-License-Identifier: CC0-1.0
-->

# sensible-test

* Tests form trees
* Test declaration syntax mirrors the test tree
* Exposes results to in machine readable format for CI
* Tests are filterable

### Example test

```C
struct sentest_state *state = sentest_start(config);
sentest_group(state, "the number one") {
  sentest_group(state, "compared to one") {
    sentest(state, "is considered equal") {
      sentest_assert_eq(state, 1, 1);
    }
  }
  sentest(state, "isn't number two") {
    sentest_assert_neq(state, 1, 2);
  }
}
sentest_finish(state);
```

### Example output

```
the number one
  compared to one
    is considered equal ✓
  isn't number two ✓
```
