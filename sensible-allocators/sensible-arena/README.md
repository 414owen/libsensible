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

## Compile options

| CPP Variable                | default     | notes                                    |
| ---                         | ---         | ---                                      |
| SENARENA_DEFAULT_CHUNK_SIZE | 4080        | Only affects senarena compilation unit   |
| SENARENA_NOINLINE           | not defined | Affects units that #include "senarena.h" |

## Benchmarks

### Methodology:

A number (n) of 8 byte arena_alloc()s that takes more than 0.5s is determined empirically.  
A number (m) of 8 byte malloc()s that takes more than 0.5s is determined empirically.

Over 20 rounds:

* We arena_alloc() 8 bytes n times in a fresh arena.
* We arena_free() the arena.
* We arena_alloc() 8 bytes n times in a reused arena.
* We malloc() 8 bytes m times.
* We free() all malloced allocations.

The time for each operation is taken from its best round,
throughput is calculated, and used for the relative comparison.

All times are monotonic durations, which include user-space time *and*
time spent in the kernel.

Speedup is a speedup in throughput (allocations per unit of time) as
defined as `Q₂/Q₁` where:

* `Q₂` is the arena throughput
* `Q₁` is malloc's throughput

For more information see [wikipedia article](https://en.wikipedia.org/wiki/Speedup#Speedup_in_throughput).

### AMD Ryzen 5 5625U

This table measures speedup in throughput of using sensible-arena to allocate,
compared to various malloc implementations.

| speedup                      | glibc  | tcmalloc | mimalloc | jemalloc |
| ---                          | ---    | ---      | ---            | ---            |
| arena_alloc()                | 7.204  | 3.127    | 1.683          | 2.870          |
| arena_alloc() (reused arena) | 12.469 | 3.335    | 3.440          | 7.049          |
| arena_free()                 | 1.609  | 111.753  | 61.997         | 16.545         |

<details>
<summary>Benchmark machine details</summary>

```
$ uname -a
Linux nixos 6.1.55 #1-NixOS SMP PREEMPT_DYNAMIC Sat Sep 23 09:11:13 UTC 2023 x86_64 GNU/Linux
```

**glibc** version: 2.37  
**tcmalloc** version: 4.5.10  
**mimalloc** version: 2.1.2  
**jemalloc** version: 5.3.0  

All malloc implementations are the versions provided by [Nixpkgs](https://github.com/NixOS/nixpkgs) (23.05, [3b79cc4bcd9c09b5aa68ea1957c25e437dc6bc58](https://github.com/NixOS/nixpkgs/tree/3b79cc4bcd9c09b5aa68ea1957c25e437dc6bc58)).

</details>

<details>
<summary>Full benchmark output</summary>

#### glibc 2.37

```
Arena allocation time:             0.421s (min), 0.430s (max)
Arena (reused) allocation time:    0.170s (min), 0.248s (max)
Arena free time:                   0.614s (min), 0.651s (max)
Arena allocations per us:          358.785
Arena (reused) allocations per us: 621.036
Arena allocation frees per us:     236.842

Standard allocation time:         0.185s (min), 0.674s (max)
Standard free time:               0.212s (min), 0.228s (max)
Standard allocations per us:      49.805
Standard allocation frees per us: 147.229

         arena_alloc() vs malloc() speedup: 7.204
(reused) arena_alloc() vs malloc() speedup: 12.469
          arena_free() vs   free() speedup: 1.609
```

#### tcmalloc 4.5.10

```
Arena allocation time:             0.430s (min), 0.441s (max)
Arena (reused) allocation time:    0.294s (min), 0.413s (max)
Arena free time:                   0.017s (min), 0.021s (max)
Arena allocations per us:          608.793
Arena (reused) allocations per us: 649.283
Arena allocation frees per us:     13073.448

Standard allocation time:         0.490s (min), 0.517s (max)
Standard free time:               0.851s (min), 0.860s (max)
Standard allocations per us:      194.709
Standard allocation frees per us: 116.986

         arena_alloc() vs malloc() speedup: 3.127
(reused) arena_alloc() vs malloc() speedup: 3.335
          arena_free() vs   free() speedup: 111.753
```

#### mimalloc-2.1.2

```
Arena allocation time:             0.414s (min), 0.830s (max)
Arena (reused) allocation time:    0.291s (min), 0.406s (max)
Arena free time:                   0.013s (min), 0.014s (max)
Arena allocations per us:          323.479
Arena (reused) allocations per us: 661.277
Arena allocation frees per us:     19749.853

Standard allocation time:         0.427s (min), 0.698s (max)
Standard free time:               0.371s (min), 0.421s (max)
Standard allocations per us:      192.235
Standard allocation frees per us: 318.563

         arena_alloc() vs malloc() speedup: 1.683
(reused) arena_alloc() vs malloc() speedup: 3.440
          arena_free() vs   free() speedup: 61.997
```

#### jemalloc-5.3.0

```
Arena allocation time:             0.467s (min), 0.988s (max)
Arena (reused) allocation time:    0.295s (min), 0.402s (max)
Arena free time:                   0.038s (min), 0.123s (max)
Arena allocations per us:          271.739
Arena (reused) allocations per us: 667.488
Arena allocation frees per us:     2173.577

Standard allocation time:         0.312s (min), 0.739s (max)
Standard free time:               0.487s (min), 0.533s (max)
Standard allocations per us:      94.699
Standard allocation frees per us: 131.374

         arena_alloc() vs malloc() speedup: 2.870
(reused) arena_alloc() vs malloc() speedup: 7.049
          arena_free() vs   free() speedup: 16.545
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

I stepped though mimalloc, the first allocation took 6583 instructions.
The second took 15 (excluding some function call overhead), two of which
were conditional jumps, a `ja`, and a `je`. This is explained in more detail
in this [mimalloc paper](https://www.microsoft.com/en-us/research/uploads/prod/2019/06/mimalloc-tr-v1.pdf).

Here it is:

```
<__libc_malloc>:
  mov    rax,QWORD PTR [rip+0x1c299]
  mov    rsi,rdi
  mov    rdi,QWORD PTR fs:[rax]
  cmp    rsi,0x400                      # allocating more than 1024 bytes?
  ja     <__libc_malloc+0x40>           # no!
  lea    rax,[rsi+0x7]                  # rax is 8(bytes) + 7 = 15
  shr    rax,0x3                        # rax is 1
  mov    rax,QWORD PTR [rdi+rax*8+0x8]  # rax = page (for size?)
                                        # _mi_page_malloc() inlined
  mov    rdx,QWORD PTR [rax+0x10]       # rdx = page->block
  test   rdx,rdx                        # is page->block NULL?
  je     <__libc_malloc+0x50>
  mov    rcx,QWORD PTR [rdx]
  add    DWORD PTR [rax+0x18],0x1       # page->used++ ("pop from the free list")
  mov    QWORD PTR [rax+0x10],rcx       # page->free = mi_block_next(page, block)
  mov    rax,rdx
  ret
```

Our fast path is this:

For alignment 1:

```
  mov    rax,QWORD PTR [rsp]
  mov    rdx,rax
  sub    rdx,QWORD PTR [rsp+0x8]
  sub    rax,0x3
  cmp    rdx,0x2
  jbe    <main+0x51>
  # rax now contains the pointer
```

For other alignments:

```
  mov    rax,QWORD PTR [rsp]
  mov    rdx,rax
  mov    rsi,rax
  sub    rsi,QWORD PTR [rsp+0x8]
  and    edx,0x3
  lea    rcx,[rdx+0x4]
  cmp    rsi,rcx
  jb     <main+0x85>
  sub    rax,rdx
  sub    rax,0x4
  mov    QWORD PTR [rsp],rax
  sub    rsp,0x20
  # rax now contains the pointer
```

Which comes in at 12 (inlined) instructions, one of which is a conditional jump,
or 6 instructions if your alignment happens to be 1.

</details>

### Raspberry pi zero v1.1

This table measures speedup in throughput of using sensible-arena to allocate,
compared to various malloc implementations.

| speedup                      | glibc  | tcmalloc | jemalloc |
| ---                          | ---    | ---      | ---      |
| arena_alloc()                | 20.101 | 10.993   | 8.447    |
| arena_alloc() (reused arena) | 43.832 | 12.141   | 21.253   |
| arena_free()                 | 4.492  | 130.952  | 51.101   |

<details>
<summary>Benchmark machine details</summary>

```
$ uname -a
Linux dietpi 6.1.21+ #1642 Mon Apr  3 17:19:14 BST 2023 armv6l GNU/Linux
```

**glibc** version: 6
**tcmalloc** version: 4.5.10  
**jemalloc** version: 5.2.1  


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
