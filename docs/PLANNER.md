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

Proposal search may merge nearby positions, prune beams, use sampled futures and
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
The owner's current campaign limit is four simultaneous solver jobs, with 320 tasks,
four CPU cores and 12 GiB memory for the entire batch. Check process limits as well
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
retain all original failures and costs. First finalize the seed-0 matrix, including
failures, then freeze fresh random seed draws across the same complete coverage.
Use `th08_planner_report --matrix` to require each attempt's checkpoint and each
successful native/replay stage path to match the manifest. Its coverage denominator
includes every catalog entry, and repeated attempts cannot inflate the distinct
verified count. Missing, budget-limited and runtime-failed cases remain unverified.

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
keep the student's queried prefix immutable. Online policy and RL implementation
remain deferred.

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
