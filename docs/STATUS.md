# Current handoff status

Reviewed: 2026-10-22. Current work is
[issue #5](https://github.com/N0zoM1z0/th08-solver/issues/5): integrate and validate a
forward-only CPU controller using authorized current process memory, then diagnose
generic failures and build the offline-teacher/student loop. The earlier issue #1
work integrated the fork planner and retained its replay fidelity limitations.
The tracked runtime imports upstream `02ff72f`; results below from
`861bec9` remain historical native-profile evidence. They do not certify the new build.

## Active work

The current approach is in [Planner](PLANNER.md). The generic planner uses native
fork rollouts and verified commits; it adds no spell-92 strategy branch. The
[campaign ledger](../reports/native/planner_current_summary.json) retains successful
and failed searches, fresh verification, budgets, executable identities and total
native costs. Unfinished processes contribute only a pending identifier.

The latest scope prioritizes random-seed generalization. The reference catalog
covers 108 full runs: twelve team/solo shots, each with Final A/B at
Easy/Normal/Hard/Lunatic and a full Extra entry. The
[matrix](../reports/native/planner_route_matrix.json) and
[protocol](../reports/native/planner_route_protocol.json) fix coverage and generic
budgets before outcomes. Final routes use original dialogue inputs with an explicit
recorded unlock-history checkpoint; Extra uses non-practice initialization. The
owner stopped the remaining baseline work to start random evaluation immediately.
The owner has now moved implementation to the online controller; enumerating an
offline case does not establish completion.

The first maintained `th08_online` separates owned observations, policy, native
adapter and reporting. It performs no future native updates or rewinds and ends
on death. The corrected TSV parser preserves all four backoff horizons; prefix
observations warm motion history without choosing actions. Focused regressions
cover these contracts, the input latch, policy-instance isolation and reach pruning.
A complete Lunatic Stage 2 practice run (shot 0, seed 0, H32/beam192, reach filter,
no learned model) completes in 32993 updates. Fresh action execution agrees on
terminal, score 11880554, RNG and calculation count; natural replay has 32993 rows
and zero death/bomb-input rows. On the local AMD EPYC 7B12/GCC 12.2 Release profile,
controller mean/p99/p99.9/max are 3.24/4.79/5.47/7.51 ms, with zero 16.67-ms misses.
This concurrent test sample is not an isolated benchmark or a general deadline
guarantee. The harness waits for decisions (`deadline_enforced:false`). Learned-risk
Stage 5 also completes and freshly verifies at 44136 updates, score 23100049,
with matching natural replay rows and zero death/bomb rows. On the remote
EPYC 9654/GCC 13.3 Release profile, mean/p99/max are 4.61/7.24/8.78 ms, zero misses.
The no-model Stage 4B control dies at update 30593 in spell 88; fresh execution
matches score/RNG and identifies a bullet first observed at 30592. At that point
all 12 sampled held-input variants die because movement is already latched.
At checkpoint 30591 one sampled variant survives a 32-update post-event window.
This motivates earlier spawn/positioning knowledge; it is not a full-stage rescue.

Stage 6B's no-model candidate clears at 59927 updates and matches fresh actions,
but natural replay ends at score 129080308 instead of 95330308. It remains an
unverified candidate; the score gate is not relaxed. The same sample has 41
16.67-ms misses and a 19.75-ms maximum. Static retail inspection and a separately
checked supplied Stage 6A tape reproduce the final-practice clear-bonus difference;
clear-tail input also diverges. [Validation](VALIDATION.md) owns that evidence and
its limits. These candidates remain validation failures. All 40 native/DAT CTests
pass, including complete-stage and policy contracts; documentation checks also pass.

The latest scope requires all nine standalone entries over different seeds before
full story. A frozen uniform learned-risk diagnosis batch uses shot 0 and Lunatic
(Extra difficulty for Extra): seed-zero controls plus two fresh random seeds per
entry, 27 pairs total. The existing Stage 5 control is reused with its original
receipts and costs. The batch is complete: 14 strict verified completions, seven
deaths and six replay-validation failures. All 20 native completions agree with
fresh action execution. The same candidate and 100000-update/1800-second budgets
apply to every pair:

| Standalone entry | Strict verified / attempted | Remaining failure |
|---|---|---|
| 1, 2, 3, 5 | 3/3 each | None in this diagnosis sample |
| 4A | 2/3 | Death in spell 76 |
| 4B | 0/3 | Death in spell 88 |
| 6A, 6B | 0/3 each | Native complete; natural replay score equality fails |
| Extra | 0/3 | Death in spell 199 |

This is 19 distinct diagnosis seeds (seed zero is shared across entries), not 27
unseen seeds. The candidate consumes 1139521 native updates and 5048.74 summed
solver wall seconds, counting the reused control once; fresh verification and
replay work are additional retained costs. It has 103 measured deadline misses.
No strict real-time deadline is enforced.

A separate optional 16-pixel actor-origin soft-risk ablation repeats all nine
4A/4B/Extra diagnosis pairs under the same H32/beam192/model/budgets. Extra advances
from spell 199 to 201 on all three seeds but still dies. Stage 4A stays 2/3, fixing
seed zero while regressing another seed; Stage 4B stays 0/3. Fresh collision traces
show the learned-risk Stage 4B seed-zero death is a known pooled-laser trap with
decreasing search depth, unlike the no-model new-bullet death above. Two bounded
teacher rescues from T-2/T-8 both exhaust, costing 63153 native updates including
prefix reconstruction; these failures are not impossibility proofs.

A predeclared H64/spatial-retention comparison and a partial current-program
shot-prefix comparison continue on diagnosis cases; no winning seeds are spliced
into a universal profile. Eighteen additional distinct holdout seeds, two per
entry, remain unexecuted until a frozen candidate passes diagnosis. Overlapping
queues reserve slots to keep at most four controllers and 320 tasks overall;
each low-priority unit has 96 tasks, six CPU cores, 6-GiB high/8-GiB hard memory
limits and no extra swap. No continuous online NMNB story or retail live controller
is established. New tapes, replays, models, JSON reports and process receipts stay
under ignored local storage.

The route interfaces pass all 38 native/DAT CTests; all 108 replay-corpus outcome,
row-count and fingerprint tuples match the preceding runtime (106/108 against the
retained reference). The old seed-0 full story freshly replays with unchanged
terminal, score, RNG and calculation events. The first broad launch hit the shared
user's 4096-process limit without OOM and was stopped. The
[retired ledger](../reports/native/planner_route_initial_summary.json) retains 50
finalized runtime failures and their exact failed prefixes. Its 58 `pending`
records were interrupted without final process receipts; they are no longer running.
The [environment audit](../reports/native/planner_route_environment.json) distinguishes
their unavailable native counters from measured scope CPU. The replacement queue limits the whole campaign to four
concurrent solvers, 320 tasks, four CPU cores and 12 GiB RAM at low priority.

Storage cleanup reclaims 1084080128 allocated bytes from completed campaign
directories: 70 raw captures are losslessly compressed with restored SHA checks,
and 1428 old compile intermediates are removed. All 1131 protected replay/action
files retain their hashes. Active build/data and failure evidence remain available;
later successful raw rows are removed after recording their hashes and receipts.

The stopped local baseline ledger verifies 36 directly completed combinations:
all nine seed-0 entries for each of the four double teams. Each team clears all four
standard difficulties, both final routes and full Extra. Full tapes and natural
replays agree on all expected stages, scores and 7837191 calculation rows with zero
death/bomb input rows. All finish in the original c16 episode, retaining internal
backtracking/retry costs. Four solo attempts were stopped by the owner without final
process reports or action tapes; their four `pending` ledger records are interrupted,
not live jobs or algorithm failures. Native counters for this interrupted work are
unavailable; retained progress and the last scope CPU sample are lower-bound evidence.
The other 68 catalog entries were not started. The old queue is inactive.

Following the owner's artifact policy, new replays, actions and generated JSON stay
under ignored `reports/local/route-matrix-v2/`, including the verified archive,
`planner_route_summary.json` and `planner_route_environment.json`. The retained
[published ledger](../reports/native/planner_route_summary.json) and
[audit](../reports/native/planner_route_environment.json) remain a 24-combination
snapshot. The C++ report's optional `--matrix` mode checks each attempt's checkpoint
and successful native/replay stage path, binds the manifest hash, and counts unique
combinations while retaining attempt costs. The baseline's four pending records
carry the owner-interruption meaning described above. Historical ledgers still
reproduce byte-for-byte without matrix mode.

Random evaluation uses the same four-solver/320-task/four-core/12-GiB
limits and unchanged native executables. The frozen batch draws 24 distinct,
previously unseen OS-entropy seeds: one complete run each, split into 12 diagnosis
and 12 held-out cases. Each split covers all 12 shots once; Lunatic/Extra assignments
swap between splits, and Lunatic A/B routes balance over the batch. All 12 diagnosis
cases pass. The checked phase gate recorded the evaluated executable, runner,
report and protocol hashes, then released holdouts at 08:59 UTC. The native
executables and budgets are unchanged between phases. Holdout evaluation completed
at 10:34 UTC; the checked phase gate reports both phases verified.
Manifests, protocol, results and failure evidence stay under ignored
`reports/local/seed-generalization-v2/`. The new `--sample` report mode binds explicit
seed-case pairs and counts distinct seeds separately, retaining each attempt's costs.
The collected final snapshot verifies all 12 diagnosis and all 12 holdout pairs:
24 distinct new seeds, all completing in the original c16 episode. Fresh full-action
replay, natural replay and exact stage-path checks cover 2004150 diagnosis calculation
rows and 1968169 holdout rows. The local diagnosis ledger reproduces the frozen
phase-gate ledger byte-for-byte. All profile, ordered-stage and raw-capture receipt
audits pass; no retry/continuation or second profile was needed in this batch.
Pending progress is not a completed route; this finite sample does not prove all seeds.
The report changes pass all 38 native/DAT CTests and reproduce the historical
campaign ledger byte-for-byte. [Planner](PLANNER.md) owns the method.

The preceding full-story seed-robustness campaign is separate historical coverage.
Eight seeds were drawn before inspection under the
[frozen story-seed protocol](../reports/native/planner_seed_protocol.json). Independent
cell16/cell32 attempts all reached Final A but exhausted their unchanged 2400-second
budgets: 16 failures, zero direct completions, 213720–228336 emitted actions each.
The [seed ledger](../reports/native/planner_seed_summary.json) and separately tracked
failed prefixes retain every attempt, configuration, hash and native cost. The
previously frozen 900-second prefix continuations all complete: 16 freshly verified
NMNB routes, covering every sampled seed with both unchanged generic profiles.
Each continuation takes 41.70–187.89 seconds including prefix reconstruction;
all original failures and costs remain charged. This is 8/8 sampled seeds, not a
guarantee over all uint16 seeds or retail validation of these new files. The
[environment and lineage audit](../reports/native/planner_seed_environment.json)
records compiler, executable identities, bounded resources and 16 byte-identical
parent prefixes. No seed-specific strategy branches were added.

The default headless runtime now proves cancel regions empty within each native
bullet callback, avoiding repeated inactive-slot scans while preserving the original
predicate for active/invalidated state. The
[execution audit](../reports/native/runtime_perf_summary.json) measures 1.42–1.45x
baseline native update throughput across three complete tapes with three paired repeats.
All 108 corpus outcomes/fingerprints and all 16 random-seed raw captures match the
baseline; the two reference discrepancies remain. Full native CTest passes 37/37.
This execution gain is separate from whole-solver speed and all-seed success.

| Current Lunatic native-profile result | Evidence |
|---|---|
| Spell 92, stage 4b | Training seeds 0/1/42/65535 complete and freshly replay with spatial/focus beam retention. Frozen R64/cell16/shoot1 also passes holdouts 7/2026. Training takes 2756–2835 updates and about 15–17 seconds in the concurrent campaign; these are not isolated timing samples |
| Complete practice Stages 1/2/3 | Seed 0: 24043/30481/38231 updates, fresh action replay and natural `.rpy` round trips |
| Complete practice Stage 4b | Seed 0: 42588 updates with R128/cell16, fresh action replay and natural `.rpy` round trip |
| Continuous Lunatic Final A story | Seven seed-0 routes complete with zero deaths/bomb inputs and natural `.rpy` playback: two from Stage 1 directly, five through native prefix reconstruction. Direct R64/beam384/cell16 takes 232074 API updates (232069 calculations), score 167865093; cell32 takes 230748 updates (230743 calculations), score 166093360 |
| Frozen random-seed Final A story | Training 4455/12525/30504/31338 and holdouts 24196/35540/1763/2138: both R64/beam384/cell16 and cell32 complete via the predeclared continuation rule, with zero deaths/bomb inputs, exact full-action replay and natural `.rpy` checks. Each route has 230864–232790 API updates; original 2400-second failures remain recorded |
| Actual retail playback | The separately repaired direct R64/beam384/cell16 seed-0 story replay clears all six stages through Final A in unmodified retail 1.00d under Wine 8.0: cumulative deaths/bombs zero, internal score 167865093 and natural return to the replay menu. One tested file/profile, without a complete per-frame retail differential |
| Pinned replay corpus | 106/108; one early completion and one fingerprint mismatch remain unresolved, including in the pinned full native build |

All four practice `.rpy` streams retain identical rows after adding explicit
zero-death/zero-bomb-input checks. Whole-window verification rejects late deaths,
incomplete/capacity-failed or interrupted rollouts, and commit agreement includes
the actual human/youkai form. Search exhaustion is retained as a failure, not an
impossibility proof. The baseline global-score pruning failures remain in the ledger.

Story verification counts actual gameplay calculations separately from API updates.
Five source-owned stage handoffs return before the gameplay/replay callbacks. An
inert hook beside the native gameplay counter records those boundaries; fresh
accounting receipts bind the exact raw actions, configuration, source, DAT, terminal,
score and RNG without replacing the original solver/verification binary identities.
Every story `.rpy` must match that observed count, with no stage-count tolerance.
The report retains failed episodes, prefix reconstruction and additional receipt
costs. The requested replays and full action tapes are tracked in
[native replays](../reports/native/replays/), with hashes in the campaign ledger.

The retail Wine audit exposed an export-header defect: original native `.rpy`
files declare executable size/checksum `0/0`, accepted by the native version bypass
but crashing retail's whitelist traversal. New recording declares the pinned 1.00d
target explicitly. Originals stay unchanged; a separate
[retail-targeted story copy](../reports/native/replays/story-v2--r64-b384-c16-retail-1.00d.rpy)
has a [repair receipt](../reports/native/planner_story_retail_target.json), with all
232069 native calculation rows unchanged. Its separate
[retail audit](../reports/native/retail_story_validation.json) records the successful
complete Wine playback, read-only observation limits and retained failures. It does
not validate every saved replay or resolve the two corpus discrepancies.
[Validation](VALIDATION.md) owns reproduction and evidence.

Keep training seeds 0/1/42/65535 separate from holdouts 7/2026. Published reports omit
infrastructure addresses, account names and private paths. [Validation](VALIDATION.md)
owns commands and the remaining fidelity limits; same-build native verification does
not establish retail equivalence.

## Historical verified coverage

| Outcome | Verified count |
|---|---|
| Indexed original spell IDs | 222 |
| Indexed spell-start occurrences | 431 |
| Complete offline spell worlds | 0 |
| Complete offline spell solutions | 0 |
| DAT-derived controlled complete survival profiles | 1: ID179 Easy |
| Synthetic 7200-frame profiles solved by baseline rolling beam | 2 |
| Synthetic 7200-frame profiles solved with optional goal recovery | 3 |
| Synthetic 72000-frame continuous profiles with replayed routes | 1 |
| Older source-driven 600-frame particle fixtures | 2 |

The table above records the legacy subset engine. Controlled survival is counted
separately from its source-faithful whole-world coverage. The tracked native runtime
has its own profile and evidence:

| Native headless outcome | Verified coverage |
|---|---|
| Complete stages | All nine stage entries: Easy Stages 1/2/3/4a/4b/5/6a/6b via rolling portfolio, continuous Extra via bounded native repair; Reimu/Yukari, seed 0, every selected tape freshly replayed |
| Complete spell survival | Raw ID179 Easy, stage 6b, seeds 0/1/65535; 1292 updates including wrapper |
| Spell solver sweep | 56/56 enumerated Easy standard and Extra checkpoints, seed 0: 54 rolling-portfolio clears plus bounded native-prefix repair for IDs202/203; every selected tape freshly replayed |
| Transform/profile cross-check | IDs 85/198 pooled-laser motion, ID89 direct ECL, ID93 imminent pooled laser, IDs 193/195 source-vector and ID199 linear ranking complete for seeds 0/1/65535; ID201 bounded WAIT/ECL-shot profile complete for seed 0; ID204 relative-direction profile complete for seeds 0/65535; IDs32/139/167/183 two-leg profiles complete for seeds 0/1/65535 |
| Genuine failed baseline | Focused stationary ID179, seed 0; collision at update 382 |
| Stage 6b failure diagnosis | Easy seed 0 reactive; update 854, bullet slot 664; late intervention fails 9/9, one-update-earlier leftward intervention survives 3/9 |
| Deterministic feedback | Fresh-process tapes agree on per-frame projection, terminal, RNG and graze/score/gauge |

No universal solver or retail executable equivalence is claimed. Opcode counts and
passing component tests are not completion percentages.

## Historical results that matter

- The imported production calc chain runs without video/audio initialization or a
  wall-clock limiter. It retains gameplay and ANM updates, original shared RNG,
  dialogue, damage, cancellation, items and graze feedback. It stops at actual stage
  clear or spell end, without resetting the world between boss phases
- The current stage records cover 393119 native updates across nine entries,
  including the later Stage3/6b replacement runs. `spell-portfolio` clears Stages
  1/2/3/4a/4b/5/6a/6b in 24135/32448/39767/22089/43700/43348/61041/58853 updates.
  The rolling Extra route passes ID192 and collides in ID202 at 67738; bounded stage
  repair first reaches ID203 at 69227, then a verified resumed episode clears at
  92339 with fresh O3/O0 agreement. Every selected tape freshly replays; native carried
  bullets, items and RNG persist between phases
- Stage 6b's reactive failure is now reproducible with native collision bounds and
  independent nine-direction replays. Recording-mode movement uses the previously
  latched input; the heuristic assumes immediate movement and uses center distances
  rather than overlap feasibility. Replacing update 853 with left/up-left/down-left
  avoids update 854; replacing update 854 is too late. This is a local escape witness,
  not stage completion. No strategy parameters or original update order were changed
- The hazard policy accounts for the one-update input latch, forecasts existing native
  lasers, and mirrors active vector-acceleration order from source-owned state. A
  conservative laser broad phase reduced ID151 policy time from about 7.8 to 2.1 seconds
  without shortening its 120-update horizon. The native update remains the collision oracle
- On the fixed seed-0 Easy/Extra matrix, the simple reactive policy completed 19/56
  checkpoints. `spell-portfolio` completed 54/56 with no lost baseline completion;
  all 56 success/failure tapes replayed with matching terminal, RNG, feedback, collision
  and trace projection. IDs202/203 additionally complete through the bounded repair recipe
- Vector-acceleration projection fixed ID193's transform-0x10 collision and completed
  IDs 193/195 for seeds 0, 1 and 65535. ID199 instead completed all three seeds with
  constant-velocity ranking; its isolated selector lives outside the generic kernel
- Preserving the native final active-laser collision before removal completed ID163.
  The rolling profile retains bullet failures IDs202/203; bounded repair completes ID203
- ID85's static-angle forecast stopped at the bottom center while ten live pooled lasers
  rotated around `(192,128)`; slot 7 hit on update 631. Replacing update 630 with any
  rightward direction survives that collision. Native before/after observations expose
  slot 7's simultaneous translation and roughly `-0.003802` rad/update rotation. Its
  isolated rigid-motion profile completes the 2792-update wrapper for seeds 0, 1 and
  65535 with fresh replay and O0/O3 agreement
- ID89's unadapted source collision is a direct ECL `CalcLaserHitbox`, not a pooled laser.
  Its 590x160 hitbox first appears on update 393, and single-action replacements at
  updates 391/392/393 fail 9/9. The active ECL cursor exposes constant opcode 137 / EX 9
  from at least update 362. Its isolated adapter constrains candidates across the
  repeating callback interval, then lets the generic scorer handle bullets. It completes
  the 3162-update wrapper for seeds 0, 1 and 65535 with fresh replay agreement
- ID93's slot-0 beam is created during update 1537 and becomes fully lethal on update
  1539. Once the pooled laser is observable at update 1538, all nine input replacements
  still collide; at update 1537, only right survives the fixed suffix. The isolated
  adapter decodes the currently due non-aimed opcode 114 without consuming RNG and
  constrains proposals before spawn. All 82 decoded spawns are supported in each run;
  seeds 0, 1 and 65535 complete the 3392-update wrapper with fresh replay agreement
- ID198's static forecast first meets rotating pooled-laser slot 0 on update 446. Rigid
  motion removes that hit, but nine constant paths later oscillate into a bullet/beam
  trap on update 1062. Its isolated two-leg profile ranks every initial/continuation
  pair from shared immutable forecasts and completes the 4292-update wrapper for seeds
  0, 1 and 65535. On local x86-64 GCC 12.2 Release, difficulty 4, seed 0 and a
  6000-update budget, forecast caching retained the exact action tape while reducing
  decision time from roughly 15.3 to 10.3 seconds; file I/O is outside that metric
- ID201's generic policy hits bullet slot 24 on update 362 while its WAIT transform is
  active. The isolated profile exports a source-bounded count of unchanged-velocity
  updates through WAIT/child-transform boundaries and previews deterministic non-aimed
  ECL shots over a 32-update horizon. Its seed-0 wrapper completes at update 4292;
  15140 source-decoded spawn observations produced 184616 warnings, 638 decisions were
  constrained and no unsupported future was substituted. Fresh replay matches digest
  `954752188841509475`. At the earlier checkpoint, seed 1 collided at 3455 and seed 65535 at 700; seed 1's
  confirmed gap is an RNG-dependent child pattern, so that checkpoint was not multi-seed coverage. The current multi-seed sweep additionally clears seed 1; seed 65535 still fails
- ID202's generic policy collides at update 3265 with slot 1369, a random child already
  born on update 3258. Its copied WAIT program proves unchanged velocity through that
  hit; enabling only the existing bounded WAIT projection avoids it without changing
  the 12-update horizon. Up/up-left inputs at 3264 avoid the hit; all nine replacements
  at 3265 are too late. The rolling wrapper retains failures: seeds 0/1/65535 collide at
  4204/4216/3613, with fresh replay agreement. Seed 0's new blocker is an ECL shot born
  at 4203. A deterministic-preview extension stops earlier because opcode 99 uses
  random-angle selector 10082; no RNG future was guessed or unknown shot omitted
- ID204's generic policy collides at 1205 with slot 669 while relative-direction
  transform `0x40` decelerates it. The isolated source-bounded recurrence includes the
  final turn's movement and stops before a subsequent enabled transform. With the
  same 12-update horizon, seeds 0/65535 complete at 4832 and freshly replay at O0/O3;
  seed 1 still collides at 3008. This adds seed-0 coverage, not all-seed robustness
- ID32's corner collision at update 594 is avoidable by departing one update earlier.
  A 13-update horizon removes it but still collides at 1394; the next counterfactual
  requires a one-update diagonal leg before rightward continuation. Its isolated
  81-path profile clears all three 2372-update wrappers and continuous Stage 3 at
  39767. Fresh/O0/O3 replay agrees; both the original failure and explicit extra
  horizon/candidate costs remain visible
- ID167's unchanged-horizon two-leg profile avoids entering the bottom-left trap that
  defeats the constant-path baseline at 932. It clears all three 5492-update wrappers
  with 444852 candidate evaluations each and fresh/O0/O3 agreement. Continuous Stage6b
  at that checkpoint advanced from its ID167 collision at 42560 to ID183 at 54401;
  the subsequent ID183 profile provides the current stage clear
- ID183's baseline collides at 1041. The H13-only full-wrapper experiment still
  fails at 820; H12/two-leg4 also fails at 819.
  The fatal large bullet is already observed 31 updates earlier. Explicit H32/81-path
  planning clears all three 1292-update wrappers and continuous Stage6b at 58853,
  with fresh/O0/O3 agreement and unchanged source projection
- ID139's baseline hits a bottom-excluding bounce at update 5326. Source projection
  uses sprite bounds and the pre-movement boundary test through the final bounce.
  Bounce-only horizon 12 still fails at 5327; 81 two-leg paths at horizon 12 fail at
  4355. The observed 31-update approach motivates an explicit 32-update horizon.
  The resulting isolated profile clears seeds 0/1/65535 at 7292 and continuous Stage 6a
  at 61041, with fresh-process and O0/O3 replay. It retains the baseline failure and
  reports the larger candidate/projection cost rather than hiding it
- A pre-existing empty native `fsincos` helper left laser-cancellation item positions
  uninitialized. Long Stage 6a O0/O3 score divergence first exposed it at update 32432.
  The helper now writes both outputs using the established native sin/cos path. Eighteen
  real-runtime item geometry cases fail before the fix and pass at O0/O3 afterward;
  complete Stage 6a now also agrees. This fixes native-profile undefined behavior,
  without claiming retail x87 equivalence
- Synthetic relay/lane-switch retain live bullets and both RNG streams across phases.
  Both baseline rolling planners survive 7200 frames and regenerate/replay their tapes
- A closing-gate case exposed center-seeking beam pruning. Optional geometry-derived
  goal recovery completes it without a hardcoded scene/escape coordinate. It shares
  the original 200000 expansion budget: measured peak 198117, with target probes reported
  separately. Default behavior and the failing baseline remain available
- A 72000-frame relay (20 simulated minutes) survived and freshly replayed: 38320 births,
  2400 decisions, 120-frame peak forecast and 27471 peak model bullet references. Local
  Release runtime was about 21.1 seconds including replay, not a timing guarantee
- Actual DAT ID179 executes main sub72, child73, fast-spawn/polar bullets, 1200 lethal
  phases and the timeout/end callback. Full-horizon and rolling planning survive seeds
  0, 1, 65535; stationary fails all three and greedy succeeds only 0. Every result,
  including collision/search-failure prefixes, is independently replayed without the index
- For that short player-independent spell, full-horizon solve/replay took 92–94 ms,
  rolling 387–401 ms. Repeated overlapping forecasts/searches add work; choose strategies
  using evidence rather than assuming rolling is always better
- Early exact-successor dedup avoids 32.6% of collision queries in the moving benchmark
  and 86.5% at a clamped corner, preserving routes, attempted budgets and tie-breaking.
  Both existing DAT sub40/41 route files remain byte-identical

Exact profiles and exclusions are in [Scenarios](SCENARIOS.md); measured records are
in the [report index](../reports/native/README.md). Older subset samples used Linux
x86_64, Intel Xeon Platinum 8573C, GCC 14.2. Earlier native samples use AMD EPYC 7B12,
GCC 12.2; the ID32/139/167/183/192/202/203/204 checkpoints use Intel Xeon Platinum 8573C, GCC 14.2. Both native
profiles are Release with contraction disabled; timings depend on environment.

## Existing reusable components

The repository includes DAT/ECL/SHT/ANM/STD parsing, scalar ECL calls/waits/RNG,
resumable world-effect handoffs, context ownership, timeline/motion kernels, certified
ANM timing, bullet transforms/slot selection, laser collision/lifetime, spatial indexing
and fixed-model planning. [Architecture](ARCHITECTURE.md) maps their owners.

Earlier practice-entry, effect51/62 and camera components remain checked code. They
are optional component evidence, not prerequisites or the next integration roadmap.
The old Wriggle sub40/41 fixtures are fixed-emitter slices, not whole spells.

There are 29 core CTests without private data, plus two optional pinned-source tests
(31 total). The current headless/DAT configuration has 40 CTests, including native
process/semantic/opcode/hazard contracts, the complete-scene test and the prefix
continuation test, online policy contracts and a complete online-stage/replay test.
The complete-scene test covers complete
duration, original boss transitions, the adapted IDs 32/85/89/93/139/167/183/192/198/201/204, bounded repairs for IDs202/203 and their retained rolling failures, fresh replay, and the Stage 6b
input-latch counterfactuals.
Public CI excludes DAT and reconstruction; it cannot certify those profiles. See
[Validation](VALIDATION.md) for commands and evidence limits.
The historical GCC12 Stage6b digest differed in the earlier GCC14 run. Its old numerical
profile did not pin the complete solver build, libm or CPU, so it cannot promise
cross-environment trajectory-bit equality. The named stage6b-semantic-v1 contract
retains the historical literal/report, asserts the full scene/terminal/RNG/feedback
and collision boundary, and labels numerical-profile identity unverified. Same-build
fresh replay, O0/O3 comparison, diagnostics and all 18 latch probes remain strict.
The exact historical trajectory difference remains unexplained; this is not a
cross-host or retail bit-equivalence claim.

- ID192 now uses typed source-owned body rectangles and outward-rounded all-angle
  random-move envelopes, with H12/81 paths/one-update first leg. Seeds 0/65535 complete
  3692 updates; seed 1 retains a bullet collision at 1470. Actual Extra advances from
  ID192 at 7424 to ID202 at 67738. Body overlap joins the existing earliest-overlap
  ranking on each actual path; it is not a body-only admissibility filter
- The WAIT certificate includes DESPAWN's final lethal fired movement. ID203's earlier
  WAIT/vector-only profile failed at 1711/4823/2896. Its new observed H32 beam retains
  seven player-path prefixes per first direction (63 total), but still fails at 787
  for all three seeds. A bounded native repair automatically derives rollback proposals
  from the failed trace and completes seed 0 at 5492 after 91 candidates. Each candidate
  reconstructs the actual world/RNG in a fresh process; only the proposal beam shares
  immutable observed hazards. Seeds 1/65535 have no repaired completion claim
- Repair uses at most 2 rounds, 8 rollback segments per round and 9 direction holds of
  H/2 updates (6 for H12, 16 for H32), with 144 candidate/2160000 native-update/600-second isolated-spell search limits. Initial
  execution and final fresh replay have separate costs. All failed candidates remain
  recorded; interrupted/unverified children carry a conservative full-frame-cap charge.
  The observed beam expands at most 16533 prefixes per decision, with stable original
  score ordering and deterministic first-action-family retention

## Historical completion and follow-up work

The requested seed-0 checkpoint/stage goal is complete: 56/56 checkpoint recipes
and nine continuous stage routes. Final integrated CTest: 29/29 in 692.31 seconds.
Extra's two episodes used 16117163 verified native updates in total, including
initial reconstruction, all candidates, resumed reconstruction and final replay;
independent O0 verification is separate. Zero interrupted-update charges occurred.
The successful 92339-update tape ends after ID204 and has digest
`15529738245795540729`. The compact current report preserves the 54/56 and 8/9
rolling baselines separately from the successful repair recipes and their costs.
No all-seed, all-character, retail-equivalence or optimal-policy claim is made.

1. Future efficiency work can reduce Extra's repeated native-prefix cost or improve
   rolling proposals. Preserve the completed stage and report search/verification
   costs honestly. Do not repeat a failed finite family from an unchanged frontier
2. Preserve ID204's seed-0/65535 completion and diagnose its remaining seed-1 update-3008
   untransformed-bullet collision as optional multi-seed work. Do not broaden its
   active relative-direction forecast beyond the source-owned transform bound
3. Preserve ID202's bounded H12 repair and its failed rolling/RNG seeds. Its random
   emissions are not an empty deterministic future; improving rolling performance
   still needs a justified envelope or branch-owned RNG/state evidence
4. Keep the historical cross-host numerical-profile limitation explicit; its named
   semantic contract does not detect every possible intermediate float drift
5. Optional ID201 multi-seed robustness remains future work. Seed 1 now passes;
   seeds 42/65535 still fail. Random child patterns need owned RNG/order evidence or
   a justified conservative envelope; never reuse one sampled future across
   action-dependent branches. Recheck all retained training seeds
6. After each isolated fix, rerun its baseline, full wrapper, fresh replay, the 56-case
   sweep and the affected continuous stage. Record failed seeds as failures rather than
   changing budgets, tie-breaking or RNG consumption to improve the count

Do not return to full camera/menu reconstruction merely to unblock a controlled
benchmark. Do not concatenate isolated spell fixtures and label the result an actual stage.


The final transform-contract experiment is **not applied**: it proves real forecast
boundary/clock defects, but loses two previously successful training cases. Its
exact reviewable patch and compact results are archived under
[reports/experiments](../reports/experiments/transform-contract-rejected.json).
See [Validation](VALIDATION.md#paused-transform-contract-experiment) before resuming.
That experiment remains unapplied during the new fork-planner integration. Its
holdouts were not used to tune the rejected transform patch; the new fork planner's
separate frozen-candidate evaluation is recorded above.
