# Current status

2026-09-12: native resource coverage, scalar execution, and launch kinematics expanded.
There is no Python dependency. Verified complete offline spell solutions: 0.
The game is not launched in this phase.

## Completed

1. Preserve original preparation materials; exclude game data and local caches from Git.
2. Native DAT reading, decryption, decompression, and member hashes; ECL subprogram,
   instruction-boundary, jump-target, and spell-source parsing.
3. Restricted native emitter scheduling and all 21735 entry/mask attempts, with
   unsupported behavior and missing context exposed explicitly.
4. Bullet/laser geometry, owned CSR snapshots, finite-horizon planning for
   candidate-independent models, and independent path replay.
5. Actual sub40/41/42 regressions, source-function predicate comparisons,
   unit tests, and consistent clang-format formatting.
6. Seven Release CTest cases pass. Six ASan/UBSan unit cases, actual DAT decoding,
   and the complete restricted-execution matrix also pass under sanitizers.
7. All observed ECL payload schemas; 32 timelines / 2003 instructions; eight SHT files
   / 50 levels / 227 shot descriptors; 113 ANM files / 310 entries / 1151 scripts /
   1917 sprites / 15966 nonterminal instructions; 18 STD files / 68 objects / 552 quads /
   1332 instances / 641 instructions. Native field comparisons cover SHT headers and
   descriptors, ANM sprites and instructions, STD geometry/instances/instructions,
   and timeline offsets/times/opcodes/masks. Resource execution remains separate.
8. Integer/float arithmetic, trigonometry, point geometry, twelve conditional branches,
   and normal call-stack restoration with explicit unknown-context propagation.
9. Nine-mode launch kernel and 180000 source-comparison cases with no mismatches.
   This is the modern-port float32 velocity profile, not verified retail x87 equivalence.

The native matrix has 201 returned slices, 1224 bounded prefixes, 14973 unsupported
attempts, and 5337 attempts requiring context. This executor implements only the
first verified subset; its counts must not be mixed with the broader historical
Python interpreter's results. Per-case records are under `reports/native/`.

The local environment is Linux x86_64, AMD EPYC 7B12, GCC 12.2, Release, without
fast-math. The first full decode/audit took about 0.71 seconds. Reused-workspace sub40
scheduling initially measured a median batch average of about 9.7 microseconds;
after integer-range validation, a repeat measured about 13 microseconds. Removing
those checks has not been justified. Expanded execution measured about 18.8 microseconds
in a concurrent validation run; this is not a controlled performance regression result.

The synthetic geometry benchmark uses 1536 hazards and 20000 queries. One run
measured about 167 ms scanning versus 1.33 ms with CSR; narrow checks dropped from
26709093 to 72853. These are local component measurements, not full-world solving
costs or real-time guarantees. Generated JSON records describe their own runs and
can differ from these initial snapshots.

## Next implementation sequence

| Order | Concrete work | Acceptance |
|---|---|---|
| 1 | Extend source-oracle coverage for scalar execution and exact numerical profiles | Independent per-opcode comparisons; distinguish modern float32 and retail x87 |
| 2 | Timeline/ANM execution, callback and child-context lifetimes | Explicit ownership and clocks; no invented external defaults |
| 3 | sub40/41 emission expansion and frame-by-frame float32 bullet motion | Requests to actual batches to per-frame geometry, with fixed-input comparisons |
| 4 | Stage-one sub0 feedback and familiar alignment/shot gates | Correct state transitions under different player trajectories |
| 5 | Reisen collision windows, Double Spark, and three barriers | Separate visibility and collision, correct phases, independent predictions |
| 6 | Rising and Hourai Jewel RNG, lifecycle, and asynchronous re-aiming | Actions may change the world; cached predictions invalidate correctly |
| 7 | Assemble complete offline phase worlds and solve routes | Cover source/difficulty/entry-state variants; preserve counterexamples and limits |

The indices define the all-case work queue; each item still requires behavior
implementation and verification. Online input control, game launch, and latency
integration follow only after the offline components mature.
