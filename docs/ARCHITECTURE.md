# Architecture and correctness contracts

[Status](STATUS.md) owns current capabilities; [Scenarios](SCENARIOS.md) owns exact
checkpoint/profile semantics; [Validation](VALIDATION.md) owns reproduction/evidence.
Do not infer complete-world behavior from a supported parser, opcode or kernel.

## Responsibilities and code map

| Responsibility | Owner |
|---|---|
| Resource decoding and formats | `resources.hpp`, `resources.cpp`, `formats.cpp`, `schema.cpp` |
| Immutable ECL programs and resumable scalar execution | `emitter.hpp`, `emitter.cpp` |
| Timeline and enemy/world motion | `timeline.*`, `enemy_motion.hpp`, `world_motion.*` |
| ANM execution and timing certificates | `animation.*`, `animation_control.cpp` |
| Bullet/laser kernels | `kinematics.hpp`, `bullet_motion.hpp`, `acceleration.hpp`, `transform_program.hpp`, `laser_motion.hpp` |
| Circular bullet-slot selection | `bullet_slots.hpp`; selects slots, not a complete storage/lifecycle owner |
| Geometry/index and fixed-model search | `geometry.hpp`, `planner.hpp` |
| Synthetic continuous world and runner | `scenario.hpp`, `scenario.cpp`; separate `th08_scenarios` library |
| Controlled real-DAT ID179 world and solver | `spell_scenario.hpp`, `spell_scenario.cpp`; separate `th08_spell_scenarios` library |
| CLI/report serialization | `tools/scenario_cases.cpp`, `tools/spell_cases.cpp` |
| Older entry/effect/camera integration | `practice_entry.*`, `practice_camera.*`, `effect_pool.cpp`, `effect_animation.cpp`, `camera_particle.hpp` |
| Independent source extraction | `tools/*_source_probe.cpp`, shared `source_probe_support.*`, `tests/source_*_cases.hpp` |

Paths above are under `include/th08/` or `src/` unless qualified. Keep the generic
resource library independent of case-specific adapters. CLI code must not own hidden
simulation state. Avoid introducing a general plugin/scheduler framework until actual
profiles demonstrate that a small shared interface removes duplication.

## Program, state and context ownership

Compile resources once into immutable programs with owned full payloads. Hot scalar
operands and cold payload bytes have separate storage; long instructions must not be
truncated. Executions/call frames borrow program code, so the program must outlive
world copies, forecasts and replay. Mutable registers, timers, RNG, particles and
transform state belong to each world/checkpoint, never a global candidate cache.

ECL context snapshots contain thirty context scalar slots. Entity/global/shared
storage is not context-local and must not accidentally be copied/restored by a call.
The spawn helper supplies only the source-proven 46 template zeros; unknown fields
remain explicitly unknown. A compact native representation is not the packed retail ABI.

World-effect handoffs stop before operand/RNG consumption. A handler validates and
performs its specific effect, then acknowledges its token once. Unsupported behavior
must remain a failure, not a blanket acknowledgement. Repeated source operand reads
can mean repeated RNG consumption; do not deduplicate them as an optimization.

Normal ECL calls, child contexts and spawned enemies are different mechanisms. Newly
installed child contexts execute after the main context, in slot order, in the same
update. Spawned enemies can execute immediate ECL before parent linking. Difficulty
mask semantics differ: ECL uses containment; timeline selection uses bit intersection.
Only implement/claim the scheduling domain actually owned by the caller.

## Frame phases and RNG

Keep emission, transform installation, acceleration, displacement, culling and collision
order explicit. A certified fast-spawn bullet can activate and perform fired motion in
its final spawning update. Do not advance transform clocks prematurely at creation.
Velocity calculation and actor displacement are separate source phases with possible
intervening effects. A kernel pass does not establish global manager scheduling.

Gameplay and controlled visual RNG are separate named channels in the current profiles.
Record seed/state and hook policy. Visual isolation intentionally changes retail shared
RNG semantics; it does not justify changing gameplay randomness. If player actions can
change aiming, rank/form, damage, cancellation, pools or RNG, their future must be owned
per candidate or otherwise proven equivalent before sharing it.

## Search and replay

`solver::Model` is fixed collision geometry after each frame's action/world update.
Only candidate-independent models accept exact-position successor merging. The planner
keeps the explicit comparator-winning predecessor, caches blocked as well as safe
successors, and charges every attempted action to its expansion budget. Positions with
different float bits are not approximately merged.

Rolling execution commits a bounded prefix, retains actual world state and replans.
Synthetic geometry storage is bounded by horizon times live hazards; the action tape
grows by one byte/frame. Optional recovery selects a geometry-derived refuge and uses
only the remaining decision budget. It is a heuristic, not completeness/optimality.

Regenerate every executed route or failure prefix from its original checkpoint and use
unindexed collision checks. A search failure is distinct from player death; never invent
a fallback input. A successful horizon is not a complete scene. Replay using the same
world implementation verifies determinism/index/search integration, not independent
physics correctness. Projection digests are diagnostics, not cryptographic proofs or
complete serialized world states.

## Numerical traps already guarded

Retain the small regressions and independent source comparisons for these behaviors:

- Tiny-vector normalization uses the reviewed `1e-8` threshold, not merely nonzero length
- Coincident-position player aiming follows the source fallback (`pi/2`), not plain atan2
- Fractional transform clocks and reset/firing order matter; integer-only probes are insufficient
- Laser terminal dimensions can be signed; clamping to zero changes collision boundaries
- Timeline event slots can be broadcast state rather than a single consumed token
- Float operation ordering, signed zero/domain bounds and exact-contact inequalities matter

The numerical profile is modern native float32 with contraction disabled. Do not claim
retail x87/Direct3D equivalence from these tests. Use source hashes in the maintained
probe generators when investigating a discrepancy, rather than copying unverified
historical numeric manager priorities into a new world owner.

## Older fixtures and entry code

The existing Wriggle sub40/41 fixtures use `ecldata1.ecl`, mask 8, a fixed emitter at
(192, 96), empty transforms/pool/cursor 0 and no rank/gates/cancellation. Each instantiates
840 type 2 bullets, below 1536 total allocations, and checks 600 frames. Type 2 uses ANM
main script 2, fast-spawn script 21 and full collision size (4, 4); player movement/hurtbox
come from `ply00a.sht`. They do not include parent/familiar/world lifecycles.

Earlier Wriggle practice/camera/effect work is retained component code and evidence.
It is not the active solver roadmap. Detailed historical structural predictions and
entry investigations remain recoverable in Git history; they must be revalidated if
that integration is revisited. Do not use a literal callback operand as the effective
callback without accounting for wrapper inheritance and reset instructions.
