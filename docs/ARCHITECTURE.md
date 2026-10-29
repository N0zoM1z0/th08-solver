# Architecture and correctness contracts

[Status](STATUS.md) owns current capabilities; [Scenarios](SCENARIOS.md) owns exact
checkpoint/profile semantics; [Validation](VALIDATION.md) owns reproduction/evidence.
Do not infer complete-world behavior from a supported parser, opcode or kernel.

## Responsibilities and code map

| Responsibility | Owner |
| --- | --- |
| Resource decoding and formats | `resources.hpp`, `resources.cpp`, `formats.cpp`, `schema.cpp` |
| Immutable ECL programs and resumable scalar execution | `emitter.hpp`, `emitter.cpp` |
| Timeline and enemy/world motion | `timeline.*`, `enemy_motion.hpp`, `world_motion.*` |
| ANM execution and timing certificates | `animation.*`, `animation_control.cpp` |
| Bullet/laser kernels | `kinematics.hpp`, `bullet_motion.hpp`, `acceleration.hpp`, `transform_program.hpp`, `laser_motion.hpp` |
| Circular bullet-slot selection | `bullet_slots.hpp`; selects slots, not a complete storage/lifecycle owner |
| Geometry/index and fixed-model search | `geometry.hpp`, `planner.hpp` |
| Synthetic continuous world and runner | `scenario.hpp`, `scenario.cpp`; separate `th08_scenarios` library |
| Controlled real-DAT ID 179 world and solver | `spell_scenario.hpp`, `spell_scenario.cpp`; separate `th08_spell_scenarios` library |
| CLI/report serialization | `tools/scenario_cases.cpp`, `tools/spell_cases.cpp` |
| Older entry/effect/camera integration | `practice_entry.*`, `practice_camera.*`, `effect_pool.cpp`, `effect_animation.cpp`, `camera_particle.hpp` |
| Independent source extraction | `tools/*_source_probe.cpp`, shared `source_probe_support.*`, `tests/source_*_cases.hpp` |
| Imported production game and headless session | `third_party/th08/src/`; `modern/headless/session.*`, CPU resource/time/input adapters |
| Exact headless archive field decoding | `modern/headless/lzss_decode.hpp`; borrows the native persistent dictionary and disjoint caller buffers; changes neither encoder nor non-headless decoder |
| Exact packed headless surface copies | `modern/linux/d3dx8_compat.cpp`; copies complete equal-format owned CPU surfaces, retaining conversion/snapshot paths for other cases |
| Native proposal kernels | `include/th08/reactive.hpp`, `native_policy.hpp`, `direct_laser_policy.hpp`, `imminent_laser_policy.hpp`, `upcoming_bullet_policy.hpp`; actual game establishes collisions/outcomes |
| Native spell policy selection | `include/th08/spell_policy.hpp`; case IDs do not enter generic kernels |
| Native execution/tapes/reports | `tools/headless.cpp`; opt-in `cmake/Headless.cmake` |
| Native fork planner, proposals and process checkpoints | `tools/plan.cpp`; `plan_contracts.hpp`, `plan_observation.hpp`, `plan_beam.hpp` |
| Owned current-memory observations and online proposal history | `online.hpp`, `online.cpp`, `online_model.cpp`; the optional previous-plan cache belongs to each `Policy` and is rechecked against new observations |
| Immutable stage/spell proposal strategies | `phase_strategy.hpp`, `phase_strategy.cpp`; parse actual stage/card selectors and pixel/update units. `Configuration::phase_capture_prior` shares only a capture-form preference with independent recovery; it neither adds scoring memory nor owns a gameplay world |
| Bounded strategy archive and selection | `phase_archive.hpp`, `phase_archive.cpp`, `tools/phase_frontier.cpp`; compare declared completed-segment evidence and retain strategy diversity. Request/checkpoint deduplication is not world-state equivalence or a Native safety certificate |
| Common-start shadow synchronization | `shadow_protocol.hpp`, `shadow_native.*`, `shadow_process.*`, `tools/shadow_follow.cpp`; fixed-width wire values and a single-threaded independently initialized follower process |
| True-RNG branch verification and committed action frontier | `shadow_rollout.*`, `shadow_plan.hpp`; isolated native child futures and checked consecutive execution receipts |
| Offline terminal-score refinement | `score_refine.*`, `tools/score_refine.cpp`; complete remaining-tape evaluation, identical capture/route goals and strict terminal-score selection |
| Source capture coverage | `modern/headless/score_audit.*`; one source-catalogue goal shared by reporting and complete-trajectory selection |
| Local Wine observation/input bridge and future proposals | `shadow_bridge.hpp`, `shadow_proposal.*`, `tools/shadow_online.cpp`, `tools/wine-shadow/`; pinned retail ABI adapter, owned launcher and branch-owned proposal history |
| Native observation adapter and online execution/reporting | `online_native.*`, `tools/online.cpp`; no native future updates or live rewinds |
| Replay projection and strict corpus gate | `modern/headless/replay_check.*`, `tools/replay_check.cpp`, `tools/replay_suite.cpp` |
| Sanitized campaign evidence and verified replay export | `tools/planner_report.cpp`; retains original proofs, separate accounting/archive receipts and their costs |
| Sampled hazard stability diagnosis | `tools/hazard_probe.cpp`, `hazard_probe.hpp` |
| Independent native input interventions | `tools/headless_probe.cpp`; serial fresh-process branches over a fixed tape |

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

