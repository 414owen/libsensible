<!--
SPDX-FileCopyrightText: 2023 The libsensible Authors

SPDX-License-Identifier: Unlicense
-->

# libsensible

A collection of C libraries for writing sensible, threadsafe, fast code.

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
