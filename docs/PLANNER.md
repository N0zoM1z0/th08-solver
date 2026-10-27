# Native planner and experiment protocol

The active owner workflow is offline Native scoring for Extra and complete
Lunatic4A→FinalB, exporting `ysopwnab` replays. Retail Wine playback follows a
qualified NMNB/all-capture score; online MPC is paused. Historical live protocols
below remain component evidence. [Status](STATUS.md) owns the current targets.

Each new scoring episode first captures the real owned Wine retail seed at the
pre-RegisterChain boundary, records its entry/provenance, and keeps it fixed for
offline Native candidates. Native does not select a replacement seed. Wine seed
acquisition precedes planning; qualified retail replay validation follows it.

The target is continuous No Miss No Bomb runs for all twelve team/solo shots,
four standard difficulties and both final routes, plus Extra, with input tapes
and real `.rpy` files. [Status](STATUS.md) owns results; [Validation](VALIDATION.md) owns commands
and provenance; [Architecture](ARCHITECTURE.md) maps ownership.

## Truth and proposals

The native runtime decides what happened. Its gameplay updates, shared RNG, input
latch and collision predicates retain their original order. Observation hooks do
not change those values. The actual Release binary must pass the pinned replay
fingerprint corpus before its results are presented as matching that corpus.
Fingerprints compare the published 25-field projection, not every internal state
or every possible input. Fresh action replay alone is a determinism check.

Offline proposal search may merge nearby positions, prune beams, use sampled futures and
learn scores. These choices can lose a valid route. They cannot establish safety
or prove impossibility. Every accepted proposal must survive a complete native
rollout, or reach the actual case terminal, before a bounded prefix is committed.
Capacity overflow, interrupted children and update exceptions are errors rather
than empty hazard sets. A mismatch between verification and committed state stops
the run. Search exhaustion, runtime failure and budget exhaustion remain distinct.

## Spell capture and score objective

