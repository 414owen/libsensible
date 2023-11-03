<!--
SPDX-FileCopyrightText: 2023 The libsensible Authors

SPDX-License-Identifier: Unlicense
-->

# libsensible

A collection of C libraries for writing sensible, threadsafe, fast code.

Some libraries need to be shared because they are finnicky and shouldn't be
rewritten per-project. For this, I've written the `sensible-arena` allocator.

Some libraries are a huge quality-of-life boon, for example `sensible-test`,
a frictionless test library, which your CI understands.

Some libraries fill in the cracks between platforms, `sensible-timing` will
be one of these.

Principles:

* ISO-C99 compliant, so that it can run on your microwave
* Memory management should fit the problem
* Pretty much everything should take a (void*) context
* Code should be optimized for readability
* Zero dependencies
* Build process should be trivial

## sensible-test

* Testing should be easy
* Testing should expose results to CI
* Tests descriptions form a tree
* Tests should be filterable

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

## sensible-arena

* Incredibly optimized fast path
* Fast path is inlined by default
* Speeds up allocations by multiple times compared to general-purpose allocator
* Conceptually simple memory management

## sensible-timing

Gives you an as-monotonic-as-possible, as-accurate-as-possible,
as-wall-time-as-possible timing API. Works on linux, probably works on
modernish POSIXes, and might work on Windows.

## sensible-bitvec

This is a bitvector.

## sensible-args

Status: Work in progress

Supports flags, strings, ints, help generation...
Not recommended for use.
