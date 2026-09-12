# Wriggle complete offline world contract

This is a source-derived target contract, not the current capability ledger.
Use [Status](STATUS.md) for implemented components and [Coverage](COVERAGE.md)
for the dependency-ordered, still-unfinished integration milestones.

## Target and evidence boundary

The next integration target is the **complete first stage-one boss spell family,
IDs 2, 3, 4 and 5**, including its familiars, player-dependent behavior, shared
random stream and termination. Start with its actual spell-practice entry, then
verify its ordinary-stage entry separately. This is an implementation order, not
a replacement for the all-spell objective. No complete world or solution is
claimed by this document.

The inspected artifact is the user's DAT and reconstruction revision
`a45e99fb1942714e6edded20847e32a654d56f97`. Offsets below are decimal, measured
from the beginning of the decoded ECL member. PCs are zero-based instruction
indices within one subprogram. Source line references refer to that revision,
not a moving upstream branch. The numerical target is the explicitly selected
native float32 profile; retail x87 equivalence remains unverified.

| Artifact | SHA-256 |
|---|---|
| `th08.dat` | `9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb` |
| `ecldata1.ecl`, 45844 bytes | `6b44a0ea36648edcdeae522a2ac16d1f09bf2097d3ddaa1a61c8c1703bad68ea` |
| `ecldata1sp.ecl`, 39808 bytes | `aac506b4eaf8fdfaa90e876f74db711d3c0724f63798e7d62801b02bbb29e00e` |
| `src/EclRun.cpp` | `010049211263e47d8245c7335f56b17a8502ca0f84595c8b035926a495d90b57` |
| `src/EclRunLow.inl` | `8c6d23bf4e9daf8f96dbd344f4a03d3ed32d1d200483959682e962cd41ec0045` |
| `src/EclRunHigh.inl` | `5e8c0b8ac1bd35f92cf2c3eb8792b2fb62d00feb97a21dc27e935101c5a45914` |
| `src/EclDependencies.cpp` | `019f9cd6abdb73223d3d41cc8a6317641e6fe6bfbd7777d126a4bace3e14e2e4` |
| `src/EclExIns.cpp` | `b9e0d86a89ecc2042368b0a264a51c95ee607f12c864eafd4209879813bdc1e5` |
| `src/EclGlobals.cpp` | `d77dd825c22d7e217c13d97262d4d5b3359b09153adb7b63d596440d95680380` |
| `src/EnemyManager.cpp` | `e8febe94a833472b33f732e83ee39ee48fdc5097c5d69ff094fd1f1bb8629a7d` |
| `src/EnemyManagerUpdate.cpp` | `5692ab3214e95873626e6ab896f867746217b0556b34737c2556e1b38a454e59` |
| `src/EnemyTimeline.cpp` | `920ee34725aa6aad9f113d43454731acadab456abddac73256b2ba9a29e8e94b` |
| `src/EffectManager.cpp` | `63d45a213956008b44874bc4707c971a7799a9c551b07e732bf1f55282c2209e` |
| `src/Spellcard.cpp` | `d8ed23efa216888b17a060732d37ddbe9617d4d47e3761c56a5414928609f51b` |

The native structural reports retain the exact member, subprogram, instruction,
difficulty-mask and spell-site identities. A local C++ inspection probe decoded
the DAT through `resources::Archive` / `parse_ecl`; it did not execute ECL, skip
unknown effects, launch the game, modify game data, or run Python.

## Entry identities: do not merge these worlds

| ID | Difficulty bit | Stage spell instruction | Practice spell instruction |
|---|---|---|---|
| 2 | Easy, `0x01` | sub38 PC10, offset23140, mask`0xf1` | sub24 PC10, offset18116, mask`0xf1` |
| 3 | Normal, `0x02` | sub38 PC11, offset23384, mask`0xf2` | sub24 PC11, offset18360, mask`0xf2` |
| 4 | Hard, `0x04` | sub38 PC12, offset23628, mask`0xf4` | sub24 PC12, offset18604, mask`0xf4` |
| 5 | Lunatic, `0x08` | sub38 PC13, offset23872, mask`0xf8` | sub24 PC13, offset18848, mask`0xf8` |

