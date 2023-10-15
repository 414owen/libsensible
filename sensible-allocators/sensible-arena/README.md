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

## Benchmarks

### AMD Ryzen 5 5625U

```
$ uname -a
Linux nixos 6.1.55 #1-NixOS SMP PREEMPT_DYNAMIC Sat Sep 23 09:11:13 UTC 2023 x86_64 GNU/Linux
```

#### glibc

```
alloc:
Arena allocation time:             0.417s (min), 0.425s (max)
Arena (reused) allocation time:    0.173s (min), 0.248s (max)
Arena free time:                   0.612s (min), 0.652s (max)
Arena allocations per us:          355.381
Arena (reused) allocations per us: 608.771
Arena allocation frees per us:     231.506

Standard allocation time:         0.194s (min), 0.693s (max)
Standard free time:               0.230s (min), 0.246s (max)
Standard allocations per us:      48.400
Standard allocation frees per us: 136.378

         arena_alloc() vs malloc() speedup: 7.343
(reused) arena_alloc() vs malloc() speedup: 12.578
          arena_free() vs   free() speedup: 1.698
```

#### mimalloc-2.1.2

```
Arena allocation time:             0.427s (min), 0.833s (max)
Arena (reused) allocation time:    0.307s (min), 0.422s (max)
Arena free time:                   0.012s (min), 0.015s (max)
Arena allocations per us:          322.169
Arena (reused) allocations per us: 636.613
Arena allocation frees per us:     18022.349

Standard allocation time:         0.463s (min), 0.728s (max)
Standard free time:               0.400s (min), 0.649s (max)
Standard allocations per us:      184.483
Standard allocation frees per us: 206.734

         arena_alloc() vs malloc() speedup: 1.746
(reused) arena_alloc() vs malloc() speedup: 3.451
          arena_free() vs   free() speedup: 87.177
```

#### jemalloc-5.3.0

```
Arena allocation time:             0.483s (min), 0.976s (max)
Arena (reused) allocation time:    0.321s (min), 0.440s (max)
Arena free time:                   0.041s (min), 0.223s (max)
Arena allocations per us:          275.087
Arena (reused) allocations per us: 609.978
Arena allocation frees per us:     1203.536

# Benchmarking malloc use

Standard allocation time:         0.318s (min), 0.671s (max)
Standard free time:               0.481s (min), 0.534s (max)
Standard allocations per us:      96.864
Standard allocation frees per us: 121.633

         arena_alloc() vs malloc() speedup: 2.840
(reused) arena_alloc() vs malloc() speedup: 6.297
          arena_free() vs   free() speedup: 9.895
```

### Raspberry pi zero v1.1

#### glibc 6

```
Arena allocation time:             0.499s (min), 0.525s (max)
Arena (reused) allocation time:    0.226s (min), 0.230s (max)
Arena free time:                   1.565s (min), 1.582s (max)
Arena allocations per us:          30.979
Arena (reused) allocations per us: 70.600
Arena allocation frees per us:     10.277

Standard allocation time:         0.266s (min), 0.653s (max)
Standard free time:               0.452s (min), 0.457s (max)
Standard allocations per us:      1.607
Standard allocation frees per us: 2.296

         arena_alloc() vs malloc() speedup: 19.280
(reused) arena_alloc() vs malloc() speedup: 43.938
          arena_free() vs   free() speedup: 4.477
```

#### tcmalloc 4

```
Arena allocation time:             0.504s (min), 0.508s (max)
Arena (reused) allocation time:    0.226s (min), 0.229s (max)
Arena free time:                   1.578s (min), 1.589s (max)
Arena allocations per us:          31.989
Arena (reused) allocations per us: 70.923
Arena allocation frees per us:     10.229

Standard allocation time:         0.266s (min), 0.667s (max)
Standard free time:               0.476s (min), 0.480s (max)
Standard allocations per us:      1.572
Standard allocation frees per us: 2.184

         arena_alloc() vs malloc() speedup: 20.349
(reused) arena_alloc() vs malloc() speedup: 45.116
          arena_free() vs   free() speedup: 4.684
```

#### jemalloc 2

```
Arena allocation time:             0.487s (min), 0.520s (max)
Arena (reused) allocation time:    0.219s (min), 0.245s (max)
Arena free time:                   1.574s (min), 1.625s (max)
Arena allocations per us:          30.255
Arena (reused) allocations per us: 64.119
Arena allocation frees per us:     9.677
# Benchmarking malloc use
Standard allocation time:         0.266s (min), 0.872s (max)
Standard free time:               0.463s (min), 0.491s (max)
Standard allocations per us:      1.202
Standard allocation frees per us: 2.135

         arena_alloc() vs malloc() speedup: 25.169
(reused) arena_alloc() vs malloc() speedup: 53.341
          arena_free() vs   free() speedup: 4.533
```