The native session calls the original ordered calc chain once per supplied input.
Replay fast-forward can restart the chain inside that call; replay calculation rows
and Session API calls are separate counts. The session keeps ANM updates and original
shared RNG consumption, with synchronous initialization and
`timeGetTime = floor(frame * 1000 / 60)`. It omits the draw chain and text rasterization,
not the GUI/message update logic. This profile does not split visual RNG streams.
Initialization and loading completion are explicit adapter boundaries, described in
[Scenarios](SCENARIOS.md); no claim about original menu entry is needed.

`State::calculation_updates` observes the actual `gameplayFrameCounter` increment,
after native setup gates. The observer survives stage counter resets and process forks,
consumes no RNG, and leaves the existing entry trace hook unchanged. Stage-control
calls can return before that increment and all later replay callbacks. Accepted-path
counts/events remain process-private and travel with checkpoint state; only effort
over all branches uses shared atomic counters. [Planner](PLANNER.md) owns the recording
and report contracts for these distinct counts.

In recording mode the supervisor samples controller input at priority 0, Player moves
at 9, BulletManager updates/collides at 14, and ReplayManager latches movement input
at 17. Player therefore reads the previous latch. The headless adapter preserves this
order; proposals must account for the pending input rather than reorder the game.

Within `BulletManager::OnUpdate`, after item updates, a stack-owned
`CancelRegionScope` checks whether all player cancel regions are inactive. That
proof allows each bullet to return the original empty-scan result immediately.
The two source creation APIs invalidate every enclosing scope for that player;
active or invalidated scopes keep the original ordered scan, geometry and hit/item
side effects. Existing lifetime/bomb-pointer updates only alter or deactivate
regions. A new activation writer must invalidate the proof; Debug queries assert
its invariant. The scope belongs to the single calc thread, restores its parent
on exit and never survives a native update/checkpoint. It stores no gameplay state,
advances no clock and consumes no RNG.

`Session::collision()` owns a value copy captured at the successful source collision
test before death effects alter RNG/state. It identifies bullets, lethal regions or
lasers and records the exact checked bounds and input values. Every native
`CalcLaserHitbox` call is also copied into a per-update raw view with its center, size,
origin, angle, graze flag and pooled slot when one exists. This distinguishes transient
ECL-owned geometry from `BulletManager` lasers without moving collision ownership out of
the game. `ecl_contexts()` copies active main/child cursors, raw next-instruction fields
and actor motion state; it does not resolve flagged operands or cross control flow.
`bullets()` returns a
reused post-update view buffer invalidated by its next call; active vector-acceleration
state is copied from its source transform slot without advancing it. For an active WAIT,
the view also exports the bounded number of updates whose copied transform program proves
the current velocity unchanged. Active relative-direction state has a separate source-owned
validity bound through its final turn, excluding subsequent/concurrent enabled transforms;
the optional scorer evaluates its deceleration/turn recurrence without mutating native state.
A separate WAIT/vector view certifies the activation handoff through final vector clear;
the unchanged-velocity bound includes DESPAWN's final lethal fired movement.
`enemy_body_forecast()` returns owned typed per-update endpoint rectangles or an explicit
failure. Its private adapter owns ECL/passive-program/timeline/lifecycle certificates;
`body_math.hpp` owns outward interval arithmetic, while the generic policy only consumes
rectangles along actual candidate paths. Conditional callback immunity is labeled, and
random movement uses all-angle bounds rather than consuming RNG. Unsupported enabled
body forecasts stop the caller rather than silently dropping warnings.
`lasers()` owns a
separate reused buffer of raw pooled-laser lifecycle state. Its motion fields measure the
immediately preceding native update and do not promise that motion will continue. Callers
copy either view when retaining history; the per-update laser-hitbox view is invalidated
by `step()`. `upcoming_bullet_spawns()` owns a separate reused buffer and copies only the
current cursor's bounded deterministic ECL future; a typed unsupported entry stops an
enabled profile instead of representing unknown space as empty. The ECL cursor and each
spawn-preview buffer are independently reused by their next call. The CLI
optionally owns a 32-update before/after ring, serialized after loop timing.