The ECL difficulty test is containment, not any-bit intersection:
`(instruction_mask & (difficulty_bit | enemy_override)) ==
(difficulty_bit | enemy_override)` (`EclRun.cpp:64`). Linked familiars refresh
their override to human `0x20` or youkai `0x40` before ECL execution
(`EnemyManager.cpp:883`, `EnemyManagerUpdate.cpp:152`). The main boss does not
automatically inherit that override. `Player::IsYoukai` reads `isYoukai`, not a
substitute interpretation of the focus button (`PlayerBomb.cpp:31`). Character,
focus transitions and actual form state belong in the player model.

The other ordinary-stage spell families remain distinct: IDs0/1 start in sub22;
IDs6..9 in sub44; IDs10..12 in sub48. The practice file also contains ID205 in
sub36. Covering IDs2..5 does not cover those identities.

### Ordinary stage entry

Timeline0 spawns sub25 at time4175, offset44164, position `(192,-16)`, life60000,
drop type-2, score100000. Sub25 starts at offset14980. Its ordered setup is
`59,80,80,127,77,160,148,134,63,64,126`: alternate main animation sequence0,
interaction changes, boss slot0, hitbox48x32, damage-reduction60, marker count0,
an interim timeout `(180000,38)`, position `(-32,-32)`, movement over60 frames to
`(192,128)`, and dialogue subroutine slot1 set to sub26. Its time100 jump waits
for external dialogue dispatch; the timeline alone is not proof that sub26 ran.

Sub26 at offset15216 sets life13000, life callback0 `(threshold1500, sub38)`,
timeout `(1800,sub38)`, death callbacksub33, and marker count1. The threshold
comparison is strict `<`, then life is restored to the threshold
(`EnemyManager.cpp:452`). This produces a life1500 phase entry, but position,
RNG, attached entities, timers, player and visual state depend on the preceding
nonspell. Its opcode67 movement reads the player and shared RNG. An arbitrary
sub38 launch with zeroed inherited fields is not an ordinary-stage entry.

Life/timeout callbacks release child ECL contexts, reset selected bullet/rank
state and detach/kill nonboss enemies. These are explicit transitions, not a
normal call (`EnemyManager.cpp:436`, `:592`). Stage sub38 ends through sub33,
whose opcode123 at offset19996 ends this spell before starting the next nonspell.

### Spell-practice entry: initial implementation target

`Background.cpp:127` and `EnemyManager.cpp:1393` select `ecldata1sp.ecl`.
Timeline0 contains these four nonterminal events:

| Time / offset | Instruction and required effect |
|---|---|
| 1 / 39684 | opcode0 spawns sub0, position`(30,-16)`, life20, drop-2, score1000 |
| 30 / 39716 | opcode0 spawns sub42 with the same spawn packet |
| 30 / 39748 | opcode10 waits while boss slot0 points to an active enemy |
| 60 / 39760 | opcode16 sets `showRetryMenu`; it is not an unconditional time60 win |

The wait holds the timeline clock, so its time60 event occurs after boss removal
and the remaining timeline delay (`EnemyTimeline.cpp:136`, `:244`). Unlike ECL,
timeline difficulty filtering uses any-bit intersection with the difficulty bit.

Sub42 at offset38640 is the real practice wrapper, not sub24 alone. Its ID2..5
path, in exact instruction order, is:

1. PC0..14, local time0: `136(19,0),59(0),80(8),148(0),127(0),131(1700),`
   `158(0,0,life,color),129(3),130(41),176(1),75(32,48,352,128),`
   `134(180000,41),133(0,0,41),63(192,128),124(5)`.
