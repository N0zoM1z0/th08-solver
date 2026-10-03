# Scenario and checkpoint contracts

A useful benchmark runs from an explicit reproducible checkpoint to a declared terminal
condition without launching the game. Include resource/profile identity, numerical
model, difficulty, player state, RNG policy, duration/end condition and legal actions.
Distinguish synthetic scenes, actual-DAT controlled projections and source-faithful worlds.

## Shared rules

- Preserve live hazards, RNG and relevant actor state across transitions unless the
  modeled transition explicitly clears them. Replanning is not a world reset
- Compare strategies from the same checkpoint and seed. Record survival/death/search
  failure, budget, event/RNG/route digests and generation/search/replay cost
- Save successful and failed action tapes. Regenerate their geometry for unindexed
  replay, requiring death on the same final action or the same terminal boundary
- Unknown gameplay effects stop a case. Candidate-dependent aiming/damage/RNG cannot
  be replaced by a cached fixed future. Synthetic hook policy is not retail equivalence
- Default public CI uses no proprietary DAT; real-data checks are explicit local commands

## Native headless complete scenes

`th08_headless` links the tracked reconstructed game at revision recorded in
[Validation](VALIDATION.md). It runs production ECL, timeline, player, enemy, bullet,
laser, item, effect, spell, background and GUI/message updates. This is a native
source-execution profile, distinct from the controlled subset below.

- Start with default configuration and empty score/replay files in a disposable working
  directory. Select Reimu/Yukari, difficulty 0..4, a named stage and a 16-bit seed.
  Stage mode uses original stage-practice initialization; spell mode uses original
  spell-practice initialization. The complete wrapper/prelude is actually executed
- Load DAT/ANM/SHT/STD/ECL synchronously and bypass presentation loading countdowns.
  No menu/window startup, worker loading waits or draw-chain execution is required.
  CPU ANM resource metadata and calc-chain lifetimes remain; text rasterization is omitted
- Use native float32 with contraction disabled, no fast-math, and deterministic
  `timeGetTime = floor(update_count * 1000 / 60)` milliseconds. One input executes one
  original calc-chain update; timers retain original multiplier/update order. Original
  shared RNG consumers in that chain remain active, including visual effects
- Carry the original live managers through boss/dialogue/spell transitions. Stage mode
  stops at native stage-clear state before result/next-stage processing; continuous
  multi-stage execution is not implemented. Spell mode stops when the requested spell,
  after becoming active, becomes inactive. No terminal cleanup/reset is invented
- Raw `--spell-id` is zero-based 0..221. Stage/difficulty wrappers can select another
  ID; first activation is checked and a mismatch stops the case. An insufficient
  budget before activation remains a failure, not proof that the requested ID ran
- Stop at the first native player state 2 as `collision`, before the later death-counter
  increment. `complete`, `collision`, `frame_limit`, `tape_end` and `retry_menu` are
  distinct outcomes. Exit status is 0 for complete, 2 for ordinary failure and 1 for
  invalid/unsupported execution. Unknown ECL/timeline opcodes stop rather than skip;
  header-only timed ECL opcode 0 is explicitly recognized as upstream padding

The baseline `reactive` policy proposes one of nine focused moves using a 12-frame
constant-velocity hazard estimate and a center preference. It is not a physics oracle
or optimal search. The actual game performs collision, new emission, transforms,
damage, graze/score/gauge/item feedback and cancellation. Stage mode also shoots and
alternates confirm to advance actual message scripts; spell mode defaults to movement
only. `--shoot 0|1` overrides shooting. Focused `stationary` is retained as a bad baseline.

`hazard-reactive` applies the previously latched movement, then ranks the same nine
constant-direction proposals over 12 bullet updates and 120 pooled-laser updates. It
projects an already-active vector-acceleration opcode with the native velocity-before-
position order. A spell profile may also opt into a source-owned count of linear updates
through an active WAIT; outside that proven count the transform remains unsupported.
Other active bullet transforms remain explicitly counted soft evidence; future transform-
program activation, new emission and ECL-owned transient hitboxes are not inferred by the
generic policy. Existing lasers use copied raw lifecycle fields and source collision
coordinates. A conservative perpendicular-axis broad phase can discard a candidate and
laser only when their predicted path rectangles cannot overlap at the observed angle.

