<!--
SPDX-FileCopyrightText: 2023 The libsensible Authors

SPDX-License-Identifier: CC0-1.0
-->

# libsensible

[![ci tests](https://img.shields.io/github/actions/workflow/status/414owen/libsensible/Tests)](https://github.com/414owen/libsensible/actions/workflows/tests.yml)
![license badge](https://img.shields.io/github/license/414owen/libsensible)
![repo size badge](https://img.shields.io/github/repo-size/414owen/libsensible)
![c language badge](https://img.shields.io/badge/made%20with-C-blue?logo=c)

A collection of C libraries for writing sensible, threadsafe, fast code.

Some libraries need to be shared because they are finnicky and shouldn't be
rewritten per-project, for example, the `sensible-arena` allocator.

Some libraries are a huge quality-of-life boon, for example `sensible-test`,
a frictionless test library, which your CI understands, and `sensible-args`,
for parsing command line arguments easily.

Some libraries fill in the cracks between platforms, `sensible-timing` is one of these.

* ISO-C99 compliant
* Zero dependencies
* Trivial to build
* Tested on Linux, MacOS, and Windows
* Benchmarked, where appropriate

## [sensible-test](./sensible-test)

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

## [sensible-arena](./sensible-allocators/sensible-arena)

* Incredibly optimized fast path
* Fast path is inlined by default
* Speeds up allocations by multiple times compared to general-purpose allocator
* Conceptually simple memory management

## [sensible-timing](./sensible-timing)

Gives you an as-monotonic-as-possible, as-accurate-as-possible,
as-wall-time-as-possible timing API.

## [sensible-bitvec](./sensible-bitvec)

This is a bitvector.

## [sensible-args](./sensible-args)

Status: Work in progress

Supports flags, strings, ints, help generation...
Not recommended for use.
