# Current handoff status

Reviewed: 2026-10-23. Current implementation is
[issue #6](https://github.com/N0zoM1z0/th08-solver/issues/6): preserve compact learned
artifacts, improve generic forecasting, prepare reproducible training data and
compare small learned guidance. This continues
[issue #5](https://github.com/N0zoM1z0/th08-solver/issues/5): integrate and validate a
forward-only CPU controller using authorized current process memory, then diagnose
generic failures and build the offline-teacher/student loop. The earlier issue #1
work integrated the fork planner and retained its replay fidelity limitations.
The tracked runtime imports upstream `02ff72f`; results below from
`861bec9` remain historical native-profile evidence. They do not certify the new build.

## Active work

The owner authorizes tracking selected compact learned models while keeping raw
training/run artifacts ignored. The three inherited risk packs are now copied
unchanged into [saved models](../models/README.md), with complete table identities,
production-parser smoke checks and explicit unknown seed/episode lineage.
The teacher-plus-student pack matches the four files used remotely. This preserves
research inputs; it does not add independent model-quality evidence.
The preservation checkpoint passes all 42 native/DAT CTests in 627.88 seconds,
including the new artifact-identity check, all three production-parser loads and
the unchanged complete Stage 2 action/replay regression.
The new issue's campaign is separate from the finalized 320-task optimization
campaign below, retaining its entire ledger. New experiments keep the same limits
of four concurrent solver jobs and 320 top-level tasks, with internal label/rollout
work charged separately. A frozen protocol precedes outcome-dependent choices.

The first generic forecasting diagnosis replays the retained 107/seed-46445 tape
and exactly matches its update-3333 death, score and RNG. Its colliding bullet is
already visible and collision-disabled, moving horizontally near (-0.6, 0).
At update 3332 the source mode-restoration callback restores its retained canonical
flight, near (0, 1.3); native collision follows one update later. The current state
already contains that alternative angle/speed. No activation timetable is needed
to expose the missing motion hypothesis.
The optional motion envelope contains every modeled single-switch time between
observed and canonical flight; it changes proposal geometry, never native state.
The retained failure now completes at 3941 updates, and the frozen first-three-seed
controls complete 3/3 on each of 103/107/111. Relative to the preceding hard-only
pilot, 107 changes 2/3 to 3/3 and the other two remain 3/3; these are diagnosis
seeds, with one newly improved pair, not a population reliability result.
All nine controller episodes cost 28578 native updates and 231.93 summed wall
seconds; the initial audit adds 3333 updates. Two prespecified fresh native checks
agree on terminal/score/RNG/counts over 6378 updates. Practice replay export remains
unsupported. The public implementation reproduces the prototype's full 3941-input
tape locally. All 43 native/DAT CTests pass in 671.71 seconds, including the new
complete motion regression and unchanged default Stage 2 tape/natural replay.
The envelope defaults off: its uncertain rectangles can overrestrict movement
and do not model arbitrary later transforms. These twelve campaign tasks complete
the initial diagnosis allocation. Small-model/data work continues separately;
no heldout seed has been released.

The current approach is in [Planner](PLANNER.md). The generic planner uses native
fork rollouts and verified commits; it adds no spell-92 strategy branch. The
[campaign ledger](../reports/native/planner_current_summary.json) retains successful
and failed searches, fresh verification, budgets, executable identities and total
native costs. Unfinished processes contribute only a pending identifier.

The current owner priority follows the preserved October 23
[realtime analysis](../preparations/10.23-with-claude/TH08_online_realtime_analysis.md):
reproduce its actual spell-166 workload, validate exact kernel optimizations,
then test plan reuse and deterministic planning latency before learning work.
The subsequently supplied [reference bundle](../preparations/10.23-with-claude/th08-online-realtime-2026-10-23-public.zip)
includes an updated analysis, both prototype patches and raw external evidence.
Unmodified originals remain local; the tracked publication copy removes private
conversation metadata. Exact kernels were implemented independently before those
attachments arrived and are now being compared with them. The source analyses
stay unchanged. Their two-core no-model VM results do not transfer to the current
learned-risk profile. Fresh complete 6B controls identify spell-166 starts at
updates 42681 (no model, 62020-update completion) and 45032 (unchanged model,
64744-update completion). Exact-kernel paired timing, complete action comparison
and all six fresh execution/natural-replay gates now pass; measured results are
below. The subsequent local proposal bitset also passes the complete gates.
Ordered probe/threading ablations remain local. The owned deterministic plan-lag
model passes all six complete 6B controls and replay gates. Optional lazy indexing
also preserves both complete lag-two tapes. A frozen 24-seed paired bank is
complete; its lag-two success point estimates are lower and do not establish
equivalence. A separate generic phantom-prior pilot improves the sampled spell
111 cases but retains a spell-107 death; it remains opt-in and native-proxy only.
[Planner](PLANNER.md) owns the
algorithm-phase acceptance gates and current-memory boundary.

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

The current scope targets all nine standalone entries over different seeds. The
owner also authorizes a continuous story through the natural 4A branch while the
remaining 4B/Extra failures are investigated. A frozen uniform learned-risk diagnosis batch uses shot 0 and Lunatic
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

A predeclared joint H64/spatial-retention comparison completes the same nine
diagnosis cases: 4A is 3/3 strictly verified, 4B remains 0/3 (seed zero advances to
92, the other seeds die in 88), and Extra remains 0/3 in 201. Its seed-zero 4B
controller mean is 13.65 ms, including 225.25 seconds of spatial selection work;
longer search is not a universal success or deadline guarantee. A separate H32
current-program shot-prefix ablation on three Extra seeds advances seed zero to
202; the other two still die in 201. All three failures remain charged. No winning
seeds are spliced into a universal profile.

The source-cursor investigation identifies a constant timed callback before the
new Stage 4B direct-beam death. Paired new-policy episodes reconstruct the same
34239-update prefix: without direct warnings, death is at 34335; with them, the
first beam is avoided but a later beam kills at 34827. These two failed bounded
episodes cost 69162 native updates including reconstruction and 12.65 summed wall
seconds. A separate alive-only probe costs 35573 native updates; none of twelve
held-input variants at leads 32/16/10/9 survives the complete post-event window.
An unfocused right input avoids the beam and later dies to bullets. The probe's
`event-not-observed` result reflects censored windows, not absent hazards or proof
of impossibility. Full-stage direct-warning trials finish 0/3: seed zero dies at
34827 in 92, the other two in 88. A separate 32-pixel actor-radius comparison on
three Extra seeds advances one seed to 204; the others die in 202 and 201. All
remain failed cases.

The separately named non-practice 6A/6B starts finish 6/6 strict verified, three
diagnosis seeds per entry, using the unchanged H32 uniform policy and model. Fresh
actions and natural replay agree on each terminal score and all 402530 calculation
rows, with zero death/bomb rows. Solver work totals 402530 native updates and
1913.80 summed wall seconds; verification/replay costs are additional. There are
154 measured deadline misses. This different power-zero checkpoint does not
relabel the six failed practice artifacts, certify all 25 replay fields or change
the retained 106/108 retail-reference gate.

At a later 4B checkpoint, widths 192/384/768 all return depth eight. Removing all
other hazards leaves an independently checked constant-right path safe for the
whole forecast, yet the beam discards it. This motivates an optional generic
held-direction fallback with an explicit extra-work ledger and a focused synthetic
regression. Full-world comparisons use the same three 4B and three Extra diagnosis
seeds and retain their parent configurations; this is not a stage-specific kernel.
Paired new-policy continuations from update 34818 reproduce death at 34827 with
the fallback off and advance to 39230 in spell 99 with it on. Fresh native
execution matches the latter's score/RNG and identifies a direct medium-width
laser. The pair costs 74057 native updates including reconstruction and 59.31
summed wall seconds; 792 held paths, 32457 extra forecast updates and 44 selected
fallbacks are recorded. Both episodes fail, and the later hit follows a full-depth
forecast: retaining proposals alone does not close the remaining model gap.

The complete held-fallback ablation also finishes 0/6. All three 4B cases advance
past 88/92, ending in 96/99/99. Extra ends in 201/202/204: one seed regresses and
another advances relative to its parent profile. Work totals 317130 native updates,
2407.03 summed wall seconds, 3312 held paths, 96532 extra forecast updates and
181 selections. The option remains off by default. A current-program probe of the
later beam finds player-position operands in the arithmetic after callback
installation; observed static geometry is insufficient for that reactive motion.

Offline teacher queries reconstruct each failed student's exact prefix up to
128 updates before death, then use unchanged R64/beam384/cell16 native search
with a separately frozen 900-second budget. Five of six corrections freshly verify;
one Extra episode exhausts. A separately budgeted successor starts that trajectory
512 updates before death and verifies. Its inputs diverge before the original
late checkpoint, so it does not establish that the original checkpoint was
rescuable. The original failure remains charged. These six corrected trajectories
produce 412252 verified natural-replay rows with zero death/bomb rows. Seven teacher
episodes consume 2328699 native updates, including 380864 prefix updates, and
855.47 summed planner wall seconds; fresh verification and natural replay add
16.55 and 15.71 seconds. These are privileged offline corrections, with zero
additional online clears. They provide student-state expert data for the next
model improvement, without demonstrating a universal or real-time policy.
An input-space audit finds 7938 non-shoot frames across the six verified teacher
suffixes; every suffix uses some. The present student always sets the shoot bit.
These labels therefore describe a larger teacher action space and cannot be
directly imitated by the current student. Preserve shooting as an explicit label,
or constrain future teacher queries to the student's legal output set. Quiet
inputs alter damage and RNG consumption through ordinary native execution; no
gameplay RNG state is substituted. This observation does not prove quiet input
is necessary for every successful route.

The action-constrained teacher now supports `--shoot 1 --quiet-retries 0` across
all scenes. It disables all three quiet retry mechanisms while retaining native
RNG semantics, movement/focus proposals, budgets and the adaptive default. The real-data
regression preserves a supplied quiet prefix and freshly verifies a complete
2728-update spell with shooting on every generated input. All 40 native/DAT CTests
pass in 780.79 seconds; the default spell-92 and online Stage 2 action hashes are
unchanged. Report and documentation checks pass.

Repeating the same six T-128 diagnosis queries with this constraint verifies all
three 4B continuations; all three Extra queries exhaust. A separately frozen
T-512 successor selects every unverified case and verifies one Extra continuation,
leaving two exhausted cases. Its inputs change before the original late prefix;
it does not rescue that same late state. These nine episodes yield four verified
trajectories and five retained failures, with zero additional online clears.
They consume 1380479 native updates, including 516745 prefix updates, and 340.97
summed planner wall seconds; fresh checks and natural replay add 10.31 and 9.61
seconds. The four successful tapes cover 231308 natural rows with zero death/bomb
rows, and every generated suffix input includes shooting. This is action-matched
offline evidence, with privileged teacher lookahead retained.

All three original Extra student tapes independently reconstruct through the
first 64 updates after their T-128 query boundary without death, using the same
native build and shooting throughout. This bounded witness costs 201727 prefix
updates and 8.44 summed wall seconds. The constrained teacher exhausted before
committing even one update despite an available surviving first window: its
finite proposal family misses known paths. This is a generic coverage gap, not
an impossibility result or evidence that quiet input is required. Future candidate
retention/warm starts must retain native verification and keep recorded routes
outside the online policy. No seed/spell-specific strategy branch was added;
these repeated diagnosis cases do not provide unseen-seed validation.

The optional current-memory direct-turn envelope now covers both turn signs under
an observed-rate or literal-current-batch hypothesis. It reads no future native
state and adds no seed/spell branch. A frozen paired continuation from update
38983 dies at 39239 with the option off and strictly verifies a complete 46846-update
tape with it on. Fresh execution agrees on score/RNG; natural replay has 46846
calculation rows and zero death/bomb rows. Both reconstructions remain charged:
86085 native updates, including 77966 prefix updates, and 93.36 summed wall seconds.
An earlier observed-rate-only diagnostic fails both ways at 39239; its additional
78478 updates and 9.86 wall seconds remain recorded, with no frozen source archive.

The subsequent frozen full-stage comparison repeats all three existing 4B
diagnosis seeds with H64/beam192/cell16, the same model, warnings and held fallback.
Seed zero strictly verifies at 46757 updates, score 20329969; seed 32124 dies at
40326 in spell 99, and seed 57392 still dies at 36960 in 96. Thus the parent 0/3
becomes 1/3, not a solved stage or unseen-seed result. Work totals 124043 native
updates and 1630.36 summed solver wall seconds, plus verification/replay. On the
concurrent EPYC 9654/GCC 13.3 Release sample, the successful controller's
mean/p99/max are 13.01/17.54/21.43 ms with 1150 deadline misses. The unit's memory
peak is 585760768 bytes including its build. All 40 native/DAT CTests pass in
784.43 seconds; the default Stage 2 action hash is unchanged. The option remains
off by default and its partial geometry assumptions remain explicit in [Planner](PLANNER.md).

Independent execution reproduces all three Extra diagnosis deaths with matching
score/RNG. Their local causes differ: a newly born bullet, missing fractional
spawn movement on activation, and an existing bullet's timed relative turn.
A private activation-order prototype rejects a synthetic missed collision but
does not improve a paired continuation: baseline reaches 74243 in spell 204,
whereas the prototype dies at 67021 in 202. Both fail; 141264 native updates,
including 129772 prefix updates, and 52.68 summed wall seconds remain charged.
That isolated prototype is not promoted into the maintained policy. The remaining
4B seed-57392 hit is a pooled laser, separate from the reactive direct-turn model.

The online adapter now includes the session's supported imminent pooled-laser
preview under `--queued-lasers 1`, with an owned timer-zero birth state and
separate unsupported counters. Its regression covers pre-birth escape and the
input latch. A frozen seed-57392 T-128 pair reproduces death at 36960 in spell 96
without the adapter addition and advances to 39055 in 99 with it. Both episodes
fail; work totals 76015 native updates, including 73664 prefix updates, and
35.39 summed wall seconds. The full-stage repeat finishes 1/3: seed 57392 strictly
verifies at 46385, seed zero regresses to death at 40190, and seed 32124 dies at
40389, both in spell 99. It consumes 126964 native updates and 1673.27 summed
solver wall seconds, plus verification/replay; unit memory peaks at 579760128 bytes
including compilation. Different successful seeds from different profiles are
not spliced into a universal result. All 40 native/DAT CTests pass in 783.37
seconds with the default Stage 2 action hash unchanged.

A separate private bounded-relative-turn prototype uses already exposed current
transform state and the existing pure direction-change kernel. Six T-128 episodes
repeat all three Extra diagnosis cases with the option off/on. Seed zero changes
from death at 74212 to a strict verified complete tape at 90722, score 119445663,
with matching natural rows and zero death/bomb rows. Seed 49063 dies at 74243 in
both variants; seed 9696 advances from 74208 to 75081 and still dies. All end in
spell 204. Total work is 462709 native updates, including 403070 prefix updates,
and 259.37 summed wall seconds, plus fresh verification/replay. This is one
verified prefix continuation. The maintained `--relative-turns 1` model now adds
owned-state validation and supported/unsupported/projected-work counters; it
defaults off. Its full-stage three-seed repeat finishes 1/3: seed 9696 strictly
verifies at 90718, score 121185863, with matching natural rows and zero death/bomb
rows. Seed zero still dies at 62697 in spell 201 and seed 49063 at 65014 in 202.
Work totals 218429 native updates and 934.74 summed solver wall seconds, plus
verification/replay; unit memory peaks at 579276800 bytes including compilation.
On the concurrent remote Release sample, the successful controller's mean/p99/max
are 4.19/7.22/9.31 ms with zero deadline misses; no deadline is enforced.
All 40 native/DAT CTests pass in 617.70 seconds with two test workers; the default
Stage 2 action hash remains unchanged.

A private settled-position preview matches 348 first-callback geometry predictions
over three existing 4B tapes (120000 reconstruction updates, maximum origin error
0.00003052 pixels). Its H64 T-512 off/on pair still dies at 40328/40331, costing
80659 native updates including 79628 prefix updates and 15.25 summed wall seconds.
The separately frozen H96 pair strictly verifies both variants at 46563/46806:
93369 native updates including 79628 prefix updates and 231.94 summed solver wall
seconds, plus 4.73/4.39 seconds for fresh verification/natural replay. Because the
larger horizon also succeeds without this preview, the extra pose feature remains
unpromoted. The full-stage H96 repeat uses the exact preceding H64 executable,
model, other parameters and total budgets for all three diagnosis seeds. It
finishes 0/3: seeds zero/57392 die at 31949/31656 in spell 88, and seed 32124
dies at 39297 in 99. Work totals 102902 native updates and 2068.57 summed solver
wall seconds; memory peaks at 200712192 bytes. The local continuation gains do
not survive a full-stage restart. Increasing horizon with a fixed beam is not a
monotonic improvement; the failed candidate is retained without releasing holdouts.

A private current-WAIT/child-program positioning-risk prototype repeats Extra
seed zero from stage start. The disabled control reproduces the preceding action
hash and death at 62697 in spell 201; enabling the hint advances to 66423 in 202
but still fails. Both attempts cost 129120 native updates and 524.84 summed solver
wall seconds. Three-tape differential probes match 15100 predicted child-cursor
consumption times/positions exactly; another 866 predictions are cancelled because
their parent bullets leave the playfield before emission. Checking only the final
birth position misses parents culled on their way back into view. Probe work totals
564273 reconstruction updates, including an initial terminal-boundary diagnostic
that needed correction. This checks cursor consumption, not successful child-pool
allocation or future RNG. The feature remains private pending lifecycle filtering
and a uniform multi-seed repeat; no additional full-stage clear is established.

The lifecycle-filtered private repeat now finishes 0/3 from stage start: seeds
zero/49063/9696 die at 66303/74686/67532 in spells 202/204/202. Two advance beyond
their relative-turn parent, but the previously complete seed regresses; the
positioning heuristic is not promoted. Work totals 208521 native updates and
896.13 summed solver wall seconds, with a 189333504-byte unit memory peak. The
filter separately counts 14791 predicted-cull preview observations and 1715105 pure
origin-projection updates. A further three-tape check matches all 15100 retained
predictions, costing 188091 reconstruction updates. Two independently reproduced
deaths add 133835 updates: one bullet appears eight updates before death while
forecast depth abruptly drops from 32 to seven; the other appears one update
before death. Correct cursor geometry does not close unmodeled birth coverage.

A separate generic previous-plan fallback has completed full-stage comparison on the
three existing 4B seeds, with an additional disabled seed-zero control repeat.
It rechecks a single shifted proposal against the current forecast and replaces
only a shorter beam; the input latch, depth ties and additional-query costs stay
explicit. Synthetic crossing-bullet checks pass, including changed forecasts and
cache-alignment failures. The disabled control reproduces the preceding seed-zero
action hash, score, RNG and death at 40190. The enabled seed-zero case strictly
verifies a complete 46501-update tape, score 20407686, with matching natural rows
and zero death/bomb rows. Seeds 32124/57392 die at 40287/40337 in spell 99; the
previously complete seed regresses, so the candidate stays 1/3 and does not
replace the parent profile. Including the disabled control repeat, work totals
167315 native updates and 2316.05 summed solver wall seconds, plus validation.
The unit peaks at 183959552 bytes. The candidate checks 924 warm paths / 51234
forecast updates and makes 647 replacements before the optional held fallback.
All 40 native/DAT CTests pass in
665.09 seconds with two workers, and the default Stage 2 action hash is unchanged.
The option remains off by default. No pending run counts as a completion.

A private current-terminal-ANM clock probe matches 66040 first activation-time
predictions across three existing Extra tapes. It reads one currently pointed
termination header and its clock, without scanning or executing future code.
Diagnostic work totals 260056 native reconstruction updates, including an earlier
weaker end-state check. Timing does not certify activation transforms or later
velocity/cancellation. The optional `--spawn-clock 1` full-stage comparison finishes
1/3 under unchanged budgets: Extra seed zero strictly verifies at 90900 updates,
score 118208081, with matching fresh actions/natural replay and zero death/bomb
rows. Seed 49063 advances to 74237 in spell 204; seed 9696 regresses to 61856 in
201, losing the preceding profile's clear. The disabled control reproduces that
profile's seed-49063 action hash, score, RNG and death at 65014. Including this
control, work totals 292007 native updates and 1340.82 summed solver wall seconds;
the unit peaks at 185782272 bytes. Candidate clock observations total 2191203
supported / zero unsupported, with 14881544 phase forecast updates. These timing
matches do not establish the post-activation velocity hypothesis or an aggregate
success improvement. The option remains off by default. All 40 native/DAT CTests
pass in 628.52 seconds with two workers; the default Stage 2 action hash is unchanged.

A further three-tape source probe retains 226993 reconstruction updates with
matching terminal score/RNG. It observes 12108 vector-acceleration activations whose
velocity changes before the first fired displacement; the constant-velocity spawn
hypothesis misses that update. A private owned WAIT/vector-motion prototype matches
160 isolated native fixtures over 656 BulletManager callbacks, covering all three
spawn phases, WAIT expiry, acceleration installation and final clear. These are
source-order checks, not full-world improvements. Its full-stage Extra repeat
finishes 0/3: seeds zero/49063/9696 die at 67580/67346/68259 in spell 202. The
disabled seed-9696 control reproduces the preceding action hash, score, RNG and
death at 61856. Including that control, work totals 265041 native updates and
1211.98 summed solver wall seconds; memory peaks at 201986048 bytes. The earlier
seed-zero clear regresses, so this motion feature remains private and unpromoted.

The private joint H64 search / 32-update held-tail comparison finishes 2/3, retaining
the parent's successful seed: seeds zero/57392 strictly verify at 46713/46728,
scores 20401653/20632579, with 93441 matching natural rows and zero death/bomb rows.
Seed 32124 still dies at 40509 in spell 99. The disabled control exactly reproduces
the parent's seed-zero action hash, score, RNG and death at 40190. Including that
control, cost is 174140 native updates and 2389.95 summed solver wall seconds;
memory peaks at 231665664 bytes. Candidates check 193452 endpoints / 1318523 held
tails / 10627995 forecast updates and select 1265 different endpoints; sorting and
tail checking cost 1.62 seconds. Forecast/capture extends to 96 while branching
stays at 64, so this is a joint candidate, not an isolated tail-ranking claim.

The Extra death probes reproduce score/RNG over 136093 additional native updates.
At a further 74234-update checkpoint, an already stopped relative turn has one
supported turn update left; after it clears, speed jumps from zero to five. The
finite-difference fallback wrongly continues that jump as acceleration. Requiring
matching active-transform masks for rate history fixes the focused regression;
the parent fails it, while an unchanged active vector acceleration keeps its
unavoidable latched hit. All 40 native/DAT CTests pass in 617.74 seconds with two
workers; the default Stage 2 action hash remains unchanged. The complete Extra
repeat strictly verifies all three diagnosis seeds under the unchanged H32,
relative-turn/current-ANM-clock profile: seeds zero/49063/9696 complete at
90688/91280/90858, scores 119762351/118148248/119100826. Fresh actions and natural
replay agree, with zero death/bomb rows. This is 3/3 diagnosis coverage, not all
uint16 seeds; the same current-memory policy adds no native lookahead or rewind.
Solver work totals 272826 native updates and 1228.69 summed wall seconds; fresh
execution adds 272826 updates and natural playback adds 272826 calculation rows.
The exited unit's memory-peak property is unavailable; its 6-GiB high/8-GiB hard
limits and successful process result are retained.
A separately frozen evaluation draws two fresh CSPRNG seeds, 17735/7575,
against that exact binary/model/profile after the three-case diagnosis gate.
Both seeds are absent from retained manifests and tracked seed reports. They are
new evaluation cases, not releases of the original eighteen reserved holdouts.
The frozen candidate fails both: 17735 at 62706 in spell 201, and 7575 at 65845
in 202. This **0/2 fresh evaluation** remains separate from the 3/3 diagnosis
result. Work totals 128551 native updates and 588.06 summed solver wall seconds;
unit memory peaks at 136683520 bytes. Independent execution reproduces both
deaths, score and RNG for 128551 additional native updates. The first trace shows
corner confinement and falling forecast depth before a known descending bullet
hits; the second first observes its collider one update before the fatal latched
movement. Neither failure is a solved or successful replay case. The candidate
was not retuned or given replacement evaluation seeds.
The separately frozen post-evaluation diagnosis tests all five known Extra
seeds with the maintained collision-mode correction and one uniform H32/T32,
cell16 profile. Branch depth stays 32 while capture/forecast extends to 64;
endpoint, held-tail, spatial-selection and native costs remain charged. These
five cases are known diagnosis inputs for the new candidate, not independent
evaluation. It finishes 0/5: seeds zero/9696/49063/17735/7575 die at
55422/66703/66696/66512/54659. All three previously complete diagnosis seeds
regress. Work totals 309992 native updates and 2319.52 summed solver wall seconds;
unit memory peaks at 295944192 bytes. This joint profile is not promoted.

Independent short-window native interventions retain all failed branches and
their own action-dependent RNG futures. For seed 7575, changing only the fatal
update's sampled input still hits for all 18 direction/focus choices; starting
one input update earlier avoids that local hit for 10/18. Seed 17735's 18 earlier
16-update holds all fail by the original terminal. These are bounded local
witnesses, not stage solutions or impossibility proofs. Two original-tape controls
match score/RNG; the 56 total processes cost 3626995 native updates. A further
65845-update emitter trace shows a currently visible WAIT/child program near the
new collider. Its conditional emission-point cue matches 816 native points,
including 17 observations near the collider's first position from updates
65827 through 65843 before birth at 65844. The old adapter, its private port to
current history/clock code and the remote preflight each replay 65845 updates,
197535 additional updates total, with matching terminal score/RNG and no RNG or
calculation mutation during reads. This establishes an available positioning cue,
not child direction/RNG, cancellation or complete future-world coverage.
A private frozen H32/cell0/guard32 child-positioning comparison runs the
same five known seeds plus a disabled seed-zero control. It reuses the existing
motion/cull kernel and guard/grow/risk weights as soft cost; the source cue is not
a hard child-collision certificate. All new prediction/cull/unsupported/risk work
is separately counted. It strictly completes only seed 17735 at 91179 updates,
score 118632595; zero/9696/49063/7575 die at 66280/66356/66433/66111. This 1/5
diagnosis loses all three original clears and remains private. Its disabled
seed-zero control exactly repeats the parent's complete action SHA, score and
RNG at 90688. Including that control, work totals 447047 native updates and
1944.68 summed solver wall seconds, plus the 65845-update preflight; fresh checks
and natural playback each add 181867 updates/rows with zero death/bomb rows.
Unit memory peaks at 393273344 bytes. Candidate counters retain 273555 cues,
zero unsupported / 19567 culled points, 1873563 pure projection updates and
2052188 risk frames. A new clear from a known evaluation failure does not change
the frozen predecessor's 0/2 fresh evaluation.
A focused synthetic regression exposes a timing defect in that soft cue:
an imminent emission expires on the already latched root update, before a new
sampled input can move. The parent fails; retaining the same point through one
post-birth update, clipped to the forecast horizon, passes with reach pruning
and counts the added risk frames. This also covers a child first processed on
the next pool pass. A new private six-case comparison freezes this correction
with the same seeds, weights and budgets, plus another 65845-update native
preflight. It finishes 0/5: zero/9696/49063/17735/7575 die at
66565/66452/67009/66253/66068, losing the preceding candidate's new clear as well
as all three original clears. The disabled control again exactly repeats the
parent's complete seed-zero tape, score and RNG. Including that control, work
totals 423035 native updates and 1836.46 summed solver wall seconds, with
325300224-byte unit peak memory; fresh execution and natural playback add 90688
updates/rows with zero death/bomb rows. Candidate counters retain 239004 cues,
zero unsupported / 18241 culled points, 1639776 pure projection updates and
2027589 risk frames. A separate 66565-update death reconstruction matches
score/RNG and again first observes the collider one update before the fatal
latched movement. Fixing a cue's causal timing does not supply the missing birth
and flight geometry or establish a complete controller gain. Both child-risk
variants remain private and unpromoted; successful seeds across different
profiles are not pooled. All comparison queues in this continuation are
complete, audited and stopped, with temporary transfer packages removed. The
original eighteen reserved holdouts remain untouched.

The maintained `--tail-check T` option defaults to zero and retains H-update
branching/depth while checking held suffixes through H+T. The narrow direct-laser
arc predicate now uses oriented endpoint half-planes with the existing outward
rounding allowance. Its legacy cosine/dot tolerance rejects a clear first update;
10001 independent static-predicate samples across that angular interval have no
hit. Source-order tests cover narrow, wide and full-circle arcs. The real failed
4B prefix's family-isolation check changes from depth zero to 64 at the same
snapshot after this geometry correction; three diagnostic replays cost 121521
native updates in total. These are modeled-prefix checks, not full-stage rescues.
All 40 native/DAT CTests pass in 654.80 seconds with two workers; default Stage 2
actions remain unchanged. The complete three-seed 4B repeat with both corrections
finishes 0/3: seeds zero/57392/32124 die at 40131/40326/40310 in spell 99. The
preceding frozen 2/3 result remains separately identified; more accurate boundary
geometry does not establish a better complete controller under a partial forecast.
Work totals 120767 native updates and 1683.07 summed solver wall seconds, with a
191856640-byte unit memory peak. A separate private continuation hypothesis now
retains positive pre-birth literal rate cues through geometry-matched active
laser observations instead of immediately replacing them with the last measured
rate. Owned-cache checks cover birth timing, missing observations, stages,
weaker cues and idempotent prefix warming. Its complete repeat verifies only seed
57392 at 46513 updates, score 20814017; seeds zero/32124 still die at 40137/40315
in spell 99. The disabled seed-zero control exactly repeats its parent's action
hash, score, RNG and death at 40131. This 1/3 candidate remains private and
unpromoted; it does not recover the preceding frozen profile's 2/3 coverage.
Including the control, work totals 167096 native updates and 2318.82 summed solver
wall seconds; unit memory peaks at 199639040 bytes. Fresh execution adds 46513
updates and natural playback has 46513 rows with zero death/bomb rows.

The shot-1 Lunatic Final A story through natural 4A strictly verifies two of its
three diagnosis seeds under one unchanged profile.
The batch uses the frozen spawn-clock artifact with that option disabled, before
the later transform-history and arc corrections, and a uniform H64/spatial profile:
seed zero and two new diagnosis seeds, starting at power zero in Stage 1 without
checkpoint concatenation. Its unchanged learned model was trained on shot 0, so
this also tests transfer to a different shot. Completion requires fresh actions,
natural replay and the exact ordered six-stage path; no pending run is a clear.
The complete path is 1/2/3/4A/5/6A, with 231306 API updates, 231301 calculations,
five control-only handoffs and score 175595107. Fresh actions match score, RNG and
all five transition events; natural playback has all 231301 expected rows and zero
death/bomb rows. The policy uses no native lookahead or rewind and no reconstructed
prefix. Solver wall time is 3624.21 seconds; concurrent remote controller
mean/p99/max are 15.60/22.44/33.26 ms, with 56530 deadline misses and no enforced
deadline. Seed 29768 also strictly completes at 232670 API updates, with 232665
calculations, five handoffs and score 186628562. Its fresh transition tuples and
natural stage path agree, with zero death/bomb rows. It costs 3647.43 solver wall
seconds, with controller mean/p99 15.61/22.70 ms and 57997 deadline misses.
The complete 2/3 batch costs 630494 native updates and 9843.38 summed solver wall
seconds; fresh checks add 463976 updates and natural playback adds 463966 rows.
Unit memory peaks at 268288000 bytes. These are native-profile completions,
without enforced deadlines, retail equivalence or all-seed certification.
The owner-requested two successful replay/action pairs are now archived in
[native replays](../reports/native/replays/), with the exact frozen profile,
artifact hashes, decision timings and fresh publication checks in the
[publication audit](../reports/native/online_story_4a_validation.json). Both
published tapes freshly execute with matching score/RNG/counts and all five
transition tuples; playback of the unchanged published replay bytes again
matches the ordered stage path, calculation count and score with zero death/bomb
rows. That publication snapshot predates the retail playback below; it does not
rerun the controller or transfer the frozen result to current main.

Both unchanged online-story replay files now complete in unmodified retail 1.00d
under Wine 8.0, using separate private prefixes/displays and built-in Boss playback
mode. Read-only sampling begins at actual Stage 1/frame 0 and observes the complete
native-slot path `[0,1,2,3,5,6]`, the final clear sequence and natural replay-menu
return. Seeds 0/29768 finish at internal scores 175595107/186628562 with cumulative
deaths/bombs both zero. All 728064 loaded executable-code bytes match the pinned
original before and after each playback. Active startup effects/slow/frameskip are
2/0/0; no gameplay/RNG memory or executable patch is used.
The original observers report `terminal_mismatch`: their redundant mask check
incorrectly retains the old 4B route's `0x77`, while their already-correct expected
4A path has mask `0x6f` (111). Original failed reports and sampled traces are retained.
A separate audit derives the mask from the already-published frozen manifest,
checks the recorded ordered path, logged counters and reported maxima, final score, clear flag,
menu receipt and code integrity, and passes both files. A nonzero-death negative
control is rejected. The observers count 122582/123992 active samples at a nominal
20-ms period; the retained one-second/stage-change logs contain 2576/2607 rows.
These are sampled interoperability evidence, not a complete per-frame
physics differential. Each owned Wine group has four-core/four-GiB limits, low
CPU/I/O priority and no additional swap. These tests certify the two frozen replay
files, without establishing current-main story success or live 60 FPS decisions.
Seed 60950 dies at 166518 in Stage 5 spell 107. Five diagnostic executions
(832590 additional native updates) reproduce score/RNG and show a long-observed
phantom becoming real with a velocity change on the fatal update. The main
current ECL cursor points to a future literal batch that selects two bullet masks
and calls the source freeze/restore function; static subroutine inspection costs
zero native calculations. The current phantom-risk forecast does not model that
scheduled switch. A private current-main literal-batch census predicts 895
existing-bullet activations with exact first-update position/velocity agreement;
the adapter integration also matches the actual collider. Its pure projection
checks cost 55901 updates at that observation and separately report 103 unsupported
requests/motions. This supports a conditional source cue, not arbitrary future
world coverage. A fixed-stride census over all three unchanged story tapes checks
12625 predicted geometries across 20 scheduled activations with exact native
agreement. Its initial fired-only filter omits the fatal bullet, which native
collision feedback has already changed to despawning; the revised audit accepts
only that exact same-update collision slot and reports it separately. Both full
three-tape audits preserve score/RNG and cost 1260988 additional native updates.
A paired disabled/enabled comparison reconstructs the same prefix
ending two API updates before death, then starts two fresh policy episodes with
the same H64 profile. Both arms include the transform-mask history/arc corrections,
before the separate collision-mode correction below, and share new tie-RNG
episodes. Both strictly complete: disabled/enabled at 232336/232975 API updates,
scores 181261416/182841964. Fresh actions agree on score/RNG/counts/transitions;
natural playback totals 465301 rows with zero death/bomb rows. Work totals 465311
native updates, including 333032 prefix updates, and 2119.29 summed solver wall
seconds; fresh execution adds 465311 updates. Unit memory peaks at 264937472 bytes.
The enabled arm records 5697 positive cues, 325054 unsupported requests/motions and
295140 pure projection updates. Both arms pass, so this does not establish a cue
benefit; the source preview remains private. Neither continuation adds a full-start
online story, and the original failed seed remains failed.

The maintained rate history additionally requires matching collision-enabled
mode. Source freeze/restore can change velocity while the active-transform mask
stays unchanged; that jump is not a continuing speed/angular rate. A synthetic
source-derived regression fails on the parent and passes after the correction,
agrees with reach pruning, and preserves real acceleration under an unchanged
mode/mask. All 40 native/DAT CTests pass in 642.89 seconds with two workers,
including a full Stage 2 solve, fresh execution and natural replay with unchanged
action SHA. The successful frozen story/Extra results above predate this correction and do
not certify its full-story or Extra coverage.

Exact controller optimizations now avoid full spatial ranking and per-call hash
nodes, precompute spawn-grid sums in the original float addition order, reuse
policy-owned search storage and per-search movement deltas, and preserve half-pixel
keys with an exact integer-significand path plus the original library fallback. The
[planner](PLANNER.md) owns these mechanisms; [validation](VALIDATION.md) records the
420-case full-sort oracle, fresh/reused workspace checks, failed-root isolation
and 26165824 quantization plus 204800 movement comparisons. Budgets, node/tie ordering, learned weights,
native physics and gameplay RNG are unchanged.

Uninstrumented GCC 13.3/EPYC 9654 Release comparisons use H64/beam192/cell16,
guard16 and the same teacher-plus-student files. Complete standalone Stage 1,
shot 1/seed 0, and Stage 2, shot 0/seed 29768, have 100000-update/7200-second
budgets. Every version finishes with identical full action bytes and behavioral
fields: 23149/32843 calculations, scores 3796648/11813854 and unchanged terminal RNG.
Each row compares its own baseline/candidate measurements. Table costs include
the paired prefixes and two complete early stages; later-entry work is charged
separately below.

| Exact change | Stage 1 controller mean (ms) | Stage 2 controller mean (ms) | Runs / native updates | Summed solver wall (s) |
| --- | --- | --- | --- | --- |
| Partial spatial rank | 15.45 → 12.53 | 14.27 → 11.81 | 16 / 140784 | 1924.16 |
| Spawn-grid preparation + search storage | 12.47 → 11.07 | 11.74 → 10.50 | 28 / 169584 | 1984.41 |
| Half-pixel conversion | 11.10 → 10.84 | 10.50 → 10.31 | 16 / 140784 | 1534.09 |
| Flat spatial index + selection storage | 10.85 → 8.72 | 10.28 → 8.38 | 16 / 140784 | 1376.43 |
| Per-search movement deltas | 8.73 → 8.10 | 8.38 → 7.74 | 16 / 140784 | 1190.10 |
| Scalar AABB rejection evaluation | 8.12 → 7.95 | 7.76 → 7.64 | 22 / 455184 | 1317.17 |

Three alternating paired 2400-update prefixes per workload use the shot-1
story start, seed 0, and standalone Stage 4A start, seed 29768. They are timing
samples, not completed stories/stages. Isolated grid preparation reduces the
story-start mean 13.76--13.82 to 12.43--12.57 ms; search-storage reuse then reduces
12.42--12.47 to 12.16--12.20 ms. The latest flat-index comparison changes
11.97--12.02 to 9.95--9.97 ms at the story start and 10.97--11.01 to
8.93--8.99 ms at Stage 4A. Movement reuse changes 9.96--9.98 to 9.19--9.21 ms at
the story start and 8.92--8.97 to 8.33--8.35 ms at Stage 4A. The AABB experiment
also includes three alternating pairs after 50000 explicit 6B prefix updates;
their 300000 reconstruction updates are included in its row. All behavioral
pair checks agree. The initial
separate instrumented profile adds 2400 native updates; its timing is excluded.

Each full-stage gate freshly verifies the two unique tapes for another 55992
native updates and 55992 natural replay rows, with zero death/bomb rows. All 40
native/DAT CTests pass after each change in 697.28/636.97/688.80/662.47/943.95/724.92 seconds with
two workers; the default Stage 2 action SHA remains unchanged. At most four
controllers use a twelve-core CPU quota and eight-GiB memory cap on the shared
host; the earlier isolated micro units each use six-core/four-GiB caps.
The AABB gate's p99 is 10.79/12.14 ms, maxima are 13.21/13.70 ms and neither
sample has a decision above the 1000/60-ms deadline. These clocks exclude native execution
and file I/O, and the harness still waits for decisions. No enforced-deadline,
live retail, new seed coverage or independent physics claim follows from this speedup.

A later-entry comparison extends the same H64/beam192/cell16/guard16 profile from
pre-optimization `83d09a6` to `0ee1056`, with unchanged native library, DAT and model
files. All seven pairs have identical full action bytes and all non-timing
behavioral fields. The last column records the latest AABB recheck (`75b952e`);
these timings are descriptive rather than a paired comparison against the old
columns. The separate full 6B pair is recorded below:

| Entry / shot / seed | Updates and outcome | Controller mean, old → optimized (ms) | p99, old → optimized (ms) | Latest mean / p99 (ms) |
| --- | --- | --- | --- | --- |
| 3 / 1 / 0 | 35534, complete | 14.11 → 8.19 | 22.33 → 16.93 | 7.57 / 15.58 |
| 4A / 1 / 29768 | 43912, complete | 14.40 → 8.60 | 20.38 → 15.84 | 7.91 / 14.27 |
| 4B / 0 / 0 | 31333, death in 88 | 13.27 → 8.10 | 18.67 → 13.88 | 7.41 / 12.26 |
| 5 / 0 / 0 | 44346, complete | 14.98 → 9.34 | 20.92 → 16.62 | 8.50 / 14.97 |
| 6A / 0 / 0 | 72545, complete | 15.13 → 9.47 | 21.31 → 17.30 | 8.62 / 16.38 |
| 6B / 0 / 0 | 64744, complete | 15.55 → 9.98 | 30.18 → 27.08 | 9.13 / 26.12 |
| Extra / 0 / 0 | 61156, death in 201 | 14.45 → 8.54 | 20.67 → 14.94 | 7.79 / 13.44 |

Main-stage entries are Lunatic; Extra uses difficulty 4. The two Finals use the
documented non-practice power-zero standalone checkpoint (`story:1`, natural
route, no explicit route unlock); all other entries, including Extra, retain
their `story:0` checkpoints. All five completions freshly execute with matching
score/RNG/counts and naturally replay with zero death/bomb rows, adding 261081
native updates and the same number of replay rows. The fourteen timed runs cost
707140 native updates and 8438.46 summed solver wall seconds. Four earlier Final
setup attempts were rejected before gameplay because explicit route unlock is
invalid at a direct Final checkpoint; their exit-2 receipts and original protocol
remain retained. The corrected profile adds two four-update preflights. The
retained 4B/Extra deaths are behavior agreement, not solved workloads. These known
diagnosis seeds do not add unseen coverage; later-stage p99 still prevents a
general 60 FPS claim.

The predecessor movement recheck (`4125adb`) matches every protected parent action
byte and behavioral field across all seven later entries. Its five completions again pass fresh
execution and natural replay; both failures retain their exact update and spell.
These seven additional candidate runs cost 353570 native updates and 2992.92
summed solver wall seconds. Including the paired early-stage checks above, this
movement campaign costs 23 runs, 494354 native updates and 4183.02 solver wall
seconds; seven fresh checks add 317073 native updates and natural playback adds
317073 rows with zero death/bomb rows. The unit's measured memory peak, including
four-job compilation, is 571793408 bytes under its eight-GiB hard limit. Its
6B p99/max are 26.40/54.86 ms, with 3118 deadline misses. No budget, strategy,
model, native physics or gameplay-RNG change contributes to the measured speedup.

The scalar online AABB predicate (`75b952e`) evaluates the four original rejection
comparisons together, retaining inclusive contact and the original bounds.
All 403116 finite-geometry comparison cases and all 40 native/DAT CTests pass
(724.92 seconds); the default Stage 2 action SHA is unchanged. Nine alternating
Release prefix pairs and full Stage 1/2/6B pairs have identical complete actions
and behavioral fields. The full 6B pair improves mean 9.33 → 9.13 ms and p99
26.37 → 26.12 ms, while maxima are 54.88/54.92 ms and deadline misses 3142/2962.
The maximum does not improve, and the harness still waits for late decisions.
All nine candidate entries match their protected parent tapes: seven completions
again pass fresh execution and natural replay, and both deaths retain the exact
update/spell. The eight full runs beyond the early-stage table row add 418314
native updates and 3548.28 solver wall seconds. This complete AABB campaign costs
30 runs, 873498 native updates (300000 prefix updates) and 4865.44 solver wall
seconds; seven fresh checks add 317073 native updates and natural playback adds
317073 rows with zero death/bomb rows. Measured unit peak, including four-job
compilation, is 573657088 bytes under its eight-GiB hard limit. New raw artifacts
stay local. These known diagnosis entries add no unseen-seed, current-main story,
retail or enforced 60 FPS claim.

Private diagnosis of that spell-170 window counts 215996427 hard probes, 208114877 soft
risk calls (170034826 nonempty), 850065867 AABB tests and zero indexed lasers.
This rejects laser caching as that window's target. Shared query preparation and
two exact spatial-quantizer layouts have no repeatable end-to-end win in local
pilots and remain unpublished. Including the successful AABB pilot, the four
local pilot batches and two diagnostic passes cost 981600 native updates
(900000 prefix updates) and 929.60 summed solver wall seconds. Their raw source,
all outcomes and action comparisons remain local, outside the Release totals.

The October 23 laser/selection campaign instead reproduces the actual spell-166
workload. Prepared inverse rotations, bounded stable spatial selection and an
optional policy CPU target preserve all complete actions. Three alternating-order
2400-decision trials per model profile reduce mean 17.34–17.40 ms to 10.49–10.74 ms
with portable kernels, then 9.40–9.53 ms with `native`. `x86-64-v3` also preserves
both window tapes; it has no new complete-run claim. Separate instrumented
no-model diagnostics put 86.6% of remaining controller work in search and 9.7% in
indexing. Full controls and candidates on EPYC 9654/GCC 13.3 Release give:

| Model / policy build | Mean (ms) | p99 (ms) | Max (ms) | >16.67 ms |
| --- | --- | --- | --- | --- |
| None / previous portable | 8.459 | 25.415 | 55.044 | 2041 |
| None / exact portable | 7.866 | 12.848 | 18.296 | 23 |
| None / exact native | 6.787 | 11.653 | 16.523 | 0 |
| Existing risk / previous portable | 9.144 | 26.106 | 54.942 | 2957 |
| Existing risk / exact portable | 8.472 | 13.328 | 18.289 | 17 |
| Existing risk / exact native | 7.404 | 12.136 | 16.558 | 0 |

Every row completes at 62020 (no model) or 64744 (risk model) updates. Action
SHA prefixes remain `dfab51e6…` and `005d0ecd…`; score/RNG and all other behavioral
fields agree. Six fresh executions and natural replays check 380292 calculation
rows with zero death/bomb rows. All 40 native/DAT CTests pass in 704.22 seconds;
documentation and both optional architecture contracts pass separately. The
42 controller episodes cost 2045526 native updates, including 1578834 prefix
updates, and 4191.97 summed wall seconds; fresh execution/playback costs are
additional. Portable/native binaries are `323e8ad6…`/`70c9756a…`; the source archive,
compiler flags, DAT/model hashes and full receipts remain in ignored local storage.
These concurrent host measurements preserve behavior without enforcing deadlines;
two zero-miss complete samples do not establish general 60 FPS or story reliability.

The successor-key bitset replaces the 65536-slot hash without changing its
cyclic half-pixel/focus equivalence or serial first-owner order. Bounds are
acceleration hints: a new regression checks migration to the full key domain,
wrapped aliases, focus bits, scratch resets and four rounding modes. Three
alternating native prefix pairs per model preserve all actions and reduce mean
by about 4–5%. Four complete portable/native candidates retain the same two
full tapes and freshly verify with 253528 natural-replay rows and no death/bomb
rows. No candidate needs a domain expansion. Native complete-run measurements:

| Model | Mean before/after (ms) | p99 before/after (ms) | Max before/after (ms) |
| --- | --- | --- | --- |
| None | 6.787 / 6.358 | 11.653 / 11.374 | 16.523 / 16.061 |
| Existing risk | 7.404 / 6.891 | 12.136 / 11.761 | 16.558 / 16.204 |

Both native candidates have zero measured deadline misses. Portable means improve
7.866 → 7.297 and 8.472 → 7.872 ms, but maxima are 18.596/21.402 ms and misses
26/18, versus the preceding 23/17: mean gains are not universal tail gains.
All 40 native/DAT CTests pass in 660.45 seconds. The accepted 16-episode campaign
costs 808606 native updates (526278 prefix updates) and 2116.38 summed solver
wall seconds; fresh execution and playback are additional. Portable/native binary
prefixes are `f300339b…`/`5eeb7b97…`. A prior build audit stops six attempts after
finding a portable model-reader object in the native build. Four completed windows
and two interrupted windows are retained, with 185026 exact completed updates and
91200 observed interrupted updates; additional interrupted work is unknown and
bounded by their frozen frame caps. Their timings are excluded, not relabeled.

The 72 ordered kernel/probe ablations preserve their window tapes and cost
3330468 native updates, including 3157668 prefix updates, and 1901.67 summed wall
seconds. At beam192, the 8192-slot hash is slower than the original table. Per-parent
position arrays show a small mean gain but mixed maxima; the empty-grid variant
saves less than 1% in this window. Serial batch probing and condition-variable
workers are slower. Two spinning workers improve some timings, but the risk-model
mean ranges 8.715–9.501 ms against roughly 9.51 ms; four workers have maxima up to
22.064 ms. Those thread/batch layouts are not selected over the consistently faster
single-thread bitset. This does not reject all parallel planning designs. All raw
sources, identities, costs and endpoints remain local; no new seed/story/retail
or enforced-deadline claim is added by these optimization controls.

The optional owned plan-delay model completes all six 6B cases: lag 0/1/2, with
and without the unchanged model, under the same H64/beam192/cell16/guard16 profile.
Lag zero retains the full `dfab51e6…`/`005d0ecd…` tapes and default Stage 2 retains
`c4acd20a…`. Two repetitions of each lag-one/two spell-166 window have identical
actions, behavior and executor event counts. All six complete tapes pass fresh
action execution and natural playback, totaling 384516 rows without death/bomb
rows. The delayed trajectories have different RNG/state histories; their latency
rows are descriptive, not identical-workload speed comparisons:

| Model / lag | Complete updates | Executor mean / p99.9 / max (ms) | Planner p99 (ms) | Planner overruns / searches | Emergencies |
| --- | --- | --- | --- | --- | --- |
| None / 1 | 63948 | 0.805 / 3.152 / 3.256 | 9.135 | 2 / 63948 | 26 |
| None / 2 | 64472 | 0.793 / 3.106 / 3.263 | 9.197 | 0 / 32245 | 28 |
| Existing risk / 1 | 64769 | 0.953 / 4.496 / 4.799 | 9.590 | 0 / 64769 | 24 |
| Existing risk / 2 | 64563 | 0.935 / 4.436 / 4.656 | 9.479 | 0 / 32288 | 25 |

All four modeled executor samples have zero deadline misses and 23 input/position/
movement/phase invalidations apiece; no observer or carried native state is reset.
The two lag-one planner overruns remain recorded: fixed-N accounting still adopts
on schedule, so this is not an actual late-result policy. Whole synchronous
controller latency remains separately reported. The 14 episodes cost 754568 native
updates, including 350852 prefix updates, and 2311.47 summed wall seconds; fresh
execution/playback costs are additional. Binary prefix is `5f72dee8…`. All 40
non-documentation tests pass in the 41-test suite; the stale core-count declaration
was fixed and documentation passed on recheck. The strict C++ paired-bank contract
is the new core test. No fresh-seed reliability or complete-story result is added
by these known-seed controls, and no actual asynchronous/retail bridge is claimed.

Lazy indexing leaves the entire forecast and reach filter unchanged and skips only
an unused index suffix while a delayed plan is pending. Three alternating-order
spell-166 pairs per model preserve actions and all search/executor event counts;
each 2400-decision window builds about 115200 frame indices instead of 153600.
Two complete lag-two candidates retain their exact parent tapes (`cabbd899…` and
`a0abef28…`), score and RNG. Fresh action execution and natural playback agree on
129035 rows with no death/bomb rows. Complete-run executor measurements are:

| Model | Mean eager / lazy (ms) | p99.9 eager / lazy (ms) | Max eager / lazy (ms) |
| --- | --- | --- | --- |
| None | 0.793 / 0.676 | 3.106 / 3.134 | 3.263 / 4.144 |
| Existing risk | 0.935 / 0.789 | 4.436 / 4.274 | 4.656 / 4.676 |

Index construction falls about 25%; executor means fall about 15–16%. Maxima do
not consistently improve. Both candidates have zero executor misses and planner
overruns in this sample; actual synchronous controller maxima are 14.811/15.325 ms.
These concurrent measurements do not add a deadline guarantee. The 14 controller
episodes cost 684113 native updates, including 526278 prefix updates, and 618.45
summed wall seconds; four fresh execution/playback commands are additional.
Source/binary prefixes are `55fc12f9…`/`ea1c2a47…`. All 41 native/DAT CTests pass
in 636.69 seconds, and documentation passes again after the prose update.
Raw receipts remain local.

The subsequent diagnosis freezes 24 new seeds, two spell-practice proxies (111
and Extra 202), and paired no-model lag-zero/lag-two profiles before execution.
All 96 planned cases finish: 34 native completions, 62 deaths, no budget or runtime
failures. The strict C++ analyzer checks every manifest pair and reproduces
byte-for-byte locally from the collected trials and frozen protocol:

| Spell | Sync complete / 24 (Wilson 95%) | Lag-two complete / 24 (Wilson 95%) | Sync-only / lag-only | Exact McNemar p |
| --- | --- | --- | --- | --- |
| 111 | 5 (9.2–40.5%) | 2 (2.3–25.8%) | 4 / 1 | 0.375 |
| Extra 202 | 16 (46.7–82.0%) | 11 (27.9–64.9%) | 7 / 2 | 0.180 |

Lag two has lower success point estimates in both cases. These small samples
establish neither equivalence nor absence of regression; synchronous remains the
default. Modeled lag-two executor maximum is 4.637 ms with no misses, while actual
synchronous-controller maximum is 18.171 ms. Zero planner overruns against a
two-frame budget do not turn those waits into a live asynchronous controller.
The bank costs 293967 native updates and 1633.46 summed controller wall seconds,
without prefix updates. Individual bank tapes have not all received fresh/retail
verification and are not certified expert routes. Six separately drawn holdouts
remain unreleased; a post-draw audit finds no overlap with checked prior local
protocol seeds. This phase reaches task 296 of 320, retaining the six earlier
aborted build-audit attempts. Practice proxies cannot establish story probability.

The opt-in fired-phantom prior uses only the current bullet category, without a
spell/seed/timer strategy. A prespecified diagnostic pilot uses the first eight
bank seeds for 111 and the first three for 103/107. Eight frozen synchronous bank
controls are reused; twenty new pilot episodes and one new default-mode control
are run. The new native default control retains its parent action tape and all
behavioral fields. Complete paired results, including the remaining death, are:

| Spell | Ignore complete (Wilson 95%) | Hard prior complete (Wilson 95%) | Ignore-only / hard-only | Exact McNemar p |
| --- | --- | --- | --- | --- |
| 111 | 1/8 (2.2–47.1%) | 8/8 (67.6–100%) | 0 / 7 | 0.015625 |
| 107 | 0/3 (0–56.1%) | 2/3 (20.8–93.9%) | 0 / 2 | 0.500 |
| 103 | 3/3 (43.9–100%) | 3/3 (43.9–100%) | 0 / 0 | 1.000 |

This small diagnosis supports further evaluation of the generic category prior,
not an all-seed, learned-model or story claim. Extra obstacles increase search
work; one dense-103 update takes 42.392 ms. Defaults remain unchanged. Both strict
paired summaries reproduce byte-for-byte locally. The 21 new controller episodes
cost 57319 native updates and 472.57 summed wall seconds; reused controls stay
charged to the preceding bank. Prespecified 111/107 tapes pass fresh action
execution on 7184 updates, with terminal/score/RNG/calculation agreement.
The attempted natural-playback gate fails before simulation: this verifier
exports `.rpy` only for non-spell runs and explicitly leaves the practice replay
path empty. The incorrect harness assumption and failed command remain recorded;
no replay is fabricated or substitute seed selected. These proxy results are
not certified expert routes. All 41 native/DAT CTests pass in 646.05 seconds.
Source/binary prefixes are `1809b5cf…`/`e92ad99e…`; raw inputs, tapes, failed and
successful receipts remain ignored. The 21 controllers plus three verification
commands reach the 320-task campaign cap, including the failed playback command.
No holdout is released and no further campaign solver job is launched.
The remaining 107 trace has depth 64 through update 3331, depth zero at 3332 and
native death on update 3333. It supplies a concrete late-warning boundary for
further motion/activation diagnosis, without identifying the missing mechanism
or justifying a spell-specific schedule.

The owner prioritizes further measured algorithmic speedups before model/training
replacement. Native/replay correctness remains required even if a later stochastic
policy accepts a finite complete-story success rate. The eventual initial policy
target may be at least 50 percent full Lunatic NMNB runs; a stage clear rate or a few
selected successful tapes does not establish that probability.
Optional component diagnostics separate capture, policy phases, live native work,
prefix warming and output. Two local GCC 12.2/EPYC 7B12 H64/cell16/guard16 windows
use the unchanged model and 2400 live decisions each:

| Diagnostic window | Controller (s) | Search (s) | Index (s) | Forecast (s) | Capture (s) | Live native (s) |
| --- | --- | --- | --- | --- | --- | --- |
| Story start, shot 1 / seed 0 | 27.33 | 25.15 | 1.47 | 0.61 | 0.047 | 0.505 |
| 6B after 50000 prefix updates, shot 0 / seed 0 | 24.36 | 19.89 | 2.46 | 1.84 | 0.110 | 0.493 |

Search is about 92%/82% of controller work. History takes 0.050/0.055 seconds and
post-loop output 0.00067/0.00850 seconds. The late window uses movement reuse and
costs 52400 native updates including 3.58 seconds of separately charged prefix
warming; the early window predates that change. These instrumented concurrent
samples locate work, without serving as a paired speed comparison, solved story,
remote benchmark or deadline claim. Raw reports stay local. The initial diagnostic
change passes all 40 native/DAT CTests in 1029.16 seconds, including profiled/unprofiled
and complete-prefix contracts; the default Stage 2 action SHA is unchanged.
Documentation CTest also passes.

Eighteen additional distinct holdout seeds, two per
entry, remain unexecuted until a frozen candidate passes diagnosis. Overlapping
queues reserve slots to keep at most four controllers and 320 tasks overall;
controller units have 96 tasks, six CPU cores, 6-GiB high/8-GiB hard memory
limits and no extra swap. Teacher corrections use at most two planners, 192 tasks,
four CPU cores and 8-GiB high/16-GiB hard memory limits; the successor is serial.
Measured teacher memory peaks stay below 1.3 GB. No enforced-deadline or retail live
controller is established. Except for the explicitly requested two complete 4A
story captures and their publication audit, new tapes, replays, models, JSON
reports and process receipts stay under ignored local storage.

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
Six finalized online failure captures are additionally losslessly compressed after
matching the collected local SHA values, reclaiming 30535680 allocated bytes.
Every protected replay/action hash in that scope remains unchanged.
The October 23 optimization cleanup reclaims another 73035776 allocated bytes
across two checked passes: 72 completed-build objects are removed and 173 decision
tables are losslessly compressed with restored SHA checks. All 1060 protected
files retain their hashes, including actions/replays, executables, source archives,
protocols and failure receipts. Active batches, shared native/data dependencies
and the interrupted build-audit evidence are excluded from cleanup.

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

There are 31 core CTests without private data, plus two optional pinned-source tests
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