`spell-portfolio` selects hazard options in `spell_policy.hpp`. The default is the
source-vector profile. ID199 uses the measured linear-ranking ablation: it completed
seeds 0, 1 and 65535, while vector ranking failed seed 0; IDs 193/195 show the opposite
need for vector projection. ID85 measures each pooled laser's translation and shortest
angle delta across the preceding native update. Its isolated profile fits the rotation
center implied by those two source-owned states and extrapolates one rigid transform per
forecast update; moving lasers bypass the constant-angle broad phase. This proposal is
refreshed every update and does not claim that other pooled lasers continue rigid motion.
ID93 reads only a currently due native ECL opcode 114. Its typed observation copies the
52-byte spawn payload, resolves a local-float angle without calling the gameplay operand
resolver, and reconstructs the same-update pooled-laser lifecycle from the source-owned
enemy position and shoot offset. Aimed opcode 115, child contexts and variable geometry
stop the adapted case; suppressed spawns produce no warning. The warning ranks the nine
directions before the pooled laser exists, accounting for the already-latched first move.
ID198 combines that fixed-origin rotation observation with 81 two-leg proposals: each
initial direction lasts four candidate-controlled updates, then one of nine continuation
directions repeats. Bullet projections and laser lifecycle forecasts are immutable and
shared across those paths within one decision; mutable player paths remain separate and
all proposal/check costs are reported. Only the first action executes before fresh native
state is observed and the portfolio replans. ID201 uses a 32-update bullet horizon. For a
live bullet whose only active transform is WAIT, the session follows the copied transform
records without mutating them and exports exactly how many later updates retain the current
velocity. Disabled records and source `allowWhileActive` order are preserved; a terminal
child-pattern record contributes the parent's final movement/collision update, while any
other enabled transform ends the proof.

ID201 also has a narrow future-ECL adapter. From the current main cursor it accepts only
the same-time opcode sequence needed by this spell: known local-float add/subtract,
literal/local-float transform record writes, and deterministic non-aimed fan/circle shots.
It copies the descriptor and local variables, forecasts supported enemy interpolation,
applies sprite geometry, suppression, offscreen culling and pool limits, and emits each
new bullet at its first native collision update. It never calls the gameplay operand
resolver or RNG. Unknown control flow, selectors, aimed/random shots, motion, transforms
or capacity stop this enabled case. Spawn warnings are sorted by update so every constant
candidate path advances once and is reused across all bullets at that boundary. The
seed-0 wrapper completes; seeds 1 and 65535 remain failures, and seed 1 confirms that
RNG-dependent child patterns are outside this adapter.

ID202 opts into the same bounded projection of already-observed WAIT bullets, with its
original 12-update horizon. The random child's angle and speed have already been chosen
by the native runtime before observation; forecasting its proven linear interval does
not sample a future RNG state. ID202 does not enable the deterministic ECL adapter:
other visible opcode-99 shots use random-angle selector 10082. The baseline collision
at 3265 is avoided, but seed 0/1/65535 wrappers still collide at 4204/4216/3613. This is
a local modeling improvement, not another completed checkpoint.

ID32 uses the existing two-leg scorer with a 13-update observed-bullet horizon and
one-update first leg. At the baseline bottom-left corner, the 12-update view waits
at decision 582 and collides at 594; rightward intervention at 582 survives, while
all nine replacements at 583 fail. Horizon 13 alone avoids that hit but fails at
1394. At decision 1382, only down-right followed by the original rightward suffix
survives, motivating the short first leg rather than another horizon increase.
The isolated profile ranks 81 paths instead of nine, with 12 instead of 11 future
bullet steps. It changes no source projection, RNG, tie-break or terminal rules.
All three seeds complete the 2372-update wrapper; continuous seed-0 Stage 3 clears
at 39767 with carried native state and fresh O0/O3 replay.

ID167 keeps the 12-update horizon and enables the existing 81 two-leg paths with a
four-update first leg. Its baseline enters a bottom-left constant-path trap and
collides at 932; all constant proposals predict collision from observation 917.
The successful profile changes earlier decisions (first divergence 567), rather
than claiming a late escape from that prefix. It reuses unchanged source projections
and costs 444852 candidate evaluations for each 5492-update complete seed wrapper.
Seeds 0/1/65535 freshly replay at O0/O3. Its preceding-checkpoint Stage6b run passed ID167 but collided in ID183 at 54401;
the later ID183 profile supplies the current whole-stage clear.

