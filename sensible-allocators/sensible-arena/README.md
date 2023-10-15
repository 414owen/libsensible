<!--
SPDX-FileCopyrightText: 2023 The libsensible Authors

SPDX-License-Identifier: Unlicense
-->

# sensible-arena

See the Wikipedia article on [region-based memory management](https://en.wikipedia.org/wiki/Region-based_memory_management).

This is an allocator with blazingly fast throughput, which frees all of its contents in one go.
If this matches the memory use of your problem, then you should probably prefer an arena allocator
over malloc.

It's optimized for small allocations (<= 128b), but it will allocate memory
for any size if it has to.

It will expand to meet demand, until you run out of heap space.

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

### Methodology:

A number (n) of 8 byte arena allocations that takes more than 0.5s is determined.
A number (m) of 8 byte arena allocations that takes more than 0.5s is determined.

In 20 rounds:
We arena_alloc() 8 bytes n times in a fresh arena.
we arena_free() the arena.
We arena_alloc() 8 bytes n times in a reused arena.
We malloc() 8 bytes m times.
We free() all malloced allocations.

The best round time for each operation is taken, throughput is calculated, and
used for the relative comparison.

All times are monotonic durations, which include user-space time *and*
time spent in the kernel.

Speedup is a speedup in throughput as defined in this [wikipedia article](https://en.wikipedia.org/wiki/Speedup#Speedup_in_throughput) as `Q₂/Q₁` (higher is better).

### AMD Ryzen 5 5625U


| speedup         | alloc  | reused arena alloc | free    |
| ---             | ---    | ---                | ---     |
| glibc           | 7.343  | 12.578             | 1.698   |
| tcmalloc 4.5.10 | 1.899  | 1.981              | 113.738 |
| mimalloc 2.1.2  | 1.746  | 3.451              | 87.177  |
| jemalloc 5.3.0  | 2.840  | 6.297              | 9.895   |

<details>
<summary>Benchmark machine details</summary>

```
$ uname -a
Linux nixos 6.1.55 #1-NixOS SMP PREEMPT_DYNAMIC Sat Sep 23 09:11:13 UTC 2023 x86_64 GNU/Linux
```

All malloc implementations are the versions provided by Nixpkgs (23.05, 3b79cc4bcd9c09b5aa68ea1957c25e437dc6bc58).

</details>

<details>
<summary>Full benchmark output</summary>

#### glibc

```
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

#### tcmalloc 4.5.10

```
Arena allocation time:             0.531s (min), 0.543s (max)
Arena (reused) allocation time:    0.464s (min), 0.520s (max)
Arena free time:                   0.010s (min), 0.015s (max)
Arena allocations per us:          326.336
Arena (reused) allocations per us: 340.454
Arena allocation frees per us:     12111.735

Standard allocation time:         0.504s (min), 0.549s (max)
Standard free time:               0.872s (min), 0.886s (max)
Standard allocations per us:      171.841
Standard allocation frees per us: 106.488

         arena_alloc() vs malloc() speedup: 1.899
(reused) arena_alloc() vs malloc() speedup: 1.981
          arena_free() vs   free() speedup: 113.738
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

Standard allocation time:         0.318s (min), 0.671s (max)
Standard free time:               0.481s (min), 0.534s (max)
Standard allocations per us:      96.864
Standard allocation frees per us: 121.633

         arena_alloc() vs malloc() speedup: 2.840
(reused) arena_alloc() vs malloc() speedup: 6.297
          arena_free() vs   free() speedup: 9.895
```

</details>

<details>
<summary>Observations (to be taken with a grain of salt)</summary>

These observations ONLY apply to this hardware, this OS, these allocator versions,
and for this allocation size.

* glibc seems to allocate faster than it can free
  * This doesn't mean allocations are fast, it means frees are slow
* glibc is far faster at freeing small allocations (8b) than large ones (4096b)
* Modern tcmalloc and mimalloc are both incredibly fast

</details>

### Raspberry pi zero v1.1

| speedup         | alloc  | reused arena alloc | free    |
| ---             | ---    | ---                | ---     |
| glibc           | 20.101 | 43.832             | 4.492   |
| tcmalloc 4.5.10 | 10.993 | 12.141             | 130.952 |
| jemalloc 5.2.1  | 8.447  | 21.253             | 51.101  |

<details>
<summary>Benchmark machine details</summary>

```
$ uname -a
Linux dietpi 6.1.21+ #1642 Mon Apr  3 17:19:14 BST 2023 armv6l GNU/Linux
```

All malloc implementations were installed from the Debian repositories.

</details>

<details>
<summary>Full benchmark output</summary>

#### glibc 6

```
Arena allocation time:             0.500s (min), 0.503s (max)
Arena (reused) allocation time:    0.226s (min), 0.231s (max)
Arena free time:                   1.542s (min), 1.574s (max)
Arena allocations per us:          32.284
Arena (reused) allocations per us: 70.400
Arena allocation frees per us:     10.327

Standard allocation time:         0.266s (min), 0.653s (max)
Standard free time:               0.452s (min), 0.456s (max)
Standard allocations per us:      1.606
Standard allocation frees per us: 2.299

         arena_alloc() vs malloc() speedup: 20.101
(reused) arena_alloc() vs malloc() speedup: 43.832
          arena_free() vs   free() speedup: 4.492
```

#### tcmalloc 4.5.10

```
Arena allocation time:             0.507s (min), 0.546s (max)
Arena (reused) allocation time:    0.436s (min), 0.494s (max)
Arena free time:                   0.066s (min), 0.070s (max)
Arena allocations per us:          57.642
Arena (reused) allocations per us: 63.661
Arena allocation frees per us:     449.281

Standard allocation time:         0.521s (min), 0.525s (max)
Standard free time:               0.787s (min), 0.802s (max)
Standard allocations per us:      5.243
Standard allocation frees per us: 3.431

         arena_alloc() vs malloc() speedup: 10.993
(reused) arena_alloc() vs malloc() speedup: 12.141
          arena_free() vs   free() speedup: 130.952
```

#### jemalloc 5.2.1

```
Arena allocation time:             0.283s (min), 0.665s (max)
Arena (reused) allocation time:    0.261s (min), 0.264s (max)
Arena free time:                   0.035s (min), 0.150s (max)
Arena allocations per us:          28.403
Arena (reused) allocations per us: 71.463
Arena allocation frees per us:     125.586

Standard allocation time:         0.466s (min), 0.512s (max)
Standard free time:               0.690s (min), 0.700s (max)
Standard allocations per us:      3.363
Standard allocation frees per us: 2.458

         arena_alloc() vs malloc() speedup: 8.447
(reused) arena_alloc() vs malloc() speedup: 21.253
          arena_free() vs   free() speedup: 51.101
```

</details>
