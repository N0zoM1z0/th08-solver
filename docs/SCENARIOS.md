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