ID183's original constant-path profile collides at 1041. All nine interventions at
1030 fail; upward input at 1029 survives through 1041 with the original suffix. Horizon 13 alone instead fails at 820, and
H12/two-leg4 fails at 819. In the latter run the fatal 24-pixel bullet is already
visible 31 updates earlier, beyond the short horizon. Its isolated profile therefore
uses the existing 32-update/two-leg4 search scope, with unchanged linear source
projection: 81 candidates and 31 future bullet steps, 104652 candidates per wrapper.
It clears all three 1292-update seed wrappers and continuous Stage6b at 58853, with
fresh O3 and O0 replay. It does not enable ID139's boundary-bounce source adapter.

ID139 uses source-bounded active boundary-bounce projection. The native boundary test
runs before movement and uses loaded sprite dimensions, not the smaller collision box.
It mirrors strict outside tests, X then Y reflection, bounded native angle normalization,
velocity replacement and bounce counting, including the final active update. Bottom-
excluding bounce still consumes a bounce and installs speed when outside the bottom.
The view rejects concurrent transforms, active sprite animation, non-unit update rates
and scripted freeze. Future cancellation, freeze or despawn remains native-owned.
Its isolated profile ranks 81 two-leg paths (first leg four updates) over 32 bullet
updates. Bounce alone at horizon 12 fails at 5327; two-leg horizon 12 fails at 4355.
The latter fatal bullet was already observed 31 updates before impact, motivating the
32-update bound. This explicitly costs nine times as many candidates and 31 rather
than 11 future bullet steps. Seeds 0/1/65535 complete 7292 updates; seed-0 continuous
Stage 6a completes 61041 without resetting the world between spells.

ID204 keeps the same 12-update horizon and enables the source-bounded active
relative-direction recurrence. Its observation copies the current angle/base speed,
turn angle/speed, interval and timer. The validity bound includes the final turn's
movement/collision but stops before the next transform-program update; an enabled
concurrent record invalidates the bound. The scorer mirrors native deceleration,
timer comparison/reset/increment, angle addition and velocity-before-position order.
Fractional timers, a non-unit frame multiplier and scripted freeze invalidate the view.
It does not forecast aimed turns or execute the next transform record. Seeds 0 and
65535 complete the 4832-update wrapper; seed 1 retains a collision at 3008.

ID89 observes only the current native ECL cursor. A constant
opcode 136/137 selector for direct-laser EX callbacks 9/11/25 becomes a warning only
when difficulty and timer state match and the currently observed parent, interpolation,
movement and rotation state is static. The adapter refreshes that proposal every update;
it does not infer across uninspected ECL control flow. Repeating callbacks constrain the
nine-direction candidate set over their active interval; the generic hazard scorer ranks
bullets and pooled lasers within that set. Flagged selectors and dynamic geometry are
counted and left unsupported. This is an explicit proposal choice, not altered native
physics or a claim that these forecasts model every transform.

Action tapes contain decimal original 16-bit input masks, one per update (shoot 1,
bomb 2, focus 4, directions 16/32/64/128, confirm 4096). Replay uses a fresh process,
without planner decisions, and rejects extra actions beyond the execution boundary.
Only explicit `--allow-unused-actions 1` permits an unused replay suffix, recording
`unused_actions` while preserving the actual terminal/failure. This is for bounded
interventions, not successful strict replay. `--prefix-frame N` records the projection
digest after N updates; an unreachable prefix is an error.
The FNV projection covers actions, player, bullets, actor/script timers, RNG state/count
and feedback every frame. It is not a complete world serialization or independent
physics oracle. `rng_draws` reports the native generation counter at the terminal
update; original code can reset this counter, so it is not total run consumption.

The current noncopyable session owns original process-global managers. It cannot
provide independent candidate snapshots yet. Alternative futures must not share one
mutating native world; exact branch ownership is required before adding beam/tree search.
`th08_headless_probe` already provides candidate ownership through serial fresh
processes: replace only the direction bits at one 1-based input update, retain focus,
shoot/confirm and the common prefix, optionally observe a fixed unchanged suffix.
The suffix does not rerun the policy after intervention. Each child retains its own
original RNG, actors, bullets, feedback and lifecycle updates. This diagnoses local
choices; one safe observed update is not a complete future route.

