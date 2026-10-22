# Native planner and experiment protocol

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

## Current-memory online policy

[Issue #5](https://github.com/N0zoM1z0/th08-solver/issues/5) records the integration
order: complete the standalone-stage baseline over different seeds, diagnose Stage
4B, add student-state teacher corrections, then evaluate continuous routes and actual
deadline behavior. The owner also authorizes a continuous natural-4A story while
the remaining standalone 4B/Extra failures are investigated.
The first uniform learned-risk candidate fixes nine entries, shot 0, Lunatic
(Extra difficulty for Extra), a seed-zero control and two fresh diagnosis seeds
per entry. Two additional distinct seeds per entry remain held out until the
candidate passes diagnosis and its identities are frozen. Earlier failed candidates
and costs remain evidence after tuning. This does not cover other shots/difficulties.
Current
process memory is an authorized input. Pixel recognition, menus and a strict
human-visible information boundary are not prerequisites.

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

The initial execution mode is `forward-measured`: observation copying, policy and
route-input decoration are timed through input submission, with mean, p99, p99.9,
maximum and deadline misses. Native execution excluding measured file I/O, setup,
prefix cost and whole-loop wall time are separate. It waits for late decisions;
`deadline_enforced:false` is explicit. A future bounded-work/fallback policy must
be evaluated under its actual execution behavior before any real-time claim.
Full configuration and executable/DAT/action/prefix/model hashes accompany the
result. Source archives, model provenance, process receipts and failed tapes remain
in ignored local experiments. Success requires fresh complete action execution,
terminal/RNG/calculation agreement and natural replay with zero death/bomb rows;
this does not independently certify native physics or retail compatibility.

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
The existing offline campaign retains four simultaneous solver jobs, with 320 tasks,
four CPU cores and 12 GiB memory for the entire batch. The online baseline uses a
separate low-priority units with at most four independent controllers in total,
96 tasks per unit,
6 CPU cores, 6 GiB memory high-water and 8 GiB hard limit; compilation uses six jobs.
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
above. Teacher corrections, learned reactive models and RL remain later work,
with successful and failed episodes kept distinct.

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