Original managers contain process globals and internal pointers. `headless::Session`
is noncopyable and permits one instance per process. Fresh replay starts another
process. Do not memcpy this state or share one future across alternative actions.

The shadow follower is a separate common-start execution check. Its little-endian
packet contains sequence, epoch, actual input and a declared scalar/digest projection;
it contains no pointers, is identical on 32/64-bit hosts, and is not a save state.
The parent forks before constructing its Session and requires a single-threaded
caller. Each process initializes from the same explicit configuration in its own
owned scratch directory. Only acknowledged consecutive inputs advance the follower.
The first field mismatch is a failure, never an instruction to overwrite RNG.
Epoch advances on an observed stage change, including a control-only handoff.
Stage-clear flags in a story are not terminal until the existing Final/Extra rule.
Transport has a bounded read/write deadline and owns only its child and directory.
Actor/bullet/laser hashes cover their stated exported views, not all latent state;
even matching packets do not establish complete world or retail equivalence.

`shadow::predict` forks only a single-threaded owner, preserving the complete current
world, input latch and true RNG. The child also owns a temporary score/config
directory: copy-on-write memory alone cannot isolate filesystem effects. Its
bounded shared result arena is consumed only after the child exits. Death, native
fault and timeout never certify a prefix; interrupted work is charged its full
requested update cap. Native timing excludes measured file I/O; wall timing includes
setup, copying, fork, capture and waiting. `ActionBuffer` can adopt a late result
only after every actual receipt since its origin matches the predicted prefix.
Execution remains monotonic; an observed fallback clears incompatible queued work.

The optional Wine launcher owns a newly created suspended 32-bit process. Both
launcher and observer verify the original EXE's pinned SHA-256; all three complete
entry signatures are checked before any hook is installed. Initialization runs
outside the DLL loader lock, before the primary thread resumes. A partial install
rolls back or terminates that owned process. The on-disk EXE stays unchanged; its
in-memory calc, gameplay-registration and controller entries are explicitly hooked.
Trampolines remain owned for the process lifetime. No existing game is attached.

The observer records actual configuration/RNG before original gameplay registration.
A named optional checkpoint changes only the selected stage/difficulty/shot/practice
configuration at that boundary. It never sets the gameplay RNG. A fresh Native
Session reconstructs that start and follows actual sampled inputs, preserving the
one-update latch. This is common-start reconstruction, not RNG-only world import.
Its append-only 228-byte journal contains the 204-byte projection and three explicit
nanosecond clocks. Observer timing includes capture but excludes journal WriteFile;
actual inter-update periods include the remaining game/presentation/scheduler work.
The first projection mismatch stops optional exact mode and preserves post-calc bullet words,
source polar arguments and x87 control state for diagnosis. Matching hashes cover
their declared fields, not every latent manager value or complete retail physics.
Live behavioral mode treats bullet/laser digest drift and player differences up to
1/64 pixel as telemetry. Inactive spell/route labels carry no gameplay comparison
requirement until active. Input/RNG/clock/lifecycle/actor/feedback divergence remains
an explicit stop. This comparison changes no world value or collision predicate;
actual NMNB and owner-clock results determine practical acceptance.

The Wine cursor publishes a stage-control receipt even while asynchronous loading
is in progress. It resets only the new stage's observed calculation counter, then
ignores loading callbacks until gameplay resumes. Dropping that control receipt
would compare a new-stage world with Native's earlier control boundary. The live
client selects `InputProfile::SampledGameplay`: at GameManager's input boundary it
applies the supplied final sample after synchronous headless initialization has
cleared transient input. The default Controller profile retains its original
first-new-stage zero sample and all existing tape/replay semantics. Retail replay
playback cannot select the sampled profile. Both preserve the original late
movement latch and the complete native transition's carried/reset world state.

The action file maps aligned little-endian 32-bit words on local x86 Linux/Wine.
GCC lock-free atomics guard two publication slots; the owner copies an accepted
plan privately. It rejects torn/late generations and any leading input inconsistent
with its existing queue/fallback before the declared first change. It never waits
for Native planning. This is local shared-memory transport, not an SSH/network API.
The Native root remains single-threaded. One isolated proposal child advances the
fixed lead and inherited Policy history; it returns only heuristic input vectors.
Separate serial native children verify complete lead-plus-candidate futures. The
root catches up with actual receipts before publishing a remaining suffix. Extra
lead work, failed branches, full-cap timeout charges and wall overruns stay visible.
Child policy tie RNG is private; the actual root only observes. This defines a new
profile and does not reproduce the old per-update policy's search RNG sequence.

