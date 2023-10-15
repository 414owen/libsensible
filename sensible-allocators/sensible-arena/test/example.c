#include <stdint.h>
#include <stdlib.h>

#include "../src/sensible-arena.h"

int main(int argc) {
  struct senarena arena = senarena_new();
  uint64_t *a = senarena_alloc(&arena, sizeof(uint64_t) * 3, argc);
  *a = 42;
  return 0;
}