[Issue #11](https://github.com/N0zoM1z0/th08-solver/issues/11) sets the scoring objectives:
complete NMNB, capture every card in the chosen route's source catalogue, then
maximize actual terminal displayed score. A skipped conditional Last Spell stays
in the coverage target. Lunatic4A/FinalB has37 cards; Extra has14. Other branches,
difficulties and Last Words require separate runs. These are run-level capture
goals, distinct from populating every Spell Practice entry in a persistent profile.

Current outcomes and historical failures belong in [Status](STATUS.md); this
section describes the implemented mechanisms and their limits. Current Native
experiments cover Extra and the complete Magic Team Lunatic4A→FinalB route.

### Capture-aware Native horizons

`--verify-captures 1` adds the enabled source capture audit to Native branch and
owner acceptance; it requires `--require-all-captures 1`. A closed failed encounter,
unmatched lifecycle event or missing completed-stage gate returns
`CaptureFailure` and cannot certify a prefix. An open card remains eligible.
The shared boundary check validates the stage that just ended at a handoff:
`currentStage` can already name the next stage while the old clear flag persists.
Only the real final terminal validates the complete visited-stage catalogue.
The audit observes original updates without changing gameplay or RNG.

This optional goal rejects a future loss of capture within the checked horizon;
it does not guarantee a later capture beyond that horizon. Whole-run acceptance
still requires the complete source catalogue. Historical safety-only profiles
keep their declared behavior unless this control is explicitly enabled.

`--policy-horizon` and `--policy-beam` separately control proposal guidance.
The learned value-feature interface retains its fixed24-update schema. Other
policy horizons use ordinary single-action decisions, freshly generated on
each guided Native update; they do not fabricate longer learned-feature paths.
Native verification, rather than the proposal horizon or beam size, certifies
the actions. Wider guidance is tested, with no scoring gain claimed for it yet.

### Complete-tape score refinement

`th08_score_refine` starts with our own complete Native input tape. Before
changing it, an isolated Native child must finish the entire tape NMNB and
capture the source catalogue for every visited stage, including gated Last
Spells. The reporting tool explicitly enables the lifecycle audit; reporting
and selection share `score_capture_coverage`, so they cannot silently use
different capture goals.

The refiner advances the owner through the certified incumbent. At spaced
boundaries after a64-update phase warmup, it tries two form alternatives or two
fire alternatives over an explicit update window. These edits have no seed,
card-number or human-input lookup. Each trial forks the actual current world
and executes the **entire remaining tape** with its own action-dependent RNG,
bullets, actors, items and transitions. It accepts only complete NMNB/all-capture
results with the identical expected spell set, identical final stage and a
strictly higher actual terminal score. Ties retain the incumbent. Final owner
execution must match the selected trial's complete41-word terminal projection.

This evaluates delayed bonus, clear rewards, PIV and carried-item effects through
the source instead of adding an approximate local economic reward. Trial death,
fault, timeout and tape exhaustion never certify an edit. All attempts and the
initial incumbent evaluation are charged; interrupted branches charge the full
remaining-tape cap. A search budget stops new trials and leaves the incumbent
executable. The exported final tape still needs fresh following and natural
original-writer replay verification.

The current proposal family changes only focus or fire bits and retains the
remaining timed inputs. An early kill or form change can make that suffix unsafe
or out of phase, so this bounded coordinate search can reject useful strategies.
It neither proves optimality nor supplies retail or online-FPS evidence.

### Offline checkpoints and compute controls

The owner prioritizes higher complete score before reducing compute cost.
Native `th08_shadow_style` therefore accepts explicit larger search budgets and
verification/publication horizons through2048 updates. Lead, nominal-prefix
reuse and replanning interval are independent recorded controls. Defaults retain
H64/publication64,110ms, lead8, interval8 and full nominal reuse. Live Wine
controls and scheduling are unchanged.

An offline profile can use lead1 and nominal reuse1 so the owned rollout makes
fresh policy decisions after one input, rather than copying nearly a whole old
tail. Larger certified execution blocks let it spend time on the longer future.
Every replacement still requires a complete Native horizon or the real terminal;
when no certificate remains, retries keep at most the original8-update spacing.
Longer budgets are search opportunities, not evidence of better score or safety.

`--prefix-actions FILE --prefix-updates N` executes the firstN inputs literally
from the declared original entry, observing every actual update in both policy
histories. Fresh planning then continues the same world, RNG, carried projectiles,
items and actor lifecycles. Prefix execution costs remain in owner work. An early
terminal before the requested boundary is rejected. This supports a generic
offline edit/replan/complete-score loop without requiring a serialized checkpoint
or reusing one future across different actions. Final full-entry fresh execution
and natural replay checks still apply.

### Source economics and capture boundaries

The restored [Spellcard.cpp](../third_party/th08/src/Spellcard.cpp) distinguishes
an encounter, capture validity, the final captured flag and delayed bonus payment.
`EndSpell` records a capture when the timer-transition failure flag is clear and
capture validity remains set. Ordinary timer callbacks invalidate capture in
[EnemyManager.cpp](../third_party/th08/src/EnemyManager.cpp); surviving that attack
without dying does not suffice. Timed survival attacks have different script/flag
handling and can award a capture at zero remaining time. A timer value alone is
therefore not a classifier. Misses and bombs invalidate capture through
[Player.cpp](../third_party/th08/src/Player.cpp). Capture statistics are updated
only outside replay mode: watching a saved replay does not fill profile captures.

[GameManager.cpp](../third_party/th08/src/GameManager.cpp) stores score in units
of ten displayed points: `AddScore(points)` adds `points / 10`. Spell bonuses and
point-item values already use displayed units. Do not compare raw journal scores
directly with videos or scoreboards. Source score clamps at999999999 stored units.

Several coupled mechanisms matter more than a distance-to-bullet reward:

- Faster ordinary captures preserve bonus and award100–1000 time orbs, reaching
  1000 within the first one-seventh of the limit. The native timeout-spell path
  requests700. These are pending requests, emitted in deferred batches, not a
  guarantee of that many realized pickups. Time-orb pickup also increases the active spell bonus by8000, subject
  to its source limit, and increases future point-item value.
- The stage Last Spell operand compares collected orbs **plus pending capture
  reward and field orbs** with the stage requirement. Lunatic thresholds for
  1/2/3/4A/5 are3000/7200/8800/9999/9999. The clock also affects the FinalB clear
  reward. Omitting an attack by missing its gate is not a complete capture route.
- [ItemManager.cpp](../third_party/th08/src/ItemManager.cpp) uses collection height
  or max-value state to price point items. Extremely human gauge doubles their
  pickup value. Moving above the collection line can attract items; eligibility
  also depends on power/form/shot. Attraction alone does not set `isMaxValue`:
  price still uses the item's height when collected. This creates a positioning
  objective that can require staying high until pickup, not merely crossing up.
- Ordinary time-orb pickup pays `max(100, 10 * floor(totalPointItems / 2))`,
  except the source's special orb mode and2000-stage-point branch. Conversely,
  every two time orbs add10 to future point-item value. Early point collection
  therefore raises later orb income, while earlier orbs raise later point income;
  a controller should value both carried economy state and immediate pickup score.
- Native graze awards2000 or4000 displayed points and1/2/3 graze count depending
  on gauge. At extreme youkai gauge against a boss, it can generate additional
  time orbs, including extra spell rewards. Ordinary bullets graze once; lasers
  follow a separate repeated-graze path. Graze also raises rank, changing later
  nonspell patterns. The source resets boss bullet-rank influence on spell start.
- [Gui.cpp](../third_party/th08/src/Gui.cpp) clear rewards include stage base,
  graze, point items and time orbs, then final-stage life/bomb reserves and FinalB
  clock value, difficulty and configured starting-life scaling. Its displayed
  bonus is ten times the intermediate integer sum. Retaining NMNB reserves and
  collecting items therefore have consequences beyond immediate pickup score.

`Player::AwardGraze` increments the displayed counter by3 at extreme human gauge,
2 at moderate human gauge and1 otherwise. Compare actual award events and gauge,
not just that counter. The opt-in read-only audit now counts total events and
the two extreme-gauge subsets, including repeated laser contacts. Ordinary boss
spell graze at extreme youkai can request three time orbs for a team; laser calls
that suppress extra items request only one. Those pickups also affect spell
bonus and future point value. `Spellcard::AddBonusProgress` clamps at its score
limit, so faster capture need not maximize bonus when sustained orb income can
replenish it. Full capture deadlines remain a hard evaluation requirement.

Optimize these with complete native outcomes and explicit resource costs. Ordinary
speed kills, controlled familiar cancellation, item collection and timed grazing
can conflict; local graze or damage alone cannot identify the highest-score run.
The [PND score list](https://thscore.pndsng.com/index.php?th=8), checked October25,
lists a6,168,365,790 Marisa/Alice score and higher scores for other shots. It is a
score-attack reference, not an established matching-NMNB bound for this solver.

### Collection and form controls

The opt-in `score` preference adds two independent current-memory controls:
`Configuration::scoring_collection` and `scoring_aim`. The Native adapter owns a
copy of visible point/useful power items, literal boss position, current power
and the loaded SHT collection line. It reads no future drops or RNG. Collection
starts with at least four useful items and fewer than64 observed bullets, aims
12 pixels above that source line and retains its intent until items disappear,
eligibility ends or a boss arrives. This hysteresis prevents turning back while
the last items are still approaching. Boss aiming uses its actual position and
the ordinary400-pixel vertical anchor. These are proposal goals. Complete Native
rollouts decide safety with the explicitly selected budgets; the Native harness
default is H64/lead8/110ms/60ms.
Baseline actions and tie ordering remain unchanged. The scoring goals reject the
fixed-height delayed executor and existing branch-value models rather than
silently mixing unsupported control or training assumptions.

`score-survival` adds a soft focus preference only while the actual source
`ENEMY_FLAG_TIMEOUT_SPELL` is visible, for a team that can switch forms. Its0.01
node cost uses the scale of the existing action-change cost; ordinary attacks
retain the original score preference. This signal describes source timer handling,
not a proof that HP can never finish the attack early. The blanket all-spell
form prototype is retired after two complete-duration failures. These preferences
use no spell-ID decisions or input scripts and remain opt-in.

`score-harvest` allows a collection trip during a quiet nonspell boss interval:
the existing four-item/<64-bullet entry gate remains; an active spell or128
observed bullets ends a boss trip. Its existing edge-cost term is2 with40-pixel
margin. The baseline and original `score` behavior remain unchanged. Complete
scoring outcomes and weaker variants are recorded in Status.

`score-precollect` also reads unconditional point-drop counts on currently active,
damageable enemies when no boss is present. Current power decides whether a
power-or-point drop counts; periodic/random drops are omitted. The count-weighted
current X is a proposal target, not a predicted RNG scatter. A qualifying batch
can start the collection climb before its kill. New shot commands are withheld
until reaching the configured collection goal (12 pixels above the SHT line
by default, or source pickup reach+2 with priced collection); existing shots remain
live and Native branches decide their actual effects. Real drops continue the
trip until collection; empty actor/item state clears it. Precollection remains
experimental, with failed and weaker complete trials retained in Status.

Boss collection, precollection and source-survival focus strength are independent
recorded controls. The survival prior defaults to0.01; changing its nonnegative
strength does not affect ordinary attacks. No failed/incomplete branch certifies
an input prefix. The first ordinary-spell milking variants were discarded after
failed full scenes; the maintained attack-pacing control below supersedes them.
Extending an attack must still improve a complete valid terminal score.

### Stream proposals and attack pacing

The experimental `score-flow` adds a source-survival-only stream goal, with a
recorded5.01 focus prior. Every64 updates it ranks70 stationary targets on a
32-pixel grid over x48..336/y208..400. Current ungrazed ordinary bullets with
speed at most3 pixels/update are extrapolated192 updates in16-update samples.
Contact bounds include the source's20-pixel bullet expansion as well as the
bullet/player graze half sizes. The target cost combines unique contact opportunity, a body-danger proxy and
travel distance. Current lethal bodies and movement bounds exclude targets.
Local proposal contact reward3 and anchor weight0.03 apply only in this mode;
native collision/margin costs and true-RNG safety verification remain intact.
This long linear hypothesis is not a192-update Native certificate. The complete
precursor had empty macro contacts because it omitted the20-pixel expansion;
its valid score does not validate that earlier target. The mirrored-stream
contract catches the omission and checks goal direction. The source-survival-only
profile still has complete-duration failures in Status.
The default controller remains unchanged.

The separate opt-in `--attack-pacing 1` uses current source HP-to-callback,
remaining updates, damage-reduction state, extreme-youkai state, collected orbs,
orb value and bonus decay. After60 mature observations it estimates DPS from a
120-update damage history. That measured firing rate is retained through the
income experiment; it is not an exact future damage model. The default capture
reserve is1.5*HP/DPS+240 updates. With
700 additional updates,16 nearby slow ordinary bullets and8 macro contacts,
it releases new fire and samples income. It waits at most1200 updates for the
gauge/income experiment; after source extreme-youkai activation it measures240
updates. Orb rate*(current orb value+8000) must exceed current bonus decay to
continue. The reserve or low remaining HP restores fire. This is a proposal
proxy: bonus caps, future decay, PIV and clear-bonus effects are not an exact
terminal value model. Carried shots and the source20-update shot cycle remain
owned by Native; releasing the key does not instantly remove damage.
`--capture-factor` and `--capture-padding` independently expose that reserve;
defaults remain1.5 and240, and the1.2/120 complete trial retains its extra costs.

`--attack-focus-bias` independently controls the pacing form prior (default5).
The Native harness can try `--baseline-recovery 1` after a failed nominal future.
The baseline owns movement; the branch-owned primary policy observes every
future packet and supplies fire intent, including transitions back to capture.
The explicit `--recovery-style expert|zen` chooses another movement policy;
`--recovery-fire capture` instead keeps recovery fire enabled to prioritize ending
an attack after a rejected nominal future. Both require enabled recovery. The
default style/fire remain baseline/preserve. These are additional Native-checked
proposals within the same total budget, not permission to publish a failed tail.
The fixed input lead is preserved, and only a complete Native future certifies
publication. Additional safety attempts/adoptions and all rollout work are
reported. `--search-budget-ms` explicitly selects the total search budget. Both
the synchronous Native harness and live client expose the pacing, reserve,
price/staging/form and baseline recovery controls. The live client additionally
accepts `--lead-updates` (1..32) and `--publish-horizon`, bounded by the verified
horizon and wire capacity. Larger budgets do not pause Wine. Each publication
must still match all executed inputs and arrive before its first changed input.
Search rows record owner progress and queue sizes; reports include publication
consumption/slack, rejected future work, stale plans and owner timing. Synchronous
throughput is not online FPS. Actual live outcomes and costs belong to Status.

### Price-aware positioning

Point goals use current source pickup geometry. `--priced-collection 1` sets
player target Y to collection_line-pickup_reach_y-2, clamped to movement bounds.
The intended item pickup is above the price line. Auto-attraction
alone does not imply full value. `--upper-stage 1` keeps that economic staging
target when no boss is registered, instead of returning to the bottom after each
trip. Boss registration restores the ordinary goal; current lethal regions and
Native verification still own safety. `--collection-bias` prefers unfocused
movement while collecting; `--human-bias` separately sustains that form outside
an active graze goal. This supports extreme-human point doubling and shooting
gauge ramp continuity. Both defaults are zero; all four controls are opt-in and
recorded, with no card, seed or human-input lookup. Height is a proposal goal,
not a promise about prices during transit. Pacer/source-survival graze goals
retain priority over the general human preference. Precollection, when enabled,
uses the same price-aware target before restoring new fire.

`--hit-income 1` is an independent, default-off experiment. A current SHT
descriptor with negative extreme-gauge behavior can generate human shot-hit orbs
before enemy HP suppression. With no boss, the Native adapter copies accepted
nonboss shot targets with at least24 remaining reduction updates. The proposal
ranks current targets by remaining window and travel distance, aiming64 pixels
below the chosen actor for upward shots. Item trips and boss/graze goals retain
priority. This is a current-memory hypothesis, not a future hit, reward or safety
promise. Native rollouts retain all carried shots, RNG, HP and lifecycle changes.

Native `--drop-target 1` is a separate, default-off input goal. For current
damageable enemies above the player, the adapter ranks unconditional large-point
drops at the current power by `count / (HP * (1 + abs(dx)/48))`. Stable ties keep
the first source slot. The goal aims at one real actor; the existing count-weighted
centroid remains the precollection input. Bosses, actual item trips, hit-income
windows and active graze goals retain priority. This predicts neither RNG scatter
nor a successful kill. Native continuations decide safety and realized income.
The selected Extra gain uses this goal together with the recorded focus-pulse
and larger-search profile; its result does not isolate the goal's contribution.

Native `--drop-follow 1` requires `--drop-target 1` and defaults off. It extends
the same current-enemy proposal to actors below the player when an in-bounds
shooting position64 pixels below the actor exists. Coordinates use playfield
pixels, with Y increasing downward. Ranking becomes
`count / (HP * (1 + abs(dx)/48 + abs(shooting_y-player_y)/64))` with stable pool
order on ties. These distance scales are heuristic penalties, not flight times.
The proposed Y is the lower of the collection goal and shooting position on
screen; existing item trips, hit-income and graze goals still override it.
Every observation recomputes the target without retaining an enemy pointer.
Native execution owns collision, actual shot hits, item production and prices;
descending to shoot may reduce collection prices. Historical profiles keep the
original horizontal-only selection when the new control is off.

Native `--parent-chain-bias VALUE` defaults to0 (off), accepts0..10000 and
requires `--drop-target 1` when positive. The adapter then excludes attached
children from shooting-lane candidates; it preserves their ordinary impending
drop totals for precollection. The selected root supplies only a Boolean current
attachment-link cue. No pointer crosses the adapter, no chain is traversed and
neither linkedChildCount nor an assumed child payout enters the ranking.
Revision2 also copies the selected root's signed remaining reduction updates.
With the chain cue, fewer than24 reduction updates, switchable forms and no
boss/pickup trip or full four-item pending batch, the policy adds the configured
focused-form bias and suspends the ordinary human bias and competing hit-income
lane. It aims at the root in youkai form so familiars can survive to the source
parent reward. Reduction windows retain human hit-income guidance. Pending
batches yield form even when bullet density delays collection movement, because
source auto-attraction can already collect and price their items.
Pickup trips immediately restore ordinary form/aim;
root removal and boss registration clear the prior on the next observation.
The bias uses existing per-update preference-cost units and is soft: movement,
hazards and verification can select another form. This can lose human shot-hit
income, kill timing or pickup value; complete Native execution decides profit.

Source stage-clear scoring is separate from point pickup pricing. Before
difficulty/configuration multipliers, `Gui::UpdateStageElements` adds5000 raw score per
stage point,100 per time orb and50 per graze, plus its stage/life/bomb/night
terms. Raw score units display as10 points. PIV does not directly multiply that
clear sum. Actual price ceilings, later point income, capture bonuses and final
clear rewards therefore need separate accounting; an orb-to-PIV conversion is
not a promise of a fixed final-score gain.

Native `--economic-proposals 0..4` defaults to0. After a complete safe forecast,
it retains that incumbent and tries up to the requested number of32-update
fire/focus alternatives, preserving the immutable lead and incumbent movement
inputs. Each alternative re-executes the full Native horizon with its own RNG
and policy continuation. Only strictly higher endpoint cash replaces the
incumbent; ties and failed/incomplete futures retain it. These are horizon
proposals: only a higher complete NMNB/all-capture score selects a progress file.
Extra work shares the declared total search budget, with a2000ms soft comparison
slice checked before each candidate. An in-flight candidate may consume the
remaining total budget. Reports record attempted/adopted comparisons and include
their rejected futures and native/proposal costs in the ordinary totals.

### Audit and publication contracts

Native-only `--farm-to-timeout 1` is an optional pacing proposal for capture-
optional experiments. It leaves the existing damage probe and gauge/income
sample intact. After a positive sample, it can keep no-fire past the ordinary
capture reserve; a rolling240-update realized-orb sample restores fire when the
stream becomes unproductive. The unchanged cash+8000 bonus proxy is heuristic:
bonus caps, lost capture payout and future PIV make it unsuitable as a terminal
score oracle. Carried shots can still kill the boss, and recovery can still fire.
Actual timeout, safety and score are decided by Native execution. The default
is off and strict all-capture CLI goals reject this experiment.

Offline score experiments can independently permit bombs and require captures.
`--allow-bombs 1` admits X only in the explicitly configured Native owner,
rollout, buffer and fresh follower; default transports retain their NMNB input
mask. No-miss failure checks stay mandatory. `--require-all-captures 0` allows
capture loss while the source audit still records the full catalogue/outcomes.
The complete-tape refiner can test bomb windows against a complete terminal;
the planner's bomb proposals are a bounded recovery family. Native source owns
all inventory, bomb timing and scoring effects. An ignored X input is not an
actual bomb use. [Validation](VALIDATION.md#bomb-and-capture-constraints) owns CLI
controls and verification; strict historical records retain their identities.

Per-spell score audits snapshot collected orbs, point count/value and score after
the preceding capture payout is settled, then again at the source end hook.
The frozen story regression checks fast80-update transitions and the unchanged
complete replay-row hash. Spell bonus is reported separately: it is deferred
until its reward effect or the next start, not necessarily included at end.
Carried items can still be collected during the new spell; these are temporal
boundaries, not causal attribution or a reset of field state. Graze audit counts
also distinguish ordinary extreme-youkai events from extra-item-suppressed
contacts. A universal three-orb multiplier would overvalue the latter.

The Native experiment tool exposes verification64/128/256 and publication64/128
independently, with publication bounded by the complete certificate. Optional
held recovery mirrors the live client's family/order: after a failed nominal,
held directions precede policy prefixes; if none certifies, guided tails use the
remaining budget. An earlier successful longer certificate can supply another
checked publication window after a new search fails. Its executed prefix must
still match; the failed search supplies no inputs. The live `h128` profile uses
128 verification/64 publication, while historical `h128-held` publishes128.
Both retain lead8, interval8, total110ms and proposal60ms. Longer verification is
not uniformly better: the retained4A counterexample fails under128.

Remaining source-based opportunities, including net reduction-window income,
are kept in [Status](STATUS.md#unresolved-problems-and-proposed-next-work).
Current pacing uses observed income; it does not target enemies or certify those
opportunities as incremental profit.

## Execution and ownership

`th08_plan` forks a single-threaded headless session. Each branch owns copied
mutable managers, pointers, RNG and carried world state. Immutable resources share
copy-on-write pages. A child records collision-enabled bullet boxes, direct and
pooled laser calls, and lethal body/trail rectangles along a nominal input plan.
The recording is exact for that branch; another input sequence may create different
hazards, damage, cancellation, graze, items or RNG consumption.

The movement proposal uses float32 SHT speeds, native arithmetic/clamping and the
one-update recording-mode input latch. Beam search considers nine directions with
both focus inputs. Native verification rejects path-dependent counterexamples;
their recordings can guide another proposal. Only validated prefixes are committed.
Checkpoint-holder processes enable backtracking without replaying the whole prefix.
All attempted rollouts, including failures and reconstruction, belong in cost totals.

An API update is not always a gameplay calculation. Native stage handoff calls can
return before the gameplay counter and replay callbacks; replay playback can instead
restart the calculation chain within one API call. The planner observes actual
gameplay-counter increments without changing the existing trace/RNG hook. Recording
mode accepts one calculation or an observed control-only stage change and rejects
unexplained skipped/batched calculations. Reports preserve exact event indices and
separate path counts from costs over all attempted branches. Natural `.rpy` validation
uses observed calculations, never an assumed offset based on the number of stages.

`--prefix ACTIONS` reconstructs an earlier input tape through the same native
session before checkpoint holders fork. Use its original stage/story/difficulty/
seed/shot/final-route settings. Raw actions have no checkpoint header; another configuration
defines a different experiment. Mutable world/RNG/input state carries through every
update; planner tie RNG, quiet windows and proposal history start a new episode.
The final tape includes the prefix. Its raw SHA and attempted/completed native
reconstruction counts are reported separately and included in total native work.
Explicit `--frames` and `--seconds` are required; frames include the whole prefix.
Deaths, invalid input bits, checkpoint mismatch and actions after terminal reject
the episode. A prefix ending exactly at terminal may complete without new search.

Quiet windows withhold shots. This reduces shot-hit RNG draws; it does not freeze
RNG or prove that hazards are path-independent. Existing shots, graze, collection,
focus/form changes, damage and other source consumers still matter. Seed and shot/
focus manipulation must be implemented through realizable inputs, never by changing
the native RNG. Sampled hazard unions are heuristic constraints, not universal bounds.

## Diagnose when information arrives

For a blocking hazard, measure when its geometry becomes stable over sampled input
variants, when the planner observes it, when it becomes lethal, and how much lead
time is needed to escape from the current state. If observation arrives too late,
test a longer native rollout. If the geometry remains input-dependent until escape
is impossible, test earlier positioning or realizable shot/focus sequences.

These measurements explain concrete failures; they are not a completeness theorem.
Identical samples do not prove a global lock time. Preserve the sampled variants,
failures, event times, geometry and consumed native updates. The hazard probe is a
diagnostic tool; spell IDs belong in its configuration and regression cases, not in
planner branches. Large-area hazards may justify a generic response after measurement.

Long-horizon viability funnels, failure-directed checkpoint jumps and concurrent
candidate verification are follow-up mechanisms, not assumed completed features.
Keep finite failed families visible instead of repeatedly charging the same retry.

`--diversity-cell N` optionally retains the best exact proposal per N-pixel spatial/
focus bin before filling the beam by score. Zero preserves the global-score baseline.
This can retain an early exit that center-seeking scores discard; it changes pruning,
not collision semantics. Record bin/candidate counts and selection time. A larger
rollout alone can worsen pruning, so compare complete outcomes and all retry costs.
The shared selector first finds each bin's best source node, then uses partial
rank selection for bin winners and any score-fill slots. Only the retained nodes
are sorted. Ranking by score and original source index preserves the former full
stable sort's exact membership, output order, ties and parent/action fields;
hash iteration never determines a selection. This reduces ranking work without
changing the proposal budget or strategy.
The bin index uses a flat, linearly probed table with full `(x,y,focus)` equality,
so hash collisions cannot merge bins. A power-of-two table has at least twice as
many slots as input nodes, so occupancy is at most one half. The online policy
owns reusable selection buffers; all occupancy and logical contents reset on
every call, including empty input. Private retained-node storage prevents input
aliasing. The existing three-argument offline interface uses a fresh workspace.

## Current-memory online policy

This component uses current process memory without Native future steps.
[Issue #5](https://github.com/N0zoM1z0/th08-solver/issues/5) preserves its earlier
stage/learning campaigns and evaluation profiles. Those historical 4B/Extra
investigation orders are not the active queue; Extra scoring uses the separate
shadow rollout path below. Current process memory is authorized input. Pixel
recognition, menus and a strict human-visible information boundary are not
prerequisites.

`online::Observation` owns copied current player/movement state, active offscreen
bullets, collision-disabled flags, transform/acceleration fields, pooled-laser
timers and measured motion, direct collision calls and lethal body rectangles.
The native adapter skips the pool sentinel. The policy receives no Session or
native manager pointer and does not execute future native calculations or sample
gameplay RNG. A live native session advances once per submitted input and
stops on death; it never rewinds. The kinematic forecast and risk regions are
heuristic proposals. The next actual native calculation determines survival;
online choices lack the offline planner's precommit native verification.

Future birth-cue work must distinguish emission time, first collision and the
earliest responsive movement. Pool order can defer a child's first update until
the following calculation, while the root player movement is already latched.
A matching emission point does not supply the child's flight/collision geometry;
complete frozen-seed execution and replay gates still decide controller coverage.

Float32 movement retains the one-update recording latch. The policy searches 18
movement/focus inputs with half-pixel proposal deduplication, per-instance tie RNG
and optional reachable-envelope pruning. Deduplication is not equivalent-world
merging. Observations must cover consecutive API updates, including control-only
stage transitions. Explicit prefix reconstruction warms bullet velocity, spawn,
direct-laser and body history without consuming search tie RNG; a continuation is
a new policy episode, not a bit-identical resumption of an earlier search.
The shared online/offline half-pixel key remains exactly
`uint32_t(lround(coordinate * 2)) & 2047`. On the tested glibc/x86-64 binary32
profile, an integer-significand path avoids the per-node library call. It retains
the original multiplication and signed modulo result; large/special values and
other layouts use the original library expression. This changes no deduplication
precision, collision geometry or tie order.
For validated finite forecast coordinates, the scalar AABB predicate evaluates
the four original rejection comparisons together. It retains inclusive contact
and the same margin-expanded bounds for hard and soft queries, while reducing
internal short-circuit branches. Hazard traversal and the first-hit return stay
in the existing order.
Velocity-rate estimation requires the same fired phase, collision-enabled mode
and active-transform mask on both observations. Installing or clearing a transform,
or a source ghost-to-real switch, may change velocity discretely; the difference
across that boundary is not continued as acceleration or angular speed. Current
supported transform state still takes precedence. Matching mode/mask does not
certify linear motion or exclude external program writes;
the remaining finite-difference forecast stays a proposal hypothesis.
The finite coordinate profile rejects NaN/infinity or magnitudes beyond 2^20
pixels instead of silently dropping unindexable hazards.

Optional mechanisms have explicit, default-off configuration. `--spawn-guard
N` adds soft risk around current actor shot origins, extrapolated by observed
displacement. It does not predict an emission schedule. `--diversity-cell N` uses
the same stable spatial/focus proposal retention as the offline planner and
records candidate/bin counts and selection time. Neither option changes native
collision semantics or guarantees improvement.

`--queued-shots 1` asks the existing native observation adapter for bounded linear
shot prefixes decoded from current main ECL cursors. The adapter copies geometry
into owned values; the policy never follows an ECL pointer. Supported prefixes
become proposal constraints; a constant-velocity soft tail remains after their
bounded duration. Coverage is partial: child contexts, dynamic operands, aimed
patterns and other unsupported emissions may be absent. Empty output does not
prove no future births. Reports name this current-program profile, count emitted
and unsupported preview records, and retain `future_native_updates:0`. Actual
unsupported gameplay execution still stops the case. This option adds no future
native rollout or branch-independent safety certificate.

`--queued-lasers 1` separately copies timed direct-hitbox warnings from the
existing current-ECL decoder. It recognizes the source callback selectors for
narrow, medium and wide beams, requiring observed stationary geometry and constant
selectors. Both the first lethal update and a currently visible disable boundary
are retained. Dynamic geometry and variable selectors are counted as unsupported;
uninspected control flow remains outside this partial preview. No spell or seed
chooses the decoder. Its owned rotated rectangles enter the same proposal collision
queries as observed laser calls, with the input latch unchanged.

The same option also copies supported, unsuppressed pooled-laser spawns decoded
by the native session's imminent main-cursor preview. The owned laser starts at
timer zero before update one, so its birth callback is included exactly once;
later updates use the existing pooled-laser proposal lifecycle. No native update
or operand resolver runs in the policy. Aimed/unsupported previews are counted
separately, not promoted into known geometry. The current origin and subsequent
script behavior remain partial assumptions; `queued_pooled_laser_records` and
`unsupported_queued_pooled_lasers` report this additional coverage.

`--spawn-clock 1` uses a spawning bullet's currently terminal ANM cursor and
unit-rate clock instead of its empirical size/duration estimate when supported.
The adapter copies only a one-based completion update into owned memory; it reads
one terminal header, without scanning/executing future code or resolving operands.
A null cursor completes on its next call. Blocked/interrupted clocks, variable
masks, other instructions and non-unit/frozen contexts supply no clock support.
Unsupported clocks retain the named legacy duration hypothesis and are counted
separately; zero support is not an empty-future certificate.

The final animation call still performs fractional spawn displacement before the
FIRED update. Fractional motion follows the source reciprocal multiplication order.
The existing current-velocity hypothesis resumes at activation: later transforms,
external cancellation and future ANM changes remain partial, not certified physics.
The option defaults off and reports `spawn_clock_records`,
`unsupported_spawn_clocks` and `spawn_clock_frames` separately from native work.

`--relative-turns 1` projects an existing bullet's source-owned relative turn
before extrapolating observed velocity. The native session supplies a bounded
single-transform view; the adapter copies angle, speed, timer, interval and repeat
state into the observation. The pure direction-change kernel runs only through
that bound, including its last turn. Beyond it, constant velocity is explicitly
a proposal hypothesis. Concurrent/aimed turns and subsequent transform programs
are not certified. The option defaults off and reports supported/unsupported
observation counts plus `relative_turn_frames`, without native lookahead or RNG
evaluation. This also handles a stationary bullet whose turn is already installed.

`--direct-turn-envelope 1` replaces a matched direct laser's single signed turn
with both turn signs up to its last observed angular speed. Before first
visibility, `--queued-lasers 1` can also supply a cue from absolute literal writes
to the main ANM angle in the currently visible same-time ECL batch. The read-only
decoder includes both forward conditional paths, filters difficulty and rejects
variable angle writes, backward loops, unsupported instructions and truncated
records. It inspects at most 64 records inside an adjacent-sub-table byte bound;
the final subroutine has no such bound in this interface and supplies no cue.
No operand/RNG evaluation or native future update occurs.

The model intersects the player's local circle arc with the laser rectangle.
Oriented endpoint half-planes test arc membership; a cosine/dot tolerance would
over-expand narrow arcs near zero angle. Narrow arcs intersect the half-planes,
wide arcs use their union, and a hemisphere check excludes the opposite ray at
near-zero reach. The broad phase retains its radial envelope and float rounding
allowance. This
is proposal uncertainty under a last-speed/repeated-literal-batch hypothesis,
with the existing linear origin/static geometry assumptions retained. Later
scripts can change rates, origins or lifetimes. A zero cue is not an empty-future
certificate. The option defaults off; reports retain cue/unsupported counts and
the number of expanded laser forecast frames. Native execution still decides
actual survival, and all stage/seed comparisons retain failed alternatives.

`--warm-fallback 1` rechecks one suffix of the previous selected online proposal
when the new beam returns a nonzero partial horizon. Decisions must be consecutive,
and the current latched movement/focus must match the preceding sampled action.
The suffix drops that action and repeats its last input to fill the new horizon;
every position is recomputed from the actual current player state and checked
against the new forecast. It reuses no old hazard forecast or native future state.
A strictly longer prefix replaces the beam; depth ties keep the beam without extra
tie RNG. This check precedes the optional held proposals. Reports separately count
`warm_paths`, `warm_forecast_updates`, `warm_selected` and `warm_invalidated`.
The cache belongs to one policy instance, and the option defaults off. A longer
modeled prefix still depends on the partial forecast and is not a survival guarantee.

`--tail-check T` extends forecast/capture to H+T while retaining the H-update beam
and reported search depth; H+T is bounded by 200. A complete beam's retained
endpoints are visited in stable score order. Each gets up to eighteen held-input
continuations, with the longest modeled suffix preferred and score breaking ties.
The first full suffix stops enumeration without extra tie RNG. Partial beams and
held/warm fallback suffixes receive no terminal check. This is endpoint proposal
retention under a partial forecast, not H+T branching or a native safety proof.
The option defaults to zero. Reports count all endpoints, paths, checked updates
including failed collision queries, replacements, attained tail depths and the
sorting/check time separately. A longer tail also changes forecast coverage;
compare complete cases and retain parent failures and costs.

`--held-fallback 1` checks all eighteen held direction/focus proposals only when
the beam returns a nonzero partial horizon. Every proposal starts with the actual
latched input and uses the same movement, bounds and forecast collision queries.
A strictly longer surviving prefix replaces the beam result; equal depths retain
the beam, and held ties follow the existing unfocused/focused direction order.
This proposal family consumes no search tie RNG. Reports count checked paths,
forecast updates and selections separately from native work. It can retain an
escape lost by finite beam pruning; it cannot make an incorrect forecast safe or
avoid an already-latched collision. It defaults off for controlled comparisons.

Optional risk files use complete TSV rows, explicit horizons 4/8/16/32 and the
prototype's minimum count of 200 for fine/segment entries. Global entries retain
all sample counts. Malformed/truncated rows and duplicate key/horizon records fail
setup. This corrects a prototype parser that read across record boundaries and
silently lost alternate segment/global horizons. Missing model entries remain
unknown to this heuristic, not certified empty space. Prototype tables are legacy
training data; new experiments bind each file's SHA and must not call reused
training seeds unseen. Teacher-conditioned futures, rounded funnel paths and
retrospectively selected predictors remain diagnostic evidence.

Each decision builds the current segment's clipped 3x3 spawn-grid sums once,
then shares them across search nodes. Each center retains the original row-major
float addition order; running/prefix sums would change rounding. Weights and
depth factors are still applied in the original node-score expression. The grid
is owned by that search and rebuilt on every decision, with no phase cache.
Beam layers and dedup tables reuse policy-owned capacity. Every call clears all
logical nodes and generation marks, including after an early latched collision;
neither another policy nor a later decision can inherit proposal state. Capacity
is bounded by the configured horizon/beam and its eighteen successors per node.
After the latched root survives, search prepares the eighteen movement deltas
once from that call's immutable movement snapshot. Input direction/focus priority
and float multiply order are unchanged. Every successor still adds its own player
position and applies the original bounds checks; candidate order, keys, scoring
and tie-RNG consumption are unchanged. Deltas are local to the search call, so a
new snapshot cannot inherit another call's speed or multiplier. Deltas use native
position units for one modeled update.

The initial execution mode is `forward-measured`: observation copying, policy and
route-input decoration are timed before `Session::step`, with mean, p99, p99.9,
maximum and deadline misses. Native execution excluding measured file I/O, setup,
prefix cost and whole-loop wall time are separate. It waits for late decisions;
`deadline_enforced:false` is explicit. A future bounded-work/fallback policy must
be evaluated under its actual execution behavior before any real-time claim.
`--profile-components 1` additionally writes `components.json` and marks
`online.json` as `component_profiled`. Optional policy clocks separate history,
forecast construction, reach pruning/indexing, search and fallback work. Capture
and policy totals are disjoint; policy phases are nested within the policy total,
and spatial-selection time is nested within search. Do not add nested times.
Prefix reconstruction, live native execution/file I/O, loop residual, setup and
post-loop output have separate scopes. Post-loop output includes tape/log/report
serialization, quantile sorting and hashes, but excludes the component report
itself. These instrumented diagnostics locate work; uninstrumented Release pairs
still decide performance improvements. Profiling changes no policy inputs, RNG,
search budget or game pacing.
The current driver calls the owned native Session directly and advances it only
after a decision; it has no unpaused retail-memory/input bridge. A live 60 FPS
gate must let the game advance on schedule, align snapshots and input sampling
with the recording latch, and measure observation-to-input latency including
bridge overhead. Bounded work or asynchronous planning with an explicit fallback
must account for stale observations and missed decisions, then pass complete-run
and independent-seed survival checks under that actual timing behavior. Memory
access removes the need for image recognition; it does not establish those gates.
The owner-supplied local realtime analysis
guides the next algorithm phase. Its no-model VM measurements are external
evidence to reproduce on this build, including the actual spell-166 laser window.
Exact inverse rotations are prepared once per retained forecast laser after all
geometry/envelope changes, then shared by immutable queries in that frame. They
are rebuilt at each observation, preserving the original negative-angle float
products. Online spatial retention uses the observed movement bounds for a dense
bin lookup, with packed binary32 score/source-index ranks. Signed zeros share a
rank; infinities and stable source ties keep their original order and node bits.
Unsupported layouts, oversized/invalid bounds and out-of-domain nodes delegate to
the generic selector. No simulated states or exact coordinates are merged.
The optional CMake `TH08_ONLINE_ARCH=native|x86-64-v3` targets only the policy
library; an empty value retains the portable default. Reports identify that
library's architecture. Both targets retain `-ffp-contract=off`, without fast-math;
flags alone do not prove action equivalence or portability to another CPU.
These kernels pass unchanged complete actions and the full suite under the
measured profiles in [Status](STATUS.md).
Successor deduplication retains the original cyclic 11-bit half-pixel x/y keys
and focus bit. A layer-local bitset replaces hashing; bounds come from retained
parent coordinates and exact movement extrema. Marking remains serial and occurs
before lethal probes, so even discarded proposals keep their original key owner.
An underestimated domain expands to the full cyclic key space while migrating
all marks; `proposal_grid_expansions` records that work. Only buffer capacity is
reused between layers. No new coordinate or world-state equivalence is introduced.
`--planner-lag N` implements an optional deterministic delay model. Zero keeps the
synchronous policy and its complete action tapes. A search at observation u forces
the N submitted movement/focus inputs before its first new choice, then withholds
that result until u+N. The already latched input owns movement one; only the first
N-1 newly submitted inputs have moved the player at adoption. Plans own their
inputs and expected arrival, without a simulator pointer or native future.
Actual latched gameplay inputs (including shooting/bomb state), position bits,
movement snapshot, stage/spell phase and owning epoch must agree. Mismatches or
emergency replacement discard pending work without resetting observer history.

Each followed or newly adopted plan is checked against the latest forecast for
`--executor-check` updates (default 32). Checks hold the final input beyond a finite
proposal. Failure runs `--emergency-horizon 32 --emergency-beam 48`; a shallower
rescue cannot replace a deeper checked continuation. Delayed mode rejects warm,
held and terminal-tail options instead of silently ignoring their contracts.
Its reported depth is capped at executor_check; full planner/emergency search work
remains charged in the aggregate search counts. An unavoidable latched hit does
not launch or certify a delayed prefix. Native execution still determines outcomes.

Optional `--lazy-index 1` delays only the unused suffix of per-frame collision
indices while a delayed result is pending. Forecast generation and reach pruning
still cover the same full horizon. The executor initially prepares the larger of
its check and emergency horizons; a canceled pending plan extends the index before
starting a replacement full search. Indices belong to the current observation and
are never reused across observations. `indexed_forecast_frames` reports actual
construction work. Lag zero and the default flag retain eager indexing. Index
extensions remain executor work, including in component timing; they are excluded
from the reported full-search duration.

Optional `--hard-phantoms 1` applies a conservative category prior: currently
collision-disabled fired bullets (state 1) enter the ordinary hard kinematic
forecast. It requires no model, activation schedule or spell/seed branch. This
replaces their phantom-only soft-risk path; ordinary learned margins still apply
when a model is loaded. Disabled spawning states keep their existing handling.
The observation's native collision flag and collision-mode motion-history boundary
remain intact. `hard_phantom_frames` counts forecasted bullet/update pairs before
reach pruning. The default is off: additional obstacles can restrict escape, and
this hypothesis neither predicts activation nor certifies future native motion.

`--motion-envelope 1` additionally requires hard-phantom mode and copies the
current native canonical angle/speed into an owned alternative velocity for fired
ghosts. The forecast contains its observed-motion path and every modeled single
switch time to that alternative. Axis extrema form a conservative rectangle,
updated by repeated additions; no activation schedule, spell or seed is decoded.
The rectangle can overrestrict movement and does not cover arbitrary subsequent
native transforms. `motion_envelope_frames` charges the extra bullet/update
hypotheses. The default is off, and unused alternatives are not read by the native
adapter. Native outcomes and independent-seed comparisons still decide usefulness.

Optional `--child-births 1` reads a narrow current copied WAIT/child-pattern program
without resolving gameplay RNG. It projects a linear parent to the pre-movement
birth position, then encloses every angle through the birth and following update,
including both pool-order possibilities. Only a literal child WAIT that keeps
initial velocity unchanged is supported. This is partial birth coverage, not a
forecast of the child's entire lifetime or arbitrary later callbacks. Detected
unsupported candidates stop this enabled profile; finite inputs producing
nonfinite derived geometry also fail. `child_birth_frames` charges added boxes.
Native flags, RNG and collision remain unchanged; the option defaults off.

Optional `--scheduled-expiry 1` copies a one-based inclusive final collision update
for the currently observed bullet incarnation. Its narrow preview reads only the
first enabled DESPAWN reached through masked records under active NONE/WAIT and
unit clocks. It tests the active gate before skipping masked records. A blocked
WAIT at timer K permits execution at K+2; the Fired callback that changes state
still performs collision. Only later projected updates are omitted. With child
births enabled, an already validated fading child constructor can supply the same
parent cutoff. No future gameplay RNG or Native update is executed.
Unsupported previews keep the old full projection and charge
`unsupported_lifetimes`; other enabled operations stop this narrow preview without
claiming coverage. Positive cutoffs assume unchanged programs/callbacks/timing,
not a universal lifetime guarantee. They cannot certify a safe prefix or predict
slot reuse. `lifetime_records`, `lifetime_projected_frames` and
`lifetime_skipped_frames` charge hard/soft bullet projections before reach pruning.
The option defaults off and disabled metadata leaves actions/tie draws unchanged.

Optional `--retain-headings 1` distinguishes all nine incoming directions and focus
where the following layer must repeat that input, and at terminal endpoints whose
conditional target depends on it. Terminal retention is independent of tracing,
so exporting labels cannot change actions or tie draws. Half-pixel position
merging and bounded beam retention remain approximate. Enabled candidate
acceptance/tie draws can differ, while beam/horizon budgets stay literal.
`heading_layers` and `heading_keys` account for the additional membership work.
The full cyclic-domain logical bitset is nine MiB per buffer, up to eighteen MiB
for its two logical buffers, plus vector capacity/allocator overhead. Ordinary
bounded layer rectangles are smaller. This defaults off.

The experimental `--value-model` loads an immutable logistic endpoint ranker.
Its fixed contract is H24 proposals followed by holding their final input until
native update 64 or the explicit spell terminal. It predicts that conditional
branch outcome, not optimal escape value. Current/proposed features exclude seed,
spell identity, episode time and teacher future. Training-only normalization and
clipping are stored with the weights. The first model requires hard phantoms and
the motion envelope, without legacy-risk tables, lag, warm/held or terminal-tail
fallbacks. Unsupported combinations fail setup.
Only complete retained endpoints receive `-value_weight * log(max(p, 1e-4))` in
their terminal ranking. Earlier beam generation, pruning, budgets and tie draws
stay unchanged; equal terminal scores retain source order. A zero weight avoids
model evaluation. `value_evaluations` and nested `value_ms` account for preparation
and ranking; whole controller timing includes feature construction and inference.
No probability is a safety certificate or a reason to certify a failed rollout.

`th08_value_collect` is a separate offline tool. At bounded checkpoints it obtains
up to four owned complete prefixes from the ordinary beam and independently forks
the current native world for each one. Shooting/focus/movement, the original latch,
RNG, carried bullets and actors remain literal. Each child executes that prefix
and then holds its final input; it never feeds future state to the parent policy.
Native death is a negative label, a full surviving window or declared native
terminal is positive, and faults/incomplete windows cannot enter training.
Parent tapes, checkpoint/prefix identities, explicit branch inputs and dataset
hashes remain local. The first frozen producer predates the explicit sidecar and
retains its exact binary/source plus parent tape and proposal rank for recovery.
Its startup-terminal bug was rejected by transparency checks before training data
generation; the original failed attempts stay charged.
Optional `--continuation replan-h24` creates a different, explicitly versioned
conditional target. The fork inherits the already-decided parent Policy, submits
the 23 proposed controls and repeats the last on native call 24. It warms the 23
intermediate observations without search/tie draws, then resumes the inherited
no-model H24 baseline after update 24 through update 64 or terminal. This is a
baseline-policy continuation, not an optimal expert. V1 held-last data/model bytes
stay supported; mixed train/tune targets fail. Actual branch submissions and
Native attempts/time plus child observation/decision work are recorded separately.
`--sample-from`/`--sample-until` define an inclusive window and anchor its stride;
defaults preserve the original absolute grid and update-128 floor. Sampling has
its own identity; parent identity, split/weighting and duplicate rejection remain
whole-trajectory properties. Additional failure-tail strata cannot silently become
independent parents, overlap labels or replace a failed original episode.
`th08_value_train` rejects overlapping train/tune seeds and parent identities,
fits normalization and weights from training rows only, and reports conditional
classification metrics. Seed/parent splits, all parent outcomes and Native label
costs belong to the frozen campaign manifest; selected small weights and provenance
are saved in [models](../models/README.md). Independent paired native completion
and complete-stage tests remain separate from classification loss.

`th08_online` computes searches synchronously. Controller latency is the actual
whole decision; modeled executor latency subtracts only the full planner search,
retaining capture, forecast/index, checks, cancellation and emergency cost.
Planner overruns compare that search with N times the requested frame deadline;
they are diagnostics and do not postpone fixed-N adoption. Lag-zero executor
latency equals controller latency and has no separate delayed planner counters.
Per-update logs and reports include both timing scopes and executor events.
These are not asynchronous CPU-contention, snapshot-copy or live bridge tests.
Exact kernel batching, parallel probes and lazy forecast work require their
own ordered differential checks and measured adoption decisions. Behavior-changing
options use frozen paired seed banks; no significant difference is not evidence
of equivalence, and spell-practice outcomes do not establish story probabilities.
Broader learning, native-lookahead alternatives and the retail bridge remain
separate work under their documented information and execution boundaries.
Full configuration and executable/DAT/action/prefix/model hashes accompany the
result. Source archives, model provenance, process receipts and failed tapes remain
in ignored local experiments. Success requires fresh complete action execution,
terminal/RNG/calculation agreement and natural replay with zero death/bomb rows;
this does not independently certify native physics or retail compatibility.

The separate opt-in `th08_online_live` instead advances one recording API update
on each absolute steady-clock tick `epoch + floor(update * 1e9 / 60) ns`. The
Native owner captures current observations and applies route/dialogue controls;
one worker exclusively owns a lag-zero Policy. All snapshots are owned copies.
Only a reply with the current update/phase and publication time no later than the
tick cutoff can be used. Missing/late replies cause an explicit last-movement/
focus hold with shooting; this fallback is not certified safe. Every expired
queued observation still warms history without retroactive search/tie draws.
The bounded FIFO fails visibly on overflow; stale replies and abandoned/failed
work remain counted. Stop invalidates queued work and joins the bounded running
search rather than force-killing it. Worker floating-point state copies the owner's
environment. Lag, warm/held and tail modes are initially rejected.
Native execution never waits for the current search. The owner aborts if processing
crosses the next period plus the declared `--scheduler-tolerance-us`; wake-up jitter
is measured separately. It never resets the epoch or bursts multiple updates to
hide elapsed time. Native file I/O is part of this real owner deadline, with timing
also reported separately. Recording/control handoffs remain API ticks; replay
fast-forward calculation rows use their established different units. Setup and
worker shutdown cost are separate from the live loop. Actual completion still
requires full fresh execution and natural replay; a paced fragment is not a whole
stage/story result or proof of retail control.

Teacher corrections are separate offline episodes from immutable student prefixes.
The first diagnosis protocol queries all six remaining failed 4B/Extra trajectories
at T-128. A later frozen phase selects every unverified correction and quadruples
the prefix lead to T-512, retaining the earlier failures and costs. It uses the
same R64/beam384/cell16/commit8/checkpoint120/keep64/attempts4/CEGAR4 planner, with
100000 total updates and 900 seconds per episode, including reconstruction.
Fresh actions, natural replay and byte-identical parent prefixes gate labels.
An earlier-prefix success labels that earlier state; it does not certify a later
state after the trajectories have diverged. Do not count teacher repairs as online
clears or use them as held-out evaluation.
The native teacher's `--shoot 1` still permits bounded quiet-window proposals;
the current student's action always includes shooting. All six corrected suffixes
exercise this mismatch. An action/value label derived under the teacher's larger
action set need not be attainable by the student. Keep the full shooting/focus
inputs and output-space metadata, then either add a learned low-frequency shooting
decision or obtain appropriately constrained teacher labels. Never translate quiet
input into an unrecorded RNG change.

`th08_plan --shoot 1 --quiet-retries 0` constrains every newly generated gameplay
input to include shooting: it disables the local quiet fallback, checkpoint quiet
windows and short retry delays. Movement/focus retries, budgets and native
verification remain active. The default is `--quiet-retries 1`, retaining the
existing planner. The option is reported and survives sanitized report collection.
Supplied prefixes and route-choice control are executed literally; a constrained
continuation cannot retroactively alter a quiet prefix or certify full information
matching with the student. Teacher lookahead remains privileged offline evidence.
Constrained Extra diagnosis also exposes proposal coverage: a recorded student
path survives the first R64 window under the same native build, while the finite
teacher search exhausts without a commit. Preserving a known proposal or warming
the search from student behavior is future generic teacher work. Any such path
must receive its own native rollout; a recorded future cannot certify another
action branch or become an embedded online route. Diagnosis seeds remain tuning
data, and every algorithm/configuration change must pass one frozen stage/seed
evaluation covering all nine entries before releasing the untouched holdouts.

## True-RNG shadow runtime

Issue #8 authorizes a separate native-lookahead profile. A synchronized shadow
follows actual owner inputs and uses complete branch-owned Native state/RNG
for candidate prediction. The real owner cannot rewind; future planning must not
change its executed or latched prefix. This does not relax the zero-native-future
contract of the existing current-memory policy.

The recorded shot1 Lunatic4A→FinalB story and proper Extra already complete
online; their exact profiles and limits are in Status. Stage4B and new story
experiments are paused while Extra scoring is the active research scope. Earlier
practice/shot0 successes remain component evidence: practice initialization resets
state, so isolated clears alone do not certify carried world/RNG at story joins.

The first `th08_shadow_follow` gate implements common-start Native input following
over a versioned wire projection. It compares independently initialized processes,
detects the first differing field and preserves original stage handoffs. It does
not import arbitrary retail state, predict futures, generate actions or enforce
deadlines. Its finite scalar/digest agreement is not full-world or retail proof.
`shadow::predict` now verifies an explicit action sequence from the actual current
world/RNG in a branch-owned process and filesystem. A complete horizon or actual
terminal can certify a plan; death, fault and timeout cannot certify even an early
surviving prefix. `ActionBuffer` checks late results against every executed input
and projected state, replacing only the remaining suffix. The complete ID179
regression verifies 1292 updates, rejects a fatal candidate and executes the
verified queue without divergence. The search/client built on this primitive has
complete live clears; the primitive test itself is not a deadline result. Live
score conversion, broad seed reliability and every-frame60Hz remain open in
[Status](STATUS.md). The synchronous250ms scoring harness is a separate profile.

`th08_shadow_online` follows a local Wine observation journal and publishes plans
through a versioned mapped file. The default `h32` profile fixes lead 8, verified horizon 32,
minimum search interval 4 and replan-remaining threshold 28. A proposal child
advances the immutable lead first, warming inherited Policy history at each update.
The H24/beam192/cell16 policy exports at most four 23-input prefixes; its first
modeled movement belongs to the existing latch. The client pads the remaining
update and also considers the 18 held movement/focus candidates. Every complete
lead-plus-candidate executes with its own full Native world and true RNG. No sampled
RNG replacement or shared action-independent future is used.

Proposal generation has a 60-ms wall cap inside an 80-ms total search budget.
Fork/setup/termination can exceed those caps; reports retain actual wall overruns,
partial counters and full requested update charges for interrupted native work.
The lead is a latency allowance, not a claim that planning always meets one
16.67-ms period. Receipt catch-up, exact prefix comparison and owner-side stale/lead
checks protect the irreversible frontier. A depleted queue holds actual movement
and focus while shooting, with alternating dialogue skip outside spell practice.
Fallback is an explicit executed input, never a certified future or a rescue proof.

The opt-in `h64` experiment changes a bundle: verified horizon 64, minimum search
interval 8, replan-remaining threshold 56 and total search budget 110 ms. Lead 8,
the 60-ms proposal cap and the H24 proposal policy remain fixed. Each candidate's
fixed prefix is followed by Policy decisions over the branch's actual future
observations, submitting at most eight inputs before deciding again. Guidance
lives entirely in the forked child; inherited Policy history/tie RNG and the
parent's actual world remain private. Complete native execution still decides
the 64-update horizon or actual terminal. Continuation wall time and requested
inputs are reported separately from native simulation and file I/O.

The named `h64-held` experiment adds native held-action recovery. On fresh
searches with an existing plan, the 18 movement/focus prefixes precede Policy
prefixes and their final action continues through the full horizon. If none
certifies, Policy-guided tails use the remaining total budget. Startup keeps the
Policy-first order. A held tail preserves the final shoot bit as well as movement
and focus; owned dialogue state may still select the recorded Z cadence. Every
attempt is charged, including incomplete and fatal branches. `h128-held` applies
that search at horizon128, replan-remaining120, with the same lead8, interval8,
110ms total and60ms proposal cap. The bridge holds all128 inputs; greater rollout
work and altered selection order are explicit experiments. `h256-held` verifies
256 updates while publishing only the first128. The full successful certificate
is retained and validated; a fatal or incomplete256-update future supplies no
prefix. Nominal extension may reuse the unconsumed unpublished future as a proposal,
then freshly verifies a new256-update future. Only the queued published inputs
are declared unchanged to the owner. Its publication threshold remains120 and
its budget remains110ms. Reports separate verified and publication horizons.
If a new search fails, the remaining successful certificate can extend the
published window: the actual executed prefix must still match, all queued inputs
remain unchanged, and the complete original certificate must remain valid. The
wire window rebases to the current actual frontier; the Native certificate keeps
its original origin and full cost counters. Failed futures never supply this
extension. Retained-window attempts, accepted windows and newly queued inputs
are separate counters; reusing a proof does not charge another Native rollout.
This prevents a128-input transport limit from discarding the safe latter half
of a256-update forecast. Existing `h32` and
`h64` keep their prior behavior.

The separately named `h256-pulse` recovery tests a rotating family of66 native
motion proposals before paying for heuristic generation: both focus states,
nine held directions and16/32/64-update moves followed by a stationary tail for
each nonstationary direction. A persistent attempt cursor advances across actual
search origins so the110ms cap cannot repeatedly starve later directions. Native
verification includes the unchanged eight-input lead and the complete256-update
future; only128 inputs publish. Nominal reuse still runs first. Policy proposals
use only any remaining total budget. Direct/pulse attempt/selection counts and
the final cursor are reported separately. This changes candidate families and
ordering explicitly; it does not change collision, game RNG or old profiles.

`h512-buffered` is a separately named scheduling experiment using the original
H24 guided proposals:512 verified updates,128 published inputs,32 unchanged lead
inputs,480ms total,60ms proposal cap and minimum interval8. Its512-entry actual
history supports later certificate windows. The longer input lead accommodates
slower Native search while the original owner keeps running; it is not a claim
of lower computation time or16.67ms decisions. Once per publication window
(128 actual receipts), a fresh solve takes priority over nominal reuse. A failed
refresh consumes its interval too, allowing ordinary reuse to resume. Reports
separate forced refreshes, fresh adoptions, original Native costs and retained
windows. No stage/spell/seed selects this policy; older named profiles retain
their original budgets, lead and reuse behavior.

`h128-recorded` adds a separate Native-geometry proposal path:128 verified and
published updates, lead8, minimum interval8 and110ms total. That budget includes
literal nominal verification, recording, proposal search and independent final
verification. A nominal tail repeats its final held action instead of returning
to the H24 current-memory Policy. Owned GUI state still advances dialogue after
the forced lead. A failed nominal origin is not repeatedly retried while its
existing certificate runs down; fresh proposals receive the remaining budget.

The recorder forks the actual Native world, including action-dependent RNG and
lifecycle state. It observes bullet boxes, pooled/direct laser calls and lethal
regions at their collision phase, stopping at death, terminal or unsupported
behavior. These are conditional samples, not hazards guaranteed for another path.
The float32 proposal beam uses width192, cell16 and the original one-update latch.
It considers the default spatial alternatives, then fixed field goals spanning
x48/192/336 and y96/240/400. No stage, spell or seed chooses a goal. Additional
goals have independent proposal RNG streams and preserve the primary stream;
unused groups are never computed. Up to four counterexample recordings refine
the proposal constraints, within the same total budget. Only a complete independent
Native verification can publish inputs. Failed recordings cannot certify prefixes.

The planner owns reusable bounded geometry storage; it retains no mutable Native
checkpoint process. Recorder work is charged under proposal attempts/updates,
separately from all verification attempts. Reports also expose recording/search
counts, capture/search time and timeouts; Native timing excludes file I/O. The
ordinary profiles keep their existing algorithms and budgets.

`h64-recorded` keeps the original H64 nominal Policy path and its64-input
publication window. Revision3 requires a complete128-update Native verification
for a recorded repair. Subsequent literal extensions preserve that longer route
and verify128 again; publishing64 does not discard the unpublished evidence.
Lead8, interval8 and the shared110ms total remain explicit. Unused budget may
still reach ordinary fresh Policy proposals. The earlier fixed64-verification
portfolio remains a retained failed control, not a128-update safety claim.

Before requesting fresh candidates, `h64` and its named variants can reuse the unconsumed inputs of the
previous verified plan. A fresh child re-executes that whole prefix and extends
its tail to a new complete horizon. This saves repeated proposal searches; it
does not reuse old collision verdicts for a changed world. The declared first
change follows that entire unchanged prefix, so owner-side checks can accept the
extension while keeping queued inputs immutable. A failed or incomplete extension
cannot certify any prefix. Fresh alternatives still fit inside the same total
search budget. This is a distinct experimental profile, not an isolated horizon
ablation or an automatic change to the default. Reports retain continuation costs,
timeouts, deaths, queue gaps and invalidations. A real terminal reached before
any changed input is needed is counted separately from a deadline-stale plan.

An unsuccessful nominal extension disables reuse of that plan's origin, even
while actual execution advances through its still-verified queue. Subsequent
searches give fresh candidates the complete existing search budget. Adoption of
a certificate with a new origin enables nominal extension again. This avoids
repeatedly spending the budget on an unsuccessful fixed prefix; it changes
scheduling, not the budget, collision handling or certification contract.

This bounded search can enter a state with no surviving tested continuation.
Native validation establishes its declared finite window only. Failed earlier
profiles and the optional exact-mode story diagnostics remain evidence; a
complete spell does not establish a whole-stage NMNB or deadline result.
The owner explicitly prioritizes meaningful gameplay outcomes over tiny numeric
parity. The live behavioral comparison logs digest/subpixel drift and stale
inactive labels rather than rejecting them; it retains gameplay/RNG/lifecycle
checks. Exact comparison remains selectable for diagnosis, not a clear prerequisite.

The opt-in `TH08_SHADOW_ADVANCE_DIALOGUE=1` profile advances dialogue with four
updates of released Z followed by four of pressed Z, using the global receipt
sequence (including handoffs). It preserves movement/focus and alternating Ctrl.
Only the owned Native GUI's dialogue-present state selects that behavior in
proposal generation and guided tails; immutable prefixes retain their actual
inputs. Complete Native rollouts still decide whether a plan can be adopted.
The retail input hook executes those same queued inputs without separate key
injection. Spell practice rejects the flag, and this profile requires a verified horizon
of at least64.

`TH08_SHADOW_PRIOR_FINAL_A_CLEAR=1` names a second-clear story checkpoint for
double teams at standard difficulties. Both worlds load a fresh score history,
then declare one prior Normal Final A clear in the original with/without-retry
tables and recorder clearState=1. Retail waits for its asynchronous gameplay
setup before applying the declaration, before the first calculation receipt.
The original Stage 5 message selects Final B naturally; no stage jump, gameplay
RNG substitution or route-selection input policy is added. The V2 `start.txt`
records both flags; V1 files keep their original first-clear/input behavior.

### Viewing actual Wine gameplay

The owner keeps the existing Wine deployment, accepts the separate-capture Stage
4A preview, and authorizes complete Final B practice followed by actual full-story
recording through 4A to Final B with real audio. Issue #9 owns
the execution/resource brief and future Windows alternatives. Capture the rendered
owned game while `th08_shadow_online` is making decisions; rendering an exported
action tape later produces replay footage. Keep the solver profile, actual RNG
capture and original game clock unchanged, and retain failed or stopped attempts.

Record the game rectangle on its owned Xvfb root display. Wine can destroy and
recreate the game window during startup, invalidating a recorder's window ID even
when its screen position stays fixed. Read the rectangle from the owned window,
inspect an early gameplay sample and ensure it stays inside that rectangle. A
dedicated PulseAudio server with only a null sink supplies the game's output and
its monitor source. Route the owned Wine prefix and recorder to that private Unix
socket; do not record unrelated host audio. Enable WAV music, sound effects and
their volumes only in the owned recording configuration copy, and record its hash.

Deliver the standalone full Final B video by stream-copying the capture video
when muxing aligned audio. Deliver the full story and a selected Kaguya-spell
preview with balanced compression; the accepted 4A derivative uses native 640x480,
libx264 fast CRF26 and AAC128k, preserving capture cadence without interpolation.
Keep originals and all videos outside Git. Only the owner's selected matching
4A/Final-B/story replays are an exception to the generated-artifact ignore rule.

Capture video and audio in separate processes. The original combined X11/Pulse
command stalls even on a blank owned display without Wine or a solver: a 12-second
control yields 281 frames, 222 repeated timestamps and a 1450-ms gap. Video-only
and simultaneous separate audio/video capture each yield 720 frames, no repeated
timestamps and a 17-ms maximum gap. These synthetic controls isolate a recorder
fault; they do not establish smooth retail rendering or gameplay deadlines.

Use a single libx264 encoder thread and a fast preset, with lossless PCM audio
captured independently. For example, after initializing the owned rectangle and
private PulseAudio endpoint:

```sh
ffmpeg -f pulse -server "$PULSE_SERVER" -sample_rate 48000 -channels 2 \
  -i th08_game.monitor -c:a pcm_s16le reports/local/audio.wav &
audio_capture_pid=$!
ffmpeg -f x11grab -framerate 60 -video_size 640x480 \
  -i "$DISPLAY+$capture_x,$capture_y" -an \
  -c:v libx264 -preset ultrafast -crf 21 -threads:v 1 -pix_fmt yuv420p \
  -f matroska reports/local/video.mkv &
video_capture_pid=$!
```

Start before gameplay, retain actual timestamps, and stop both owned recorders
cleanly when the observer reaches its declared terminal. The current observer
exits at gameplay completion, so this does not capture ending credits. Matroska
preserves an interrupted attempt. Retain each device's logged original clock
origin. If audio starts earlier, trim that many 48-kHz PCM samples; if it starts
later, insert the measured leading silence. Encode the aligned PCM as AAC and
stream-copy the video into a seekable MP4, preserving actual gameplay time and
sequence. Do not recover missing temporal samples by changing game speed or
declaring a nominal output frame rate. Verify decoding, both media streams and
non-silent audio separately from actual journal/NMNB and fresh Native-follow checks.
Separately normalized timestamps can both start at zero while hiding the different
capture start times. Inspect packet intervals and duplicate timestamps, not just
average FPS or successful decode; bursty capture can pass both superficial checks.
Capture sampling FPS is not proof of 60-Hz gameplay. Record encoder overhead,
identities, actual seed, fallback/queue counters and outcome; per-job maximum RSS
is not a simultaneous combined memory peak. Keep all generated movies, previews,
audio packages, action tapes and experiment reports under ignored `reports/local/`.

## Resource ownership and parallelism

Independent executables can run in parallel with separate scratch directories and
result files. Within a fork planner, rollout slots and process verdicts must also be
independent before concurrent children are enabled. The imported DAT entry reader
closes/reopens its handle for each entry; audit other inherited descriptors rather
than assuming all reads share or avoid offsets. Current fork rollouts remain serial.

A measured spell-92 run spent about one quarter of search CPU time in collision
queries; beam expansion/selection also mattered. That sample does not justify adding
threads to the collision kernel. Independent complete solver jobs currently provide
useful CPU parallelism while retaining separate native worlds and evidence.

On shared hosts, bound jobs and process count using measured memory. Use low CPU/I/O
weights, a hard memory limit and a lower high-water limit, with no additional swap;
check available memory before launch. CPU parallelism does not justify risking OOM
or changing unrelated workloads. Keep machine addresses, usernames, private paths
and raw logs out of public artifacts. Record technical profiles and sanitized results.
The active Extra campaign permits at most four concurrent solver jobs and320
cumulative campaign tasks; compilation uses two jobs. The handoff ledger is
recorded in Status. Do not increase concurrency because the shared host is idle.
The following unit sizes are historical campaign profiles, not current launch
instructions. The earlier offline campaign retained four simultaneous solver jobs, with320 tasks,
four CPU cores and 12 GiB memory for the entire batch. The online baseline uses a
separate low-priority units with at most four independent controllers in total,
96 tasks per unit,
6 CPU cores, 6 GiB memory high-water and 8 GiB hard limit. Its historical six-job
compile setting is superseded by the two-job limit above.
When ablations overlap, reduce the older queue and reserve a solver slot before
starting another. The combined task limits remain at most 320; holdout work is gated.
Student-state teacher corrections use two planners at most, a 192-task/four-core
unit with 8-GiB high/16-GiB hard memory bounds; the serial successor uses 96 tasks,
two cores and 6-GiB high/8-GiB hard bounds. Both retain low weights and zero swap.
These are task-specific limits, not changes to shared user limits. Check process limits as well
as memory before launch: each fork checkpoint holder consumes a process. The
[resource revision](../reports/native/planner_route_protocol.json) retains the
initial oversubscribed launch instead of concealing its fork failures.

## Acceptance and experiments

The [full-run matrix](../reports/native/planner_route_matrix.json) enumerates 108
valid combinations. Stage 4 follows the source's character partition; it is not an
independent cross-product axis. Final A/B selection is a recorded dialogue
controller outside beam search, with original preexisting-clear history recorded
as replay `clearState` 2. Rollouts store their actual commands and commits consume
those same commands. Prefix reconstruction and fresh action verification never
replace inputs. Full Extra starts with `--story 1 --stage extra --difficulty 4`.

The [route protocol](../reports/native/planner_route_protocol.json) freezes two
generic profiles, per-episode budgets and a single eligible budget-prefix
continuation. Run the second profile only if the first has no verified completion;
retain all original failures and costs. The owner switched to random evaluation
after 36 verified seed-0 combinations, stopping the remaining baseline queue.
Random evaluation uses many fresh OS-entropy seeds with a small
predeclared selection of complete runs per seed, rotating team/solo shot, difficulty,
Final A/B and Extra. Freeze the exact seeds, assignments, sample size, algorithm and
budgets before evaluation. Report distinct seeds and planned/completed seed-case
pairs separately, including all successes, failures and retry costs. This sampling
scope supersedes the frozen protocol's original per-seed complete-matrix wording
and its requirement to finish all 108 baseline entries first.

The first batch fixes 24 distinct, previously unobserved seeds and one run per seed.
Diagnosis covers each of the 12 shots once, alternating Lunatic and Extra by shot
parity. Holdout covers the same shots with the opposite difficulty assignments;
Lunatic Final A/B alternates across shot pairs. This yields 12 diagnosis and 12
holdout cases, with 12 Lunatic and 12 Extra runs overall. Execute diagnosis first.
The phase gate requires all 12 diagnosis pairs to verify under the frozen native
executables and options. It records executable, runner, report and protocol hashes
before releasing holdouts with the same resource limits. Any incomplete or invalid
diagnosis result stops promotion for review. Retain failed holdout outcomes if
later debugging turns those seeds into training evidence.
For a failing solo shot without an existing baseline, compare the same configuration
at seed 0 as a separately charged diagnostic control before attributing the failure
to RNG. Preserve original failures and rerun costs after a generic fix.

For baseline matrices, use `th08_planner_report --matrix` to require each attempt's
checkpoint and each successful native/replay stage path to match the manifest. Its coverage denominator
includes every catalog entry, and repeated attempts cannot inflate the distinct
verified count. Missing, budget-limited and runtime-failed cases remain unverified.
For sparse multi-seed manifests, use `--sample` instead. Its coverage unit is an
explicit seed-case pair; unique-seed counts and repeated attempts remain separate.
Both modes require exact native/replay stage paths and successful process receipts.
Keep new replays, actions, generated ledgers and receipt JSON in ignored
`reports/local/`; existing published snapshots remain historical evidence.

Before publishing a checkpoint:

1. Run focused contract regressions and the normal CTest suite, including docs.
2. For native changes, run affected real-data cases and the complete pinned replay
   fingerprint corpus. Require every expected fixture, matching input hash/row count/
   fingerprint, successful child exit and natural replay completion.
3. Run complete scenarios and freshly replay the resulting action tapes. Stage/story
   results also need a `.rpy` round trip with matching terminal score and explicit
   zero death/bomb-input rows.
4. Record seed, difficulty, team, route, compiler/flags, upstream revision, source and
   executable hashes, outcome, native-update/search budgets and wall time. Separate
   file I/O and cross-build/environment comparisons from simulation measurements.

Training seeds are 0, 1, 42 and 65535. Holdouts 7 and 2026 remain untouched until a
candidate is frozen. The R64/beam384/cell16/shoot1 spell-92 candidate was frozen after
all four training seeds passed, with checkpoint60/keep12/180-second budgets; both
holdouts then completed and freshly replayed without tuning. This evaluates that
checkpoint/profile, not a continuous story or all-seed robustness.

The subsequent [story-seed protocol](../reports/native/planner_seed_protocol.json)
draws eight uint16 seeds once from OS entropy before inspecting outcomes. Training
seeds 4455/12525/30504/31338 support diagnosis; 24196/35540/1763/2138 are reserved
for the frozen candidate's holdout evaluation. Both existing seed-zero story
profiles, R64/beam384/cell16 and cell32, are frozen before these cases. Each attempt
keeps its 250000-update/2400-second budget and all native retry costs; running two
profiles costs two attempts, not one. Any tuned successor must be frozen before
using holdout feedback. Require full-story fresh action and natural replay proofs;
a finite seed sample cannot certify all 65536 initial seeds.

Training-only progress also motivated one frozen 900-second continuation for each
original `budget` result below the total frame cap. This is a separate episode with
the same profile and exact parent prefix, not an extension of the original budget.
It cannot revise earlier actions. Keep direct/continued results separate and charge
all original attempts, prefix reconstruction and new verification.

The later learning direction is recorded in
[issue #3](https://github.com/N0zoM1z0/th08-solver/issues/3). Current work establishes
the offline teacher's evidence: complete successful tapes and replay hashes,
configuration, terminal checks, costs and distinctly labeled failed prefixes.
Future imitation and student-state teacher corrections need predecision observations
aligned with the native input latch. The teacher may use future rollouts but must
keep the student's queried prefix immutable. The current online policy is described
above. Recorded teacher corrections and compact conditional value models now
provide initial evidence; broad imitation, learned reactive policy replacement
and RL remain future work. Successful and failed episodes stay distinct.

Measured execution acceleration is tracked in
[issue #4](https://github.com/N0zoM1z0/th08-solver/issues/4). Its
[frozen workloads](../reports/native/runtime_perf_protocol.json) use complete
archived stories and an incomplete prefix, with paired Release timings and
unchanged replay/collision/terminal checks. Runtime and search costs remain separate;
headless execution already removes display/audio initialization and frame waiting.

The [execution audit](../reports/native/runtime_perf_summary.json) adopts the
callback-local empty cancel-region proof described in [Architecture](ARCHITECTURE.md).
On the frozen three-story workload, native update medians fall from 11.27–11.39
seconds to 7.84–8.03 seconds: 1.42–1.45x baseline throughput, with no discarded samples.
An unchanged-source IPO experiment separately saves about 9%; it is not enabled by
default, and the two gains are not combined. One additional complete prefix-resumed
planning pair falls from 40.30 to 33.58 seconds with identical actions and deterministic
search counts. That diagnostic includes prefix reconstruction and is not a general
whole-solver speed guarantee. Original episode budgets/failures and additional
profiling, execution, planning and capture costs stay distinct.

The reviewed handoff records Lunatic practice Stages 1–3 and a
story prefix blocked at spell 92; local/integrated reproductions must be labeled
separately. A practice clear, partial story or sampled seed success is not a full run.
