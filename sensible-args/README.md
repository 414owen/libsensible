<!--
SPDX-FileCopyrightText: 2023 The libsensible Authors

SPDX-License-Identifier: Unlicense
-->


# sensible-args

Status: Work in progress

## Features:

- [x] Flags
- [x] Strings
- [x] Ints
- [x] Subcommands
- [x] Help generation
- [ ] Mandatory args
- [ ] Positional args

## What does it look like:

```
$ sensible-args-example
Usage: sensible-args-example [options]

options:
  -t      --target  STRING  Target of greeting
  -s       --shout          Target of greeting
  -e  --enthusiasm     INT  How enthusiastically to greet
  -h        --help          list available commands and arguments

```
