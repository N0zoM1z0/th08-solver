# Current status

2026-09-12: source-verified ANM scalar control and enemy/world movement integration expanded.
There is no Python dependency. Verified complete offline spell solutions: 0.
The game is not launched in this phase.

Reusable scratch drivers are now maintained through opt-in `source_enemy_motion`,
`source_world_motion`, and `source_camera_particle` CMake targets. Their generators
and cases are tracked; ignored build translation units remain generated outputs.
Fresh Release and ASan/UBSan builds reproduce the existing comparisons with zero
mismatches. See [BUILD_ARTIFACTS.md](BUILD_ARTIFACTS.md) for the local-file audit and
reproduction commands. This packaging change adds no world-execution coverage.

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
6. Nineteen Release and eighteen ASan/UBSan CTest cases pass. Sanitizer checks also cover the
   native parsers, restricted-execution matrix and both 600-frame particle fixtures,
   including the latest signed-laser/fractional-clock changes.
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
10. ANM lifetime certificates, spawning-to-fired motion, relative/absolute/aimed turns,
    cull delay and offscreen lifetime. Two sub40/41 component fixtures each instantiate
    840 bullets, generate 600 collision phases, and find independently replayed paths.
11. Laser starting/active/despawning collision-call projection and fractional clocks.
    The source oracle compares 584936 direction frames and 678369 laser frames without
    mismatches. Signed terminal dimensions have a minimized fail-before/pass-after guard.
    Direction changes now use the actual fractional source clock in both implementation
    and oracle; the previous integer adapter did not verify non-unit timing. A separate
    half-rate fail-before/pass-after regression records that correction.
12. Allocation-reusing deterministic heap search, with identical results against the
    previous implementation on 80 scenes. A local alternating benchmark measured about
    32.7 ms versus 22.1 ms median; this is a component workload, not maximum performance.
13. Explicit shared RNG state, seed backup, draw counter and integer/float range helpers.
    All 65536 seeds pass 524288 native source comparisons with no mismatches. This
    does not establish original executable evaluation order or whole-world draw order.
14. Optional shared RNG for isolated ECL scalar execution, random selectors and sign
    assignments. Another 720896 source-backed assignment comparisons pass. Multiple
    RNG expressions per instruction remain unsupported; RNG-enabled execution stops
    before unresolved shot effects. The original matrix and both particle fixtures pass
    unchanged. Completed-prefix and failed-instruction RNG ownership are tested.
15. Allocation-free deceleration, vector and polar acceleration kernels, plus vector
    installation. The oracle compares 256979 acceleration frames at changing frame
    rates with the pinned methods, vector operators and real timer, without mismatches.
16. Eighteen-record transform scheduling for acceleration, shared direction changes,
    bounce/wrap, wait, cull delay, sound and despawn requests. Another 120748 source comparisons
    cover installation, gates, overlapping effects, fractional wait clocks and sound
    order. Both particle fixtures now use this executor for birth-time installation,
    with unchanged frame and route files. Sprite replacement, child
    patterns, complete despawn lifecycle and pool ownership remain unsupported.
17. Bitset bullet-slot selection preserves the source circular scan and delayed cursor
    completion across nested spawns. All single-hole positions and 300000 source-backed
    reserve/release/completion operations pass. A selection-only near-full benchmark
    measured about 1198 ns for a compact bool scan versus 19.5 ns for the index; this is
    not full-world throughput. Actual bullet storage and allocation transactions remain
    the caller's responsibility, not an implemented complete pool lifecycle.
18. Resumable unit-rate ECL contexts preserve call/wait state and can yield before
    world instructions without consuming operands or RNG. Independent forks and
    same-frame resumption are tested, including world-side RNG between assignments.
    Complete owned instruction payloads retain long world commands; all 38110
    compiled payloads (including terminal records) match the DAT byte-for-byte.
    The 21735-entry legacy matrix and both particle fixtures remain unchanged.
    This adds the execution boundary, not fractional ECL clocks, child contexts or
    the callback frame tail. Movement handlers now consume that boundary as below.
19. ANM lifecycle control executes jumps, waits, stop/hide/static/delete, sprite
    replacement, visibility, interrupts/returns, typed scalar arithmetic, comparisons,
    masked operands, explicit RNG and player-shot hit-animation selection. The pinned
    control blocks match across 48224 frames; scalar/accessor comparisons add 228669
    calls without mismatches. All 1151 DAT scripts are audited for 600 calls: without
    RNG, 340 complete the projection, 800 remain bounded and 11 require context;
    none stop as unsupported or invalid. Independently seeded fixtures (0 and 65535
    reset per script) each yield 350 completed / 801 bounded; these are not whole-world
    draw-order claims. All 42 timing certificates independently agree, including
    separate frame-30000 checks. Rendering, visual interpolation, sprite resources
    and world lifecycle integration remain separate; ANM completion is not a spell.
