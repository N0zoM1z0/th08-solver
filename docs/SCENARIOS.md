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

Action tapes contain decimal original 16-bit input masks, one per update (shoot 1,
bomb 2, focus 4, directions 16/32/64/128, confirm 4096). Replay uses a fresh process,
without planner decisions, and rejects extra actions beyond the execution boundary.
The FNV projection covers actions, player, bullets, actor/script timers, RNG state/count
and feedback every frame. It is not a complete world serialization or independent
physics oracle. `rng_draws` reports the native generation counter at the terminal
update; original code can reset this counter, so it is not total run consumption.

The current noncopyable session owns original process-global managers. It cannot
provide independent candidate snapshots yet. Alternative futures must not share one
mutating native world; exact branch ownership is required before adding beam/tree search.

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