`--trace PATH` writes the last 32 updates as TSV, with before/after player and occupied
bullet slots, full collision dimensions in pixels, velocity in pixels/update, active
transform flags, the source-bounded WAIT linear-update count, proposed action, sampled
input and latched movement input. It also
records every native `CalcLaserHitbox` center, size, origin, angle and graze flag, plus
raw pooled-laser lifecycle fields, preceding-update motion deltas and active ECL
cursor/instruction fields used to diagnose warnings. Live hazard-policy traces also
record all nine candidate actions, enablement, first predicted overlap, minimum
clearance, accumulated danger, center distance, best continuation and the selected
candidate. Bullet
slot reuse across updates is possible; these are pool indices, not stable entity IDs.
Collision JSON records the first lethal overlap before death feedback; laser bounds
use the original rotated test coordinates and retain the raw call geometry.
`laser_slot=-1` identifies an ECL-owned direct hitbox. Other bounds use world
coordinates. None of this changes acceptance physics or calls the RNG. The original
FNV projection is unchanged and remains a partial diagnostic projection, not a complete
state key.

### ID192 body geometry and ID203 transform boundary

ID192's baseline 466 is the boss body, with native dimensions 48×32 at (192,128).
Input 466 changes fail 9/9; input 465 has 6/9 local survivors. The adapter exports direct
inclusive endpoints using native reciprocal multiplication by 1/1.5, followed by /2.
It handles observed static/resolved interpolation and one literal future random 67
move using an outward-rounded all-angle envelope, without evaluating random operands.
The canonical pre-clamp envelope contains the in-bounds origin and remains conservative
under native clamping. Intersecting it with clamp bounds is also conservative but changes
ranking and fails 687; retaining the source excursion is an explicit proposal convention,
not extra numerical padding. No fixed epsilon or sampled random angle is used.

Support requires unit integral clocks, finite bounded motion and guarded owner state.
Narrow owner-local ECL/subgraph checks reject remote mutations, unknown selectors,
returns/callback state and unsupported operations. Timelines must remain future, terminal
or blocked on the supported boss/event. Boss callbacks' 70-update immunity and recognized
phase death cover conditional lifecycle branches. Continuous Extra's mode 1 death requires
the validated immediate 134/160/123 EndSpell prefix before collision can resume.
Unknown support returns a typed failure and stops the enabled profile. Warnings are
conditional lethal-coverage bounds, not exact geometry through arbitrary callbacks or
complete unseen-spawn prediction. Native execution remains the collision oracle.

The profile uses 81 actual two-leg paths, H12, first leg 1, with bounded WAIT projection.
Bodies affect earliest overlap only; bullet clearance/danger/ties stay unchanged on
body-safe paths. Common pending-input overlap and absence of body-safe continuations
have separate counters. Seeds 0/65535 clear 3692, seed 1 fails1470. Actual Extra reaches
ID202 at 67738 without any world reset; it does not yet clear.

A WAIT followed by DESPAWN still has a final fired movement/collision before the next
nonlethal despawn update. A distinct typed WAIT→VECTOR→NONE view models the copied
resolved acceleration, activation delay and final-clear movement, rejecting concurrent,
fractional, frozen, nonfinite or unsupported programs. ID203's preceding H12/nine-candidate profile retained failures 1711/4823/2896. The
certificate itself does not solve the spell or predict action-dependent future RNG.

### Bounded native-prefix repair

ID203 now uses an observed-bullet H32 beam, retaining seven prefixes per first-action
family, 63 total. Every prefix owns only player coordinates and score; bullet projections
are immutable. It consumes the pending native input before candidate movement. Stable
sorting preserves the existing comparator, numeric action/last-action ties and generation
order. The reported continuation action is the last beam action, not a two-leg command.
Observed pooled lasers and supplied body warnings are explicitly unsupported. The
ID203 profile does not enable the ID192-specific body adapter; these are partial
observed-bullet proposals, not complete world-safety certificates. Unsupported bullet
transforms remain counted soft projections, never certified empty future space.
The rolling beam still collides at 787 for seeds 0/1/65535.