20. Allocation-free enemy polar, interpolated and orbital movement preserves source
    local/world coordinates, mirroring, clamping, parent offsets and fractional clocks.
    Velocity update and position integration remain separate around the shot/ANM phase.
    Another 580000 source phases pass bitwise comparisons, including changing rates.
21. Fifteen ECL movement effects (63..76 and 178) now apply
    transactionally to the same enemy and scalar storage used by resumable execution.
    Typed packed/cold operands share the scalar resolver. Source-ordered assignments,
    live computed-field reads and repeated RNG reads are preserved; ambiguous pairs
    of random factors still stop explicitly. The pinned effect/operand blocks match
    across 420868 effects, with twelve additional native atomic-failure checks. Random
    boundary/bias movement includes all seeds, strict/overlapping margins, previous-angle
    reads, wrapped-distance ties and the no-player branch of opcode 178. Coincident
    player aiming has a minimized fail-before/pass-after regression. Missing context
    leaves enemy, workspace, pending instruction and RNG unchanged. This does not
    implement the enemy manager, timeline, spawning, shot dispatch or ending lifecycle.
22. Source-faithful timeline control owns complete payloads, exact fractional clocks,
    pending effect tokens and explicit gate/event context. All 32 timelines / 2003
    payloads match the DAT. Another 200000 source frames cover masks, stale instructions,
    boss/message/event waits, event broadcast/consume, ordered effect boundaries and
    repeated sentinel ticks. The real ID2..5 practice timeline fixture yields sub0,
    sub42 and retry-menu requests, with five explicitly synthetic boss-wait frames.
    The 160-entry no-world-context baseline stops honestly at `REQUIRES_CONTEXT`;
    no spawns or completed boss worlds are inferred from those requests.
23. Effect 51 camera-particle callbacks preserve sixteen U16 initialization draws,
    source-order motion/culling, explicit camera/boss/ANM fields and stage tint.
    All 65536 seeds, 328503 updates and eight atomic-failure checks match extracted
    source bodies. A fault-injected tiny-vector normalization regression fails as
    expected and the maintained implementation passes. Successful culls commit motion
    but skip later world reads. Allocation, surrounding ANM execution, camera evolution,
    freeze scheduling and effect-pool retirement remain unimplemented world work.

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
| 2 | Connect timeline/ANM execution, callback and child-context lifetimes | Explicit ownership and clocks; no invented external defaults |
| 3 | General transform-program execution and pool lifecycle | Extend the verified sub40/41 particle subset without inventing future state |
| 4 | Stage-one sub0 feedback and familiar alignment/shot gates | Correct state transitions under different player trajectories |
| 5 | Reisen collision windows, Double Spark, and three barriers | Separate visibility and collision, correct phases, independent predictions |
| 6 | Rising and Hourai Jewel RNG, lifecycle, and asynchronous re-aiming | Actions may change the world; cached predictions invalidate correctly |
| 7 | Assemble complete offline phase worlds and solve routes | Cover source/difficulty/entry-state variants; preserve counterexamples and limits |

The ANM instruction blockers in the previous baseline are resolved within the control
projection. The remaining 11 unseeded entries require the actual shared RNG context,
not arbitrary seeds. Next connect resource identity and lifecycle consumers without
discarding visual state that those consumers read. The first complete-world path
also needs concrete world observations for timeline gates, spawning/child contexts,
effect allocation/ANM/camera lifetimes, damage and callbacks. Do not infer world completion
from this queue. Effect 51's callback projection supplies only one part of that lifecycle.

The indices define the all-case work queue; each item still requires behavior
implementation and verification. Online input control, game launch, and latency
integration follow only after the offline components mature.

The first complete-world integration target is the actual ID2..5 practice entry,
not a longer fixed-emitter fixture. [WRIGGLE_WORLD_CONTRACT.md](WRIGGLE_WORLD_CONTRACT.md)
records its source-derived wrapper, familiar/child-context closure, effect RNG,
damage, callback and ending requirements. A native comparison confirmed 175 stage
and practice records after explicit sub-ID remapping; their entry states differ.
