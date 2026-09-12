# Current implementation status

Reviewed: 2026-09-12. This is the current capability ledger, not a development diary.
Maintained implementation, tools and tests are C++17. No game or input controller
is launched. Historical preparation results are not counted as native coverage.

| Outcome | Verified count |
|---|---|
| Indexed original spell IDs | 222 |
| Indexed spell-start occurrences | 431 |
| Complete offline spell worlds | 0 |
| Complete offline spell solutions | 0 |
| Source-driven 600-frame particle fixtures with replayed routes | 2 |

Do not derive a completion percentage from opcode support, test counts, or these
two fixtures. A spell ID can have different stage/practice, difficulty, character,
form, entry-state and RNG-dependent execution cases.

## What works now

| Layer | Implemented and checked | Still outside that claim |
|---|---|---|
| Resource decoding | All 317 DAT members; ECL, timeline, SHT, ANM and STD structural parsing and field checks | Running all resource programs or gameplay |
| Scalar ECL | Typed arithmetic, branches, normal calls/waits, explicit RNG, owned full payloads and resumable world handoffs | General ECL/EX world effects, fractional ECL clocks, child contexts and callback tails |
| Context ownership | Thirty-slot snapshots; source-proven 46 scalar template zeros; compact call frames with unknown validity preserved | Full actor initialization, spawn transactions or globally coherent shared storage |
| Timeline | Source clocks, masks, waits, event slots and pending-effect tokens; real practice-entry request boundaries | Executing spawn/dialogue/boss/power/menu effects or defeating the boss |
| Practice entry ownership | Owned 480-slot spawn pool, immediate resumable ECL prefix, restricted frame tail and post-spawn stores; real timeline packet connected to sub0 PC1 | Effect51/ANM/camera integration, surrounding world phases and a complete practice entry |
| Enemy motion | Polar, relative/interpolated and orbital motion; separate velocity/displacement phases; transactional ECL effects 63..76 and 178 | Actor pools, form/death/pause gates, child/parent lifecycle and shot/ANM scheduling |
| ANM | Control/scalar execution, waits, interrupts, sprite identity, explicit RNG and hit-animation metadata; restricted lifetime certificates | Render/interpolation fields, resource-loading side effects and integration with world consumers |
| Camera particles | Effect 51 initialization and update callbacks with explicit camera, boss, tint and RNG | Effect allocation, surrounding ANM, camera evolution, freeze and retirement |
| Bullet/laser kernels | Nine launch modes, direction/acceleration phases, supported eighteen-record transforms and laser collision/lifetime projection | General transforms, sprite replacement, child patterns and complete pool/cancellation lifecycle |
| Slot selection | Source-ordered 1536-slot circular selection and nested cursor completion using bitsets | Bullet storage, successful initialization or full-pool RNG behavior |
| Geometry | Box/laser predicates, owned CSR broad phase, signed laser dimensions and unindexed differential checks | Whole-game gates or a formal proof over every floating-point input |
| Planning | Deterministic bounded search, reusable buffers, independent route replay, fixed-model differential tests | Complete or optimal search; safe merging of different candidate-dependent world states |
| Integrated fixtures | Actual sub40/41 requests connected through motion, geometry, search and replay | Complete Wriggle, Reisen or any other spell |

Implementation paths and ownership rules are mapped in [Architecture](ARCHITECTURE.md).
The [comparison chain](PROVENANCE.md) identifies independent source evidence and
exclusions for each layer; passing an adapter does not make it a complete world.

## Current all-resource and execution baselines