2. PC15..18 at times1,5,9,13: effect40 with four different colors; PC19..26 at
   time43: `124(15),173(1),81(8),81(3),59(0),127(0),77(48,32),148(0)`.
3. PC27 at offset39152 compares the EX-published ID with1; IDs2..5 jump to
   PC32 at39260. There `48(id,5,43,+108)` does **not** jump. PC33 at39288 sets
   life1500; PC34 at39304 updates the life gauge; PC35 at39332 calls sub24.

EX19 is `PublishCurrentSpellCardNumber`: it writes active context int variable0
from `currentSpellCardNumber` and consumes no RNG (`EclGlobals.cpp:65`, table
index19; `EclExIns.cpp:787`). Do not infer the selected ID from difficulty alone.

Opcode176 sets `playerDeathDissolve` mode1 (`EclRunHigh.inl:985`). Consequently,
the practice guards in opcodes129/130/133/134 preserve the wrapper's relevant
presentation/callback state. In particular, sub24's literal callback19 is **not**
the effective ending callback: its opcode134 updates the timeout threshold to
1920 but preserves callback41; opcode153 then copies death callback41 into the
timer callback. Its later opcode130 also preserves41. Stage sub38 instead uses
sub33. A handler that merely assigns the visible operand is incorrect here.

The spawn template is source-initialized, not unspecified zero defaults:
`EnemyManager.cpp:131` zeroes storage then establishes active/collision/damage
flags, default hitbox24x24x24, zero motion, disabled callbacks, rank speed range
`[-0.15,0.15]`, shot sounds7/25 and minimum-player-distance-squared1024.
`SpawnEnemy1` copies this template and immediately executes the first ECL frame
before setting final spawn metadata (`EnemyTimeline.cpp:16`). `CallEclSub`
resets PC, main/secondary clocks and sub ID only; it does not clear all registers
(`EclManager.cpp:91`). Preserve both rules in snapshots and callback entry.

The practice prelude is not random-free. Sub0 at offset244 repeatedly emits
effect51: groups of16 at local times0,4,8,12,16,20, then groups of4 every4 frames.
It is killed by sub24 opcode95, but effects already allocated remain relevant.
Effect51 uses script73 and `InitializeTintedBossTrackingCameraParticle`
(`EffectManager.cpp:110`, `:571`), whose initializer performs eight floating RNG
calls per successful allocation. Its update lifetime depends on the stage camera
(`:602`), so effect-pool occupancy can change later RNG consumption. Do not
replace this prelude with a guessed draw count or omit it as decoration.

## Ordered spell-program closure

Stage subs38..43 correspond to practice subs24..29. A native byte comparison
checked all175 records, including six sentinels: opcode, time, flags, masks,
payload size and every payload word agree after subtracting14 from the explicit
sub operands of opcodes52/92/130/134/135. This was a discriminating test of the
shared-code hypothesis, not proof of shared entry state or runtime equivalence.

| Responsibility | Stage sub / start | Practice sub / start |
|---|---|---|
| Full spell main | 38 / 22960 | 24 / 17936 |
| Familiar parameter adapter | 39 / 25352 | 25 / 20328 |
| First rotating bullet sequence | 40 / 25500 | 26 / 20476 |
| Second rotating bullet sequence | 41 / 26444 | 27 / 21420 |
| Familiar shooting child context | 42 / 27388 | 28 / 22364 |
| Familiar main/orbit/lifetime | 43 / 27836 | 29 / 22812 |

### Main spell: stage sub38 / practice sub24

The table uses stage sub IDs. Practice IDs are14 lower. All times are **context
local times**; a blocking normal call does not make its caller's timer advance.