`tools/headless_repair.cpp` uses this fresh-process boundary for bounded prefix repair.
The child CLI keeps strict replay distinct from explicit prefix continuation; per-update
policy logs supply source-prefix projections and rollback triggers. The outer process owns
budgets, pinned executable provenance, artifact validation, failure ledger and replay.
`observed_path_policy.hpp` owns only the H32/63-prefix player-path proposal beam.
No native snapshot or global-state copying is introduced by either component.
`th08_plan` separately branches that native state with process forks. Its shared
tapes contain observations and counters; mutable managers remain private to each
branch. `plan_contracts.hpp` rejects incomplete/failed rollouts, while
`plan_observation.hpp` compares committed state/RNG/feedback with verification.
`plan_beam.hpp` owns optional spatial proposal retention; its bins are not world
equivalence classes. See [Planner](PLANNER.md) for verification and backtracking.

`include/th08/online.hpp` defines a separate owned current-memory observation and
CPU policy. `src/online_native.cpp` copies native views and permitted source fields
without retaining their pointers or stepping a Session. `src/online.cpp` owns
per-instance observation history, forecast/search buffers and tie RNG;
`src/online_model.cpp` parses immutable risk tables before execution. The policy
library does not link the native runtime. `tools/online.cpp` owns the single live
Session, checkpoint/prefix reconstruction, route inputs, clocks, action tape and
report. No forked future or rewind enters this controller. Current-memory forecasts
are heuristic and actual native execution can reject them by death. Optional actor
origins and bounded current-program shot prefixes are copied by the adapter; their
native storage never enters the policy. Unknown preview coverage is explicit.
Online and offline searches share the typed spatial/focus proposal-retention helper;
each search owns its nodes and records selection costs. See
[Planner](PLANNER.md#current-memory-online-policy) for the observation, history,
deadline and evidence contracts.

Keep emission, transform installation, acceleration, displacement, culling and collision
order explicit. A certified fast-spawn bullet can activate and perform fired motion in
its final spawning update. Do not advance transform clocks prematurely at creation.
Velocity calculation and actor displacement are separate source phases with possible
intervening effects. A kernel pass does not establish global manager scheduling.

Synthetic and controlled-DAT profiles may define separate named gameplay and visual
RNG channels. Record their seed/state and hook policy. The imported Native profile
keeps the source shared RNG; it does not use that visual split. Visual isolation
changes retail RNG semantics and does not justify changing gameplay randomness. If player actions can
change aiming, rank/form, damage, cancellation, pools or RNG, their future must be owned
per candidate or otherwise proven equivalent before sharing it.

## Search and replay

The online native adapter optionally reads a bounded current ECL byte batch into
a scalar turn cue (`ecl_turn_hint.hpp`). It copies only the resulting value into
an owned queued laser; no native pointer enters the policy. The pure decoder
never evaluates variables or RNG, and unsupported/bounds-limited reads remain
counted. `online.cpp` owns the optional two-sign angular proposal envelope and
its radial index bound. This does not replace native collision or establish
future pose/lifetime validity.

For `--relative-turns`, the adapter copies the native session's bounded relative
direction view and current repeat state into `online::RelativeTurn`. The policy
uses the existing pure bullet kernel within that source bound, then labels its
linear continuation as a heuristic. No actor or native VM pointer crosses this
interface, and aimed/concurrent transforms do not enter the independent model.

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

The existing Wriggle sub 40/41 fixtures use `ecldata1.ecl`, mask 8, a fixed emitter at
(192, 96), empty transforms/pool/cursor 0 and no rank/gates/cancellation. Each instantiates
840 type 2 bullets, below 1536 total allocations, and checks 600 frames. Type 2 uses ANM
main script 2, fast-spawn script 21 and full collision size (4, 4); player movement/hurtbox
come from `ply00a.sht`. They do not include parent/familiar/world lifecycles.

Earlier Wriggle practice/camera/effect work is retained component code and evidence.
It is not the active solver roadmap. Detailed historical structural predictions and
entry investigations remain recoverable in Git history; they must be revalidated if
that integration is revisited. Do not use a literal callback operand as the effective
callback without accounting for wrapper inheritance and reset instructions.

Native lethal-region observations preserve the exact endpoints submitted to
`Player::CheckLethalCollision`, including misses and invulnerable contacts. They
are last-update evidence, invalidated on the next native step, not stationary
future hazards. Future body bounds remain separately certified; an empty current
actor set must still pass global timeline coverage before returning empty bounds.
Unknown body creation/motion is not converted to an empty or frozen future.
