<!--
SPDX-FileCopyrightText: 2023 The libsensible Authors

SPDX-License-Identifier: Unlicense
-->

# sensible-arena

See the Wikipedia article on [region-based memory management](https://en.wikipedia.org/wiki/Region-based_memory_management).

This is an allocator which can only free everything in one go.

Allocations are explicitly aligned.

```C
// functions
struct senarena senarena_new();
void *senarena_alloc(struct senarena *arena, size_t byte_amount, size_t alignment);
void senarena_clear(struct senarena *arena);
void senarena_free(struct senarena arena);

// macros
void *senarena_alloc_type(struct senarena *arena, type);
void *senarena_alloc_array_of(struct senarena *arena, type, amount);
```