| PCs / time | Ordered instructions and dependencies |
|---|---|
| 0..9 / 0 | `105(0),95,113(-1,-1),80(4),110(0,0),134(1920,33),153,132(0),6(extraInt3,0),64(90,4,192,128)` |
| 10..13 / 0 | Four masked opcode122 spell starts; exactly one difficulty branch runs |
| 14..20 / 90 | `81(4),160(180),129(2),130(33),7(localFloat4,64),62,139(40,1,color)` |
| 21..44 / 150 | Four same-frame parameter packets and normal calls tosub39; shared int0=120, float0=`0,pi/2,pi,-pi/2`, float1=pi, float2=localFloat4, int1=`10,60,10,60` |
| 45..47 / 210 | Blocking `52(40)`, then add24 tolocalFloat4, then `67(60,4,2)` |
| 48..49 / 270 | `139(40,1,color),62` |
| 50..73 / 330 | Four packets/calls tosub39 as above, but float1=-pi |
| 74..76 / 390 | Blocking `52(41)`, add24 tolocalFloat4, `67(60,4,1.5)` |
| 77 / 450 | Jump to local time90, offset24200 / PC19, not to the spell-start instructions |

Each sub40/41 sequence lasts360 local frames; it is not a detached parallel
emitter. Thus time450 is not a450-frame whole-spell period. The boss timer is
separate, continues during those calls and interrupts the active call stack on
timeout. Exact absolute frame numbering must include the spawn-time immediate
`RunEcl` call and any same-frame re-entry; do not infer it solely from this table.

### Familiar adapter, main and child

Sub39's complete nonterminal opcode order is `6,7,28,28,92,53`. It retains call
int0 and float0, divides call floats1 and2 by call int0, then opcode92 creates
sub43 at local position`(0,0)`, life=callInt1, drop-2, score100. Unlike a bullet,
this is a separately allocated enemy with its own health, main context, child
context, ANM, alignment effects, attachment links and position inheritance.

Opcode92 first calls `SpawnEnemy2` using the parent's context-local register
block, including call parameters. The child immediately executes ECL before the
parent handler finishes linking it and installing parent-relative coordinates
(`EclDependencies.cpp:585`, `EnemyTimeline.cpp:64`, `EclRunLow.inl:879`). A newly
allocated higher enemy slot may then execute again later in the manager's same
frame scan. Child allocation failure and the480-slot limit must be explicit.

Sub43's complete nonterminal order is
`54,77,79,83,174,160,18,73,2,74,135,1`:

- Set common enemy ANM55; hitbox24x24; interaction flags16; form effects enabled;
  replace alignment effect with ID33; damage reduction10.
- Divide localFloat1 by100, then opcode73 orbits around the current position
  using duration=callInt0, angle=callFloat0, angular velocity=callFloat1 and
  radial velocity=callFloat2. Its ownership and numerical semantics must come
  from the motion handler, not a precomputed screen-space circle.
- Wait callInt0 (120) on the secondary clock. Then opcode74 keeps the supplied
  angular velocity with zero radial velocity, and opcode135 starts child slot0
  atsub42. Main opcode1 at local time960 terminates the enemy; that timestamp
  excludes the preceding secondary-clock wait. Destruction/parent transitions
  may end it earlier.

Sub42's full nonterminal sequence is `6x8,7x4,96,96,2,96,2,4,53`. Difficulty
sets wait=`100,80,50,20`, count2=`1,1,2,2`, speed=`1,1.5,2,2.5`. The first
opcode96 at offset27628 has mask`0x3f`, type3/color6, five speeds and spawn flags520.
The next at27672 has mask`0x5f`, type1/color5 and difficulty-dependent count/speed.
After one wait, the type3 human branch fires again at27732, then another wait
and a jump to27628. The return after the unconditional loop is not its normal
termination mechanism. Re-evaluate masks after every form transition.

Opcode135 allocates/zeroes the child block, sets its PC/time, then copies context
variables through the field immediately before secondaryTime
(`EclRunHigh.inl:646`). Run main first, then child slots0..3 in order; the child
installed this frame can run this frame. Each context executes callbacks and
interpolations, then ticks its own clock before selection of the next context.
Enemy movement/shot-and-ANM selection occur only after all contexts
(`EclRun.cpp:114`, `:178`, `:183`, `:206`).