`th08_headless_repair` first executes that policy from the requested original checkpoint.
For a collision with an observed terminal safe-to-unsafe transition, it enumerates
rollback counts 1..8 times 16 updates and nine row-major direction holds of 16 updates.
The first replaced action is one-based; earlier tape actions and the hold's non-direction
bits are preserved. Each child process replays its exact prefix, executes its intervention,
then resumes the unchanged policy. Source-prefix projection digests and all forced actions
are checked, so action-dependent native aiming, feedback and RNG belong to that child.
No native memory snapshot or shared future world is used.

After each full round, only the longest genuine collision survivor advances, with first
candidate winning equal-frame ties. Search stops on the first full completion, no progress,
unsupported trigger or explicit budget. The fixed tested family is two rounds, 144 candidates,
2160000 native candidate updates and 600 seconds; the CLI exposes frame/update/wall budgets
for longer scenes. Initial execution and final verification replay are counted separately.
The seed 0 ID203 run automatically finds a complete 5492-update tape after 91 candidates;
none of its intervention frames are encoded in the algorithm. This does not establish
other seeds or a continuous Extra clear, which still stops earlier in ID202.

Strict `--replay` stops at its tape boundary. Explicit `--resume-prefix` continues
with the selected policy; a 50-update control prefix reproduces the original 787 tape.
A complete policy log owns per-update digest/horizon/overlap records. The outer runner
pins and hashes child executable bytes, validates the entire child report and scene,
requires new output directories and preserves all attempted artifacts. Timeout sends
TERM, then KILL after bounded grace and reaps the child; protocol/interruption errors
retain per-phase costs and an upper bound for unverified updates. Fresh replay equality
is a projection/feedback check, not full-state identity or retail equivalence.

## Synthetic continuous profiles

`th08_scenario_cases` supports `relay` (curtain/rings/moving lane), `lane-switch`
(reordered phases) and `late-gate` (a delayed wide obstacle). They are algorithm stress
scenes, not original Touhou stages. Definitions own phases, movement/hurtbox and visual
policy; checkpoints own world frame, bullets, player and both RNG states.

Gameplay starts at the selected 16-bit seed. Visual RNG starts at `seed xor 0xa5a5` and
consumes the configured 0..32 U16 draws per frame, independent of gameplay. Changing
visual draws was verified to preserve world/events/gameplay RNG and the complete route.
This is intentionally controlled semantics, not simulated original visual draw ordering.

Each snapshot is lethal geometry after the world update, tested at the player's
post-action position. Death frames are one-based; phase endpoints are exclusive update
indices. A boundary at 2400 therefore first collides at 2401. Bullets and RNG are carried
through that boundary. Forecasting copies the checkpoint and cannot mutate it.

Strategies are stationary, next-step greedy and rolling beam. Default rolling horizon
is 120, committed prefix 30, beam 128 and 200000 attempted expansions per decision. Geometry
memory is O(horizon × live bullets); the executed tape is O(duration), one byte/frame.
JSON saves actions as `(y+1)*3+x+1`, where 4 means stay. TSV saves summaries only; use JSON when the persisted action tape is needed.
The TSV-producing run still regenerates and verifies its route before writing.

### Reproduced failure and optional recovery

A center-seeking beam can discard early escape alternatives and later exhaust even
though a complete route exists. The baseline late-gate records this failure; an explicit
left-goal route is an escape witness, not a general fix.

`--recover-goal 1` adds at most one retry after `search_exhausted`. It samples an 8-pixel
grid for the nearest movement-bounded point free throughout the forecast, then plans
toward it. It does not read a profile name or hardcoded escape coordinate. Normal route
verification still establishes safety; the reach bound is only a proposal filter.
The retry shares the original budget, with its beam capped to fit the remainder.
Target probes and failed/retry search costs are reported separately. Default is off.

This repairs the checked late-gate case while retaining identical relay/lane-switch
routes. It can still fail when no stationary grid refuge exists or budget is insufficient.
Failure executes no invented action and is not a proof that the scene is impossible.

## Real-DAT controlled ID179 Easy

