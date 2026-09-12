# Performance approach and current measurements

The aim is efficient verified execution, not an unsupported claim of absolute
optimality. Work reduction, contiguous ownership and scratch reuse are designed
into the C++17 implementation. Fast-math and unverified numerical approximations
are not permitted. Complete-world throughput has not yet been measured because
the complete-world executor is not implemented.

## Implemented optimizations

| Mechanism | Work or storage removed | Correctness guard |
|---|---|---|
| Decode once; immutable ECL modules | Repeated DAT/decode/compile work across entry attempts | Member hashes, bounded parsing and byte-identical owned payloads |
| Hot operands plus owned cold arena | Common operand indirection without truncating long world commands | Complete-payload comparisons over all compiled records |
| Context-only call frames | Copy/restore only thirty scalar slots rather than all 101 | All-slot ownership, unknown validity, nested calls and unchanged DAT matrix |
| Fixed-state motion and transform kernels | Per-step allocations and duplicated mutable state | Pinned source comparisons and phase-order regressions |
| Bitset bullet-slot index | Linear near-full circular scans | Source-order selection, all single-hole cases and nested cursor completion |
| Owned CSR spatial index | Most unnecessary narrow collision predicates | Unindexed scans, boundary/contact and signed-laser checks |
| Reused planner buffers and deterministic heap | Per-frame tree allocation and full sorting | Retained reference planner, identical routes and independent replay |

The current local x86_64 layout is 304 bytes per call frame and 5520 bytes per
workspace, compared with 944 and 15120 before compaction. These are measured type
sizes, not throughput. The [context-storage test](../tests/context_storage_tests.cpp)
prints the current sizes; [ownership details](ECL_CONTEXT_STORAGE.md) explain the domain.

## Current benchmark records

Records under `reports/native/` are refreshed samples, not stable timing promises.
The reviewed machine is Linux x86_64, AMD EPYC 7B12, GCC 12.2, Release, with floating
point contraction disabled. Load, build profile and compiler changes affect timing.
Do not compare sanitizer timing with Release or add independent microbenchmarks
to estimate complete-world solve cost.

| Record | Workload and interpretation |
|---|---|
| [emitter_benchmark.json](../reports/native/emitter_benchmark.json) | 41 batches of 1000 real sub40 restricted runs; excludes decode/compile, world, collision and planning |
| [geometry_benchmark.json](../reports/native/geometry_benchmark.json) | 1536 synthetic hazards, 20000 queries; compare scan/index hits and narrow-test counts as well as wall time |
| [planner_benchmark.json](../reports/native/planner_benchmark.json) | 14 alternating reference/optimized batches on a 360-frame synthetic scene; identical routes required |
| [bullet_slots_benchmark.json](../reports/native/bullet_slots_benchmark.json) | 21 batches of 20000 near-full selection queries; excludes actual allocation and lifecycle |
| [motion_summary.json](../reports/native/motion_summary.json) | Two fixed-entry 600-frame model generation/search/replay samples, not complete spells |
| [summary.json](../reports/native/summary.json) | Full structural audit wall time including hashes and report writes, not simulation |

Fresh commands are in [Validation](VALIDATION.md). Avoid simultaneous benchmarks
when collecting comparisons. Historical one-off timing anecdotes are no longer
used as current status; the checked-in records carry the present samples and scopes.

## Optimization boundary for the next world layer

Reuse immutable resource programs across candidates, but copy/fork all mutable
state that can affect future events. Do not share a cached emitter future when
player actions change aiming, alignment, damage, RNG or pool availability. Do not
merge candidates by position alone under those conditions. Preserve source operand
read order and separate velocity, shot/ANM and displacement phases.

Measure complete entry-to-terminal execution once it is verified, identify actual
bottlenecks, and only then add justified specialization or SIMD. An optimization
is acceptable only when its independent event/geometry/replay checks retain the
same results in the stated numerical and input domain.