### Rotating bullet callees are not complete spells

Sub40/41 each have this nonterminal order:
`6,7,7,7,111,111,99x4,97x4,111,97x6,15,16/15,16,17,5,53`.
The difficulty masks select one opcode99, one first opcode97 and, except Easy,
two later opcode97 commands. Forty iterations fire every9 local frames. The
second transform write's allow-while-active field differs between the two
subprograms, as does the angle-update sequence. Reuse the verified implementation
and raw payloads, not a simplified common generator.

Their existing fixed fixtures establish840 births for their selected mask and a
600-frame collision model, not the actual full spell. They omit:

- The full main/wait/call/boss-timer timeline and repeated alternating sequences.
- Both familiar waves and their moving, aimed, alignment-dependent shooting.
- Moving boss origin, inherited position, player-distance suppression and pool
  contention. Actual dispatch rejects shots within the default32-pixel distance
  **before** resolving their operands (`EclDependencies.cpp:687`).
- Player shots, familiar and boss damage, parent damage propagation, capture
  validity, deaths, timeout transitions, cancellation and surviving animations.
- The continuously shared RNG including non-bullet effects and boundary-aware
  boss movement. Opcode67 chooses a random direction conditioned on player x
  (`EclDependencies.cpp:122`); even the boss future is candidate-dependent.

No explicit EX136/137 appears inside this six-subprogram spell closure. EX19 is
required by the practice wrapper. Absence of an EX instruction does not remove
native per-frame callbacks, ANM/effect callbacks or lifecycle dependencies.

## Whole-world frame and ending contract

The relevant calc-chain priorities are background8, player9, enemies11,
spellcard12, effects13, bullets14 (`Global.hpp:87`). Preserve the complete order
of participating consumers, not just their individual formulas. The enemy manager
runs timelines before scanning enemy slots0..479 (`EnemyManagerUpdate.cpp:110`).
For each active enemy it resolves pause/form gates, ECL, movement/inheritance,
visibility/lifetime, life callback, timer callback, ANM, collision/damage, death,
then effect tracking and timer tails. Life/timeout callback branches re-enter ECL
within that same update; ordinary death callback replacement does not use that
same immediate branch (`:268`, `:578`, `:666`).

Boss timeout1920 is a boss-clock threshold, not a guaranteed wall-clock frame:
the practice wrapper enables pauseTimer; player/bomb states can pause enemy
execution; scripted freeze affects timer advancement. The timeout is checked
after that frame's ECL, movement and offscreen handling, before damage. A model
cannot erase all hazards at the start of an arbitrarily numbered frame1920.

Ordinary death or timeout enters stage sub33. Practice enters sub41 at37456.
The practice ending path executes `184,129,130,80,7,66,173,6,40`, then the
capture-state-dependent effect sequence (`7,6,124`, repeated `15,37,140`, opcode5,
`124,140,6`) or jumps over it. Opcode123 at38504 invokes `EndSpell`; subsequent
instructions handle the ending wait, boss removal and eventual entity deletion.
Selector10082 draws a random angle even before the capture-state branch;
selector10099 resolves actual spell capture state, not a guessed boolean.

`Spellcard::StartSpell` establishes active/capture-valid state, owner, bonus and
time limit, clears bullets for transition and initializes spell effects
(`Spellcard.cpp:707`). `EndSpell` clears active, applies cancellation and enemy
cleanup on the non-timeout path, and sets captured only when capture remains
valid (`:1003`). `HandleTimerCallback` explicitly invalidates normal capture,
marks the timeout transition and removes bullets (`EnemyManager.cpp:592`).