Member `ecldata7sp.ecl`, SHA256
`7f1a847fdd7ceb5e35dfd3529a54961ab4d1c9e7607fbcfb577936465326ab0e`.
Spell179 is 「永夜返し  -丑の刻-」, owner 蓬莱山輝夜. Exact occurrence:
main sub72 PC10, offset 56084, instruction mask 0xf1, execution bit 1.
DAT identity and reconstructed source pin are in [Validation](VALIDATION.md).

### Supplied checkpoint

- Begin after sub72 PC0..9, immediately before START_SPELL; empty call/child state
- Boss at (192, 224), move-to target already the same position; interaction bits 3 disabled;
  no other actors, shoot interval/offset zero, sounds-1, empty transforms
- Boss timer 0/limit 1200, death/timer callback 1, initialized source-template scalars,
  context extraInt3=0; empty 1536-slot bullet pool with cursor 0
- Player alive at (192, 400), focused `ply00a.sht` speed/hurtbox, movement-only actions;
  no shots, bombs or form changes; explicit gameplay and visual seeds

Callback 1 is part of this supplied profile. Actual practice wrapper 83 can retain
callback 2, so this is not wrapper-equivalent initialization. The prefix is supplied
state, not a claim that menu or preceding stage initialization executed.

### Executed behavior and ending

Actual scalar ECL calls/waits execute through charge sub33 and install child73 at update 162.
The child runs that same update, emitting two eight-bullet rings every 15 updates through 1197:
140 requests and 1120 successful allocations. Births remain below 1536, so the source circular
cursor never wraps or contends even as earlier slots retire.

Type 2 colors 2/6 use certified ANM main script 2 and fast-spawn script 21. Creation installs
polar acceleration without advancing its clock. Ten spawning updates move at half initial
velocity; the last also enters fired transform/motion/collision. Fired phases preserve
transform, displacement and sprite-bound cull order. Polar alone has no turn/bounce
128-frame outside grace. Update 171 is the first lethal phase (zero-based world index).

Exactly 1200 lethal phases cover boss timers 0..1199. At timer 1200, main/child ECL execute
before timeout; the child is removed and the player becomes invulnerable. Callback 1 then
executes END_SPELL and SET_BOSS(-1). EndSpell retains remaining occupied bullets as
nonlethal despawning slots; it does not free them. The segment stops before engine/menu
field 10051, reward/item processing and post-spell despawn-animation updates.

A next-stage adapter must implement that transition state. Treating these slots as an
empty pool or concatenating this case repeatedly does not establish an original stage.

### RNG, independence and verification scope

Visual requests use one separate-stream U32 per requested particle. No camera or visual
consumer reconstruction is needed. Gameplay consumes four U16 draws for the two initial
random angles and two more for the terminal departure angle. Visual-seed changes leave
hazards/gameplay unchanged; gameplay-seed changes affect actual DAT angles.

Circle shots do not aim at the player; active spell suppresses rank adjustment; minimum
distance is zero before shooting; no damage/cancellation/form actions are allowed.
Graze/score/item feedback is explicitly omitted. Original graze can affect gauge, subrank,
effects and items, so the complete original world is not candidate-independent.

Compare stationary/greedy with full 1200-frame and rolling 180/commit 30 planning. Seeds 0, 1,
65535 passed both planners to the actual controlled end callback, with fresh unindexed
replay. All collision/search-failure prefixes also replay. Exact results, budgets,
hashes and routes are in [reports](../reports/native/README.md).

The adapter's immutable Program must outlive World copies. Its digest covers the owned
projection and hazard trace, not every field of a full game. This is a complete controlled
survival segment, not retail capture, x87 equivalence or full-stage completion.

### Bounded H12/H32 repair extension

For each selected collision, use the actual final policy horizon (only 12 or 32).
A horizon change or unknown row breaks the safe/unsafe history. Only a witnessed
safe-to-unsafe transition within that homogeneous terminal run supplies an onset.
Hold duration is H/2; enumerate eight rollback segments and nine direction holds
in existing order. Each candidate replays its exact prefix in a fresh native
process, then resumes the unchanged portfolio. A later selected round derives its
own horizon; candidates do not change the source horizon mid-round. Native state,
RNG, input-latch semantics and collision authority remain unchanged.

The isolated ID202 seed-0 recipe retains H12 and completes 4712 updates after 24
native candidates. This does not solve its other seeds or permit deterministic
preview to omit random shots. ID203 retains its H32/63-prefix proposal profile.
