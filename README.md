# ParallelSort

## Introduction
The parallel mergesort implements three versions of parallel mergesort:
1. a $O(n)$ span algorithm that performs the two recursive mergesort calls in parallel, but performs the
merge step sequentially,
2. a poly-log span algorithm that performs the *merge*$(A, B)$ step in parallel (assume WLOG that $|A| \geq |B|$) by taking the median of $A$, searching for it in $B$, and splitting $A$ and $B$ into
$A_1$, $A_2$, and $B_1$, $B_2$, respectively, so that *merge*$(A_1, B_1)$ and *merge*$(A_2, B_2)$ can be done in parallel.
3. a more optimized version of the previous algorithm.

## Run

Build the driver once with `make`, then choose one of the modes below.

**Sequential mergesort**
```bash
make
./mergesort <num> seq
```

**Parallel mergesort**

The call to `parallel_mergesort::mergesort` in `mergesort.cpp` takes 

1. an integer that selects the parallel version:

| Version | Description |
|---------|-------------|
| `1` | Parallel recursion with a sequential merge step (baseline). |
| `2` | Fully parallel merge (higher overhead on small slices). |
| `3` | Parallel merge with sequential fallbacks under tuned thresholds. |

2. a boolean that decides whether to use ping-pong buffer (false, by default), or copy to a temporary array each iteration (true)

Edit the second argument in the `parallel_mergesort::mergesort(result, <version>, <alloc>)` call (line 40) to the intended version and run:

```bash
make
./mergesort <num> par
```

*Note: This implementation is based on the [offical parlaylib mergesort example](https://github.com/cmuparlay/parlaylib/blob/master/examples/mergesort.h), with some debugging advice from Copilot, using model GPT-5-Codex (Preview).*