Record separately: `CAPTURED`, `SURVIVED_TIMEOUT`, `PLAYER_DEAD`,
`UNSUPPORTED_WORLD_EFFECT`, `MISSING_ENTRY_STATE`, `SEARCH_LIMIT`. Full duration
survival is useful evidence of an executed world but is not a captured normal
spell. A solver result must carry the complete input route through the actual
ending transition and independent replay with the same event ordering.

## Minimal integration sequence and acceptance

1. **Source-initialized world and entry.** Own player/enemy/effect/bullet pools,
   clocks, shared RNG and immutable resource programs in one checkpointable
   world. Execute practice timeline0 and wrapper42 with EX19, real template
   initialization and practice flags. Capture the first unsupported instruction
   with member/sub/PC/offset/mask/RNG seed/draw counter. Acceptance: a native
   source-backed trace reaches exactly the selected ID2..5 start with life1500,
   boss slot0, real inherited ANM/flags and effective death/timer callback41.
   The existing restricted matrix's first blocker for sub42 is EX136 at38640;
   this is not evidence that the timeline/effect prelude is implemented.
2. **General entity frame and effect ownership.** Integrate resumable main and
   child contexts, call parameters, source motion, ANM and effect-pool lifetimes.
   Implement the opcode closure above; only project out a visual field after
   proving it cannot affect RNG, lifetime, pool occupancy or another exposed
   consumer. Acceptance: trace both four-familiar waves, immediate child spawn,
   same-frame child-context installation and all mask changes. Source-oracle
   comparisons must include pool-full and form-switch traces, not just empty
   pool/human-state happy paths.
3. **Shots, damage and terminal replay.** Dispatch requests through exact gates,
   allocation, transforms, ANM phases, collision and cancellation. Add actual
   SHT-derived player movement/shots and familiar-to-parent damage. Acceptance:
   paired trajectories alter aimed shots and opcode67 movement as predicted;
   capture and timeout produce different, correct endings; independent replay
   checks all collision phases until the transition is complete. Search may
   branch/merge only on equivalent full world state, never player position alone.

Initial explicit inputs include selected ID/difficulty, shot type/power, player
position and lifecycle/form state, input trajectory, full RNG state, numerical
profile, time-rate/freeze flags, and entry provenance. A synthetic supplied
checkpoint can test integration but must remain labeled synthetic until its
state is derived from this entry. No seed, full pool, empty pool, default rank,
invulnerability or no-shot condition may be invented to turn a blocker into a
purported full solution.

The decisive next test is a **full practice-entry trace through one real terminal
callback**, not a longer sub40/41 fixture. Compare PC/clock/call stack, allocated
entity identity, positions, selected ANM/sprite, successful bullet allocations,
shared seed/draw counter, damage and terminal flags each frame against an
independently source-backed offline reference. Validate all four difficulties,
human/youkai transitions and relevant player choices separately. Extend this
same contract to the remaining indexed families without weakening completion.

## Probe log and unresolved evidence

- Native DAT inspection confirmed both real wrappers and callback literals;
  source guard inspection rejected the hypothesis that practice ends through19.
- The independently inspected practice closure predicted a constant sub-ID shift.
  Comparing all175 raw records then confirmed that prediction without executing
  unsupported world behavior.
- Relevant existing `reports/native/{members,spell_sites,subprograms,timelines,
  slice_matrix}.tsv` provide persistent structural identities. The scratch C++
  inspection executable and extracted resources stay ignored; original DAT and
  preparation artifacts were unchanged.
- World event traces, entry RNG derivation, complete background/effect lifetime
  projection, full damage/player-shot execution, source-derived terminal routes
  and retail executable agreement remain unverified. There is no runtime proof
  of full closure merely because the static opcode queue is known.

This is a source-backed descriptive contract with a verified structural
prediction, not an operational complete-world replica. A differing effective
callback, shared RNG trace, child scheduling order or terminal collision phase
in the pinned source reference would falsify the corresponding model and must
produce a regression test before optimization continues.