| Audit | Current result | Authoritative generated record |
|---|---|---|
| ECL structure | 24 files, 1449 subs, 36661 nonterminal instructions, 2182 checked jumps, 32 EX IDs | [summary.json](../reports/native/summary.json) |
| Other structures | 32 timelines / 2003 instructions; 8 SHT / 50 levels / 227 descriptors; 113 ANM / 310 entries / 1151 scripts / 1917 sprites / 15966 instructions; 18 STD / 68 objects / 552 quads / 1332 instances / 641 instructions | [resource_summary.json](../reports/native/resource_summary.json) |
| Restricted ECL matrix | 21735 attempts: 201 returned, 1224 bounded, 14973 unsupported, 5337 require context; all 38110 owned payloads match | [slice_summary.json](../reports/native/slice_summary.json), [per-case ledger](../reports/native/slice_matrix.tsv) |
| ANM, no RNG supplied | 1151 scripts, up to 600 calls each: 340 complete, 800 bounded, 11 require context, zero invalid; 42 independent timing certificates | [animation_control_summary.json](../reports/native/animation_control_summary.json) |
| Timeline, no world observations | 160 entry/mask attempts, all require context; 2003 payloads checked | [timeline_control_summary.json](../reports/native/timeline_control_summary.json) |
| First spell entry prefix | ID2 Easy selected; supplied GUI gates; one pending actor; first blocker sub0 PC1 offset260 opcode139; no invented RNG/player state | [first_spell_summary.json](../reports/native/first_spell_summary.json), [trace](../reports/native/first_spell_trace.tsv) |
| Particle fixtures | Two 600-frame routes, each with 840 births and 688203 search expansions, independently replayed | [motion_summary.json](../reports/native/motion_summary.json), [fixture assumptions](MOTION_FIXTURES.md) |
| Pinned native source comparisons | Zero mismatches in every reported category | [source_oracle.json](../reports/native/source_oracle.json) |

The ECL matrix is the isolated restricted executor, not an audit of every world
handler. Conversely, supported world motion does not retroactively convert its
isolated `UNSUPPORTED` entries into full executions. ANM's separately seeded
profiles reset seed 0 or 65535 for every script; each completes 350 and bounds 801.
They are component checks, not the shared RNG order of a world.

## Verification and performance status

There are 21 core CTests without private data, plus the optional integrated source
oracle (22 with the pinned reconstruction). The current Release and ASan/UBSan
checks, DAT commands, CI limits and report-refresh procedure are documented in
[Validation](VALIDATION.md). The four opt-in component source oracles are separately
buildable; see [Build artifacts](BUILD_ARTIFACTS.md).

Current data-layout optimizations include contiguous immutable programs, hot/cold
operands, fixed scratch state, thirty-slot call frames, bitset slot selection, CSR
geometry and allocation-reusing search. On the measured x86_64 layout, a call frame
is 304 bytes and a workspace 5520 bytes. Timing results are scoped samples in
[Performance](PERFORMANCE.md), not maximum-performance or real-time guarantees.

## Next required integration

The first complete-world target is now one actual **ID2 Easy spell-practice entry**,
then the rest of IDs2..5, not a longer fixed sub40/41 fixture. The
[first-spell execution plan](FIRST_SPELL.md) records selection, three end-to-end
acceptance gates and the rule to follow this case's first blocker. The owned prefix
now selects the first actor and executes opcode80, retaining the timeline and spawn
transaction at sub0's effect51 request. Its first missing behavior is effect51 pool
allocation with actual ANM/camera/shared RNG inputs. GUI gates in this diagnostic
are explicitly supplied; surrounding background/player/global initialization has
not run. Neither a complete spawn for sub0 nor acceptance gate1 is passed.

The restricted spawn protocol separately passes 6000 comparisons against extracted
SpawnEnemy1/2 bodies with a controlled immediate-ECL boundary. That is evidence for
allocation/post-store ordering, not a new full ECL/world oracle. Continue the same
entry command through the prelude and wrapper EX19; do not replace it with a longer
fixed-emitter fixture.

Then connect child/parent lifetimes, shot gates/allocation, player shots and damage,
callbacks and genuine endings. [Coverage roadmap](COVERAGE.md) defines the full
all-case acceptance contract and dependency-ordered work; the
[Wriggle world contract](WRIGGLE_WORLD_CONTRACT.md) supplies exact source identities.
None of those integration milestones is currently marked complete.
