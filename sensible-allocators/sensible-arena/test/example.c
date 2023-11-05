// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: CC0-1.0

#include <stdint.h>
#include <stdlib.h>

#include "../src/sensible-arena.h"

int main(void) {
  struct senarena arena = senarena_new();
  uint64_t *a = senarena_alloc(&arena, sizeof(uint64_t) * 3, 8);
  *a = 42;
  return 0;
}
