# Provenance and verification

This is the evidence map for the current components. [Status](STATUS.md) separates
those capabilities from complete worlds; [Validation](VALIDATION.md) gives the
current commands and distinguishes public CI from private-DAT/source checks.

## Pinned inputs

- DAT: 46,838,025 bytes; SHA-256 `9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb`.
- Reconstruction: [N0zoM1z0/th08](https://github.com/N0zoM1z0/th08), commit `a45e99fb1942714e6edded20847e32a654d56f97`.
- `src/Player.cpp`: `80c6829a41a30fcce47837edaa8da90bb11779130b5c443db842c7623745242c`.
- `src/Global.cpp`: `8df17616c935d684b6636619d4726889e68f7d2d7000e27c25aebc4bc460b74b`.
- `src/Global.hpp`: `ce49422a53e5ba33b63d803d17e7051ba2a5ad7a33ae531910c048a091f37592`.
- `TH08_AllCase_20260911.zip`: `8913fffc96824c27b681ff1b1133a4385f7c7ca3e8e377fede17e552e535b199`.

Older reports also refer to a dirty worktree near `af72ca9...`; that evidence must
not be conflated with the pinned source above. Original preparation files remain
unchanged. Their code is not automatically promoted into a verified component.

## Comparison chain

| Behavior | Reference | Current check | Excluded |
|---|---|---|---|
| DAT decoding | PbgArchive, Lzss, FileSystem::Decrypt | All 317 decoded member hashes match the historical reproduction | Gameplay semantics |
| ECL structure | EclManager.hpp and historical audit | 24 files, 1449 subs, 36661 instructions, 2182 jumps, 431 spell starts | Complete VM execution |
| Boxes and lasers | Pinned Player.cpp function bodies | 300000 scenes, 600000 predicate comparisons, alive/death side-effect checks | Game loop and x87 equivalence |
| RNG state | Pinned Global.cpp / Global.hpp bodies | All 65536 seeds, 524288 integer/float operations, bitwise outputs and final seeds | Original executable evaluation order; complete world draw ordering |
| ECL random assignments | Pinned operand case blocks and EclRunLow assignment bodies | 720896 assignments over all seeds, typed conversion and final seed checks | Multiple RNG expressions in one instruction; external RNG consumers |
| Context scalar ownership | Pinned context layout, spawn-template initialization and context call/copy rules, inspected statically | Native all-slot, source-order, unknown-validity, signed-zero and nested-call tests; unchanged real-data matrix | No new full-context source-function oracle, callbacks, actor allocation or spawn lifecycle |
| Launch kinematics | Pinned launch switch and angle normalization | 180000 nine-mode cases with bitwise velocity and draw-count checks | Rank/gates, allocation and complete spawning |
| Direction changes | Pinned Bullet methods and actual ZunTimer | 584936 frames, including fractional ramps and firing/reset boundaries | Full bullet lifecycle and retail x87 |
| Laser collision/lifetime | Pinned laser update loop and actual ZunTimer | 678369 frames with ordered collision calls, signed dimensions and retirement | Rendering, world gates and complete spell context |
| Acceleration updates | Pinned Bullet methods, Float3 operators, VectorAngle and ZunTimer | 256979 frames over three modes and changing frame rates | Retail x87; complete lifecycle |
| Transform scheduling | Pinned payload layouts, AdvanceTransformProgram, fired dispatch block, update methods and playfield predicate | 120748 birth/update steps, including simultaneous effects, shared turns/wrap clocks, bounce thresholds, wait decrement and sound order | Sprites, child patterns and complete lifecycle |
| Bullet slot selection | Unchanged selection loop and final cursor-update block from SpawnSingleBullet | 300000 reservation/release/completion operations, including nested cursor completion | Launch RNG, storage initialization, cancellation and complete pool lifecycle |
| ANM lifecycle/scalars | Pinned ExecuteScript control/scalar blocks, all four typed accessors, opcode/variable enumerations, actual RNG and ZunTimer | 48224 control frames and 228669 scalar calls; 1151-script unseeded and explicit-seed audits; 42 timing certificates | Bytecode-writing destinations, visual interpolation/rendering, resource-loading effects and world lifecycles |
| Enemy motion phases | Pinned movement/configuration methods and manager integration block | 580000 configuration/velocity/integration phases, fractional clocks, easing, mirrored/inverted bounds and parent coordinates | Actor creation, lifecycle gates and intervening shot/ANM execution |
| ECL movement effects | Pinned movement opcode blocks, helpers, typed motion/player/RNG selectors, world publication and player-angle/vector-length bodies | 420868 effects, including all-seed random movement, repeated RNG reads, self-reading fields and coincident-position aiming; twelve separate native rollback checks | Multiple random factors in one unsequenced product, complete world scheduling |
| Timeline control | Complete pinned EclTimeline::Run, GUI predicates and actual ZunTimer | 200000 frames; all 32 timelines / 2003 owned DAT payloads; practice-entry handoff fixture | World effects, ungated random spawn execution, complete boss lifecycle |
| Owned spawn prefix | Pinned SpawnEnemy1/2 bodies with a controlled immediate-ECL boundary; static RunEcl/template/interaction rules | 6000 source-order comparisons; native pending/fork/full/failure tests; four DAT masks stop at sub0 PC1 offset260 | Full RunEcl reference, effect51 ownership, surrounding world phases, complete entry or spell |
| Effect 51 camera callbacks | Unchanged initializer/update, source Float3 operators, HasBoss, color fields and modern D3DX math | 65536 initializations / 328503 updates / eight native rollback checks; all seeds, color byte products and culling thresholds | Allocation, surrounding ANM, camera evolution, freeze/pool lifecycle and retail Direct3D math |
| Spatial index | Unindexed hazard scan | Random scenes, cell boundaries, exact contact, snapshot ownership, invalid arguments | Formal proof for all float inputs |
| Wriggle scheduling | Actual DAT and historical event digests | sub40/41 ordered digests, 360 ticks, 160 commands, 840 requests; sub42 alignment variants | Successful allocation, bullet motion, complete spells |
| Planning | Explicit collision-restoration fixture | Legal actions, terminal region, unindexed replay, budget failure without a route | Reisen gameplay or complete search |

Reference functions are extracted only into the optional test build directory.
The scalar adapter also pins `EclManager.hpp`, `EclOperandsInt.cpp`,
`EclOperandsFloat.cpp` and `EclRunLow.inl`; their hashes are enforced in
`tools/ecl_source_probe.cpp`. Only selected random cases and assignment bodies are
extracted. Local-storage adapters are fixture scaffolding, not complete enemy layouts.
The ANM adapter pins `AnmManager.cpp` (`c82bb37c19af4ccaabfa4bf4606d92c72e180f5f2fdd642cf3ec2131c85cecce`),
`AnmManager.hpp` (`df96ae2abd43ffc64a5967451fcd3ca5b83b75f6ad6ed37ba852370855c7f582`),
and base initialization in `AsciiManager.cpp`
(`86c0d3cca5040036f16de762e80b3126b7037c89b526044cbb74bcc4bc6abdb1`).
It retains exact control/scalar blocks, typed accessors and the final script-clock
tick, not the omitted render interpolation tail. Its sprite adapter records identity
only. Unsupported reference inputs throw instead of pretending to execute visual
commands. The default 600-call DAT profile completes 340 scripts, bounds 800 and
stops 11 for missing RNG; no seed is fabricated. Separately, explicit seeds 0 and
65535, independently reset per script, each complete 350 and bound 801. Those
profiles verify component execution, not world draw ordering or spell completion.

The enemy adapter additionally pins `EnemyManager.cpp`
(`e8febe94a833472b33f732e83ee39ee48fdc5097c5d69ff094fd1f1bb8629a7d`),
`EnemyManager.hpp` (`e56633232cfb8e0934fb9e83f592989b577cd045e623df0c2c294eed9b2bf256`),
`EnemyManagerUpdate.cpp` (`5692ab3214e95873626e6ab896f867746217b0556b34737c2556e1b38a454e59`),
and `EclHelpers.cpp` (`64a9318a9a3b89d02f221b1837e618c027c3a7814ed43481a0ca78a5c0b77f73`).
It preserves the separate velocity and manager integration phases. Literal helper
adapters are fixture scaffolding; they do not establish ECL operand/RNG order.

The movement-effect adapter covers that separate boundary using unchanged
`EclRunLow.inl` instructions 63..76 and 178, configuration helpers and selected
original integer/float operand cases. It also pins `EclRun.cpp`
(`010049211263e47d8245c7335f56b17a8502ca0f84595c8b035926a495d90b57`) for world-position
publication, and `modern/linux/d3dx8_compat.cpp`
(`8e9649ef554dcc2ac7d0218974a48d4583bae32591f4a7cd5b16ecdca3b62388`) for vector length.
The actual `Player::AngleToPoint` distinguishes coincident x/y from plain atan2;
see [Regressions](REGRESSIONS.md). Repeated source operand evaluations remain
repeated, including random speed reads for separate polar components. Tests exclude
ambiguous multiple-random products rather than assert an unverified compiler order.
The random movement helpers also pin `EclDependencies.cpp`
(`019f9cd6abdb73223d3d41cc8a6317641e6fe6bfbd7777d126a4bace3e14e2e4`).
All 65536 seeds enter timed/untimed and boundary/bias comparisons; exact-margin,
overlapping-boundary and missing-player branches distinguish source behavior.
The twelve failure-atomic checks enforce a native ownership contract, not source-engine
rollback semantics. No extracted test adapter implements the complete enemy layout.

The timeline generator pins `EnemyTimeline.cpp`
(`920ee34725aa6aad9f113d43454731acadab456abddac73256b2ba9a29e8e94b`), `Gui.cpp`
(`bd053c070d1ce136910e163898e6f97f4c00683558051d1a953967c6c5c89af8`) and
`AsciiManagerScale.cpp` (`b439ca540148240df69319277721cad4638a5422d7f98efc7a873e2fdcda62df`),
in addition to the existing enemy definitions and real timer. External method
adapters record invocation order; ungated random spawn execution throws outside
that comparison domain. No actual GUI or game loop is launched.

The camera-particle generator additionally pins `EffectManager.cpp`
(`63d45a213956008b44874bc4707c971a7799a9c551b07e732bf1f55282c2209e`), `Background.hpp`
(`bbfa9022f52c5b5332f8e690d42c7338ec97f062b43a3bfcd6dc33190484efe8`), `ZunColor.hpp`
(`e8561d8b0f2770566bc0658b9ad2e8cb2dfbebc7fcea1cf72a59b80487503af2`) and
`modern/linux/include/d3dx8.h`
(`c9f5b34bf60903919726bfed0c2055b0c203ea630994a13bb5434925b56dd20e`).
Its inputs are explicit callback-boundary snapshots, not invented allocation or
camera defaults. The modern vector normalization threshold is independently guarded;
it does not establish equivalence to the retail Direct3D implementation.

The reference checkout is not modified. Competing laser interpretations differ on
whether player extents also rotate; production function output supports center-only
rotation. Snapshot ownership tests distinguish a stable owned index from a borrowed
container that changes after construction, including copy/move cases.

Any decoded member hash mismatch, source-predicate mismatch, or replay collision
invalidates the corresponding model. An aggregate pass rate cannot hide such a failure.

Compact context storage is a source-layout-guided implementation with native
ownership/integration regressions, not an additional extracted full-context oracle.
Its exact source hashes and excluded context fields are recorded in
[ECL context storage](ECL_CONTEXT_STORAGE.md). The complete Wriggle
[world contract](WRIGGLE_WORLD_CONTRACT.md) is static source evidence with a checked
175-record structural prediction, not a runtime proof that its world executes.

## Attribution and distribution

The reconstruction uses MIT; see [Third-party notices](THIRD_PARTY_NOTICES.md).
Game DAT/EXE files, audiovisual assets, and extracted game content are not distributed.
Preparation artifacts are tracked at the user's request and retain their original
language and attribution. This project does not assign them a new license.

The Nitori workflow informed input pinning, competing mechanism checks, and independent
comparisons. Its commit trailer records workflow assistance, not human authorship.
