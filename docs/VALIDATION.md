# Validation, provenance and reproduction

Core CTest cases: 29

Normal core tests require no game assets or reconstruction. Optional source comparisons
add two CTests (31 total). Native builds add process, semantic, hazard-probe and
unsupported-instruction and online-policy contracts; private DAT enables the
complete-scene, online-stage/replay and planner-prefix CTests (40 total).
Existing public CI runs sanitizer OFF/ON without DAT/source;
it does not establish real-DAT scenario success. Keep new verification proportional to
the changed behavior; no elaborate sanitizer infrastructure is required.

The shared spatial selector's `plan_contract` regression compares partial rank
selection against the frozen former full-sort algorithm in 420 deterministic
cases. It checks every retained float field, action, violation count, parent,
output position and bin statistic across ties, signed zero, infinite scores,
negative coordinates, focus bins and different widths/cell sizes. Invalid cells
and non-finite coordinates still fail. Performance checks additionally require
identical full action bytes and behavioral reports before comparing timings.
The spawn-grid contract checks clipped edges, row separation, signed zero and a
large/small-term sum whose rounding changes under reassociation. A second contract
warms search storage, takes an immediate latched-collision return, then compares
the next full decision with an observer-only control, including selection counts
and exact action/depth. These guard per-decision grid and scratch-storage reuse.
The half-pixel key contract compares the optimized path with the original
`lround`/mask expression at every fast-range rounding boundary and its adjacent
floats, plus one million finite bit samples (26165824 comparisons). Four rounding
modes additionally check exact values, exception flags, errno and unchanged mode,
including signed zero, subnormals and large/special-value library fallback.

## Current-memory CPU controller

Build `th08_online` with the native Release configuration described in the README.
The owner permits current process memory, including current transform and laser
lifetime state. This baseline is `native-memory-kinematic-v1`, not an image-only
agent. It makes no future Session updates and does not rewind the live game.

```sh
./build-headless/th08_online --dat game_data_donottrack/th08.dat --out reports/local/online-stage2 --stage 2 --difficulty 3 --seed 0 --shot 0 --horizon 32 --beam 192 --reach-filter 1 --frames 60000 --seconds 900 --log-every 1
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/online-stage2-verify --stage 2 --difficulty 3 --seed 0 --shot 0 --verify reports/local/online-stage2/online.actions
./build-headless/th08_replay_check game_data_donottrack/th08.dat reports/local/online-stage2-verify/th8_verify.rpy reports/local/online-stage2.rows
ctest --test-dir build-headless -R headless_online --output-on-failure
```

Stage names use the repository convention `1/2/3/4a/4b/5/6a/6b/extra`, rather than
the preparation prototype's zero-based numeric CLI. `--story 1 --final-route a|b`
uses the existing native dialogue/unlock-history checkpoint contract. For a
continuation, pass `--prefix` with its original checkpoint; frame budgets include
that prefix, and observer history is warmed without replaying past search ties.
Only one-update commits and the kinematic forecast are supported. Outcomes have
distinct exits: complete 0, setup/output error 2, budget 3, death 4, runtime error 5.
Output directories must be empty; failed tapes remain evidence, not successes.

Optional `--risk-dir` requires all four files: `risk_table_merged.tsv`,
`risk_backoff_seg.tsv`, `risk_backoff_global.tsv`, `nowarn_merged.tsv`.
`--risk-phantoms 1` additionally uses collision-disabled bullets as soft risk.
The maintained line parser fixes the preparation's cross-row backoff parsing, so
those results need new reproduction rather than assigning old outcome/timing
claims to the corrected model. New model hashes and training lineage belong in
the local experiment protocol. No learned model is required by the complete-stage
CTest. Its unit companion also rejects truncated/duplicate risk rows, checks
latched movement and prefix velocity history, and compares pruned/unpruned search
with interleaved independent policy instances.
The transform-history regression observes a stopped relative turn followed by its
final constant-velocity flight. Estimating continuing acceleration across that
mask change incorrectly declares the first latched update lethal; resetting the
rate history retains a complete escape, with reach-filter agreement. An unchanged
active vector transform still accelerates from rest and keeps its genuinely
unavoidable latched hit. No zero-speed or seed-specific strategy is substituted.

Optional `--spawn-guard 16` adds actor-origin soft risk, `--diversity-cell 16`
retains spatial/focus beam alternatives, and `--queued-shots 1` uses positive
bounded shot prefixes from current main program cursors. All default to zero/off;
record each setting and compare complete outcomes rather than combining the best
seed from different profiles. Queued coverage is partial and counts unsupported
preview records; zero records is not an empty-future certificate. The policy
contract also checks early avoidance of a queued birth, the unavoidable latched
update, soft actor risk without a learned table and deterministic spatial retention.
`--queued-lasers 1` independently enables the existing timed direct-ECL warning
decoder; reports count warnings and unsupported dynamic/variable geometry. Its
regression checks escape before a rotated beam appears, the already-latched first
update and rejection of a reversed lethal interval. These forecasts are partial;
actual native execution, fresh actions and natural replay remain the outcome gates.
`--held-fallback 1` adds eighteen checked constant-input proposals after an
incomplete beam. Its synthetic two-beam regression retains a complete rightward
escape that the default beam prunes at depth eight, checks reach-filter agreement
and counts the extra forecast work. A lethal first latched update still returns
depth zero. Complete-world outcomes are evaluated separately from this proposal
contract.

The optional terminal-tail regression keeps H8 branching/depth while previewing
a birth at update nine through eight extra held-input checks. It verifies early
escape, reach-filter agreement, stopping at a complete first suffix, separate
query counts, a lethal latched update and rejection of H+T above 200. The narrow
laser-arc regression retains a clear first update that the previous cosine/dot
tolerance rejects, with 10001 independent static source-predicate samples across
the angular interval. Companion checks cover wide arcs, full-circle reach and
the opposite ray at near-zero reach. These geometry contracts are separate from
full-world outcomes.

The optional `--spawn-clock 1` regression requires an escape before a supported
animation completes, checks reach pruning and rejects a purported spawn clock on
a fired bullet. A late first snapshot with one animation call left must account
for both fractional and full displacement, including an unavoidable latched hit.
Unsupported timing retains the named legacy hypothesis. The pure ANM companion
checks all three terminal opcodes, next-call/null-cursor order and refusals for
unknown, masked, malformed, blocked, negative and fractional clocks. These timing
contracts do not certify velocity changes at activation or later native state.

The optional `--warm-fallback 1` regression uses two crossing linear bullets and
a one-node beam. Rechecking the previous plan suffix restores a full 24-update
proposal after ordinary replanning discards it at depth 13. The same check covers
reach pruning, skipped decisions, changed latched focus, a new forecast collision,
depth ties, an already-latched death and the exact additional-query counts. These
are synthetic proposal contracts; complete native outcomes remain separate gates.

`--direct-turn-envelope 1` tests a current-memory angular uncertainty model.
Its policy contract includes opposite-turn escapes from signed extrapolation,
sixteen orientations around the full circle, reachable-envelope agreement and
pre-birth cue timing through the input latch. Raw-batch tests cover difficulty,
both forward branch paths, truncation, variable writes and backward-loop rejection.
Reports add `turn_hint_records`, `unsupported_turn_hints` and
`turn_envelope_frames`; these count a partial heuristic, not certified future
geometry. Observe both complete outcomes and controller latency.

The queued pooled-laser regression starts from a harmless birth update and
requires movement before the first long active hitbox. A one-update-late snapshot
cannot recover that escape through the latched input. It also checks reach-filter
agreement and rejects a purported spawn whose timer has already advanced. Source
preview contracts remain independently exercised by the native planner tests.

The relative-turn policy regression warms a stationary velocity history, then
installs an owned source turn. It requires an early escape, counts only the
bounded kernel updates, checks reach-filter agreement and the first input latch,
and rejects aimed/inactive state masquerading as an independent relative turn.
The existing bullet-motion source differential remains a separate kernel check.

For an always-shoot student's offline teacher queries, add `--shoot 1
--quiet-retries 0` to `th08_plan` with the original checkpoint, immutable
`--prefix` and explicit total frame/time budgets. Inspect every generated suffix
input's shoot bit, the byte-identical prefix, `options.quiet_retries:0` and
`quiet_windows:0`; a zero quiet-window counter alone does not cover checkpoint
or short retry delays. The prefix CTest completes a real spell from a supplied
quiet input, retains that input exactly, checks every continuation input and
freshly verifies the result. The default planner retains adaptive quiet retries.
Matching output actions does not remove the offline teacher's privileged future
rollouts or establish online completion.

`online.json` binds executable, DAT, exact action/prefix bytes and each risk file;
the local protocol additionally binds the source archive and environment. Decision
timing includes observation copying, policy and input decoration, excluding native
execution and file serialization. Prefix reconstruction contributes to native and
whole-loop costs; setup is separate. Buffered `decisions.tsv` records predecision
state and the submitted input when requested. Reports retain mean, p99, p99.9,
maximum and misses against the configured deadline. `forward-measured` waits for
late decisions and explicitly reports `deadline_enforced:false`. A passing stage
or a low mean is not proof of a real 60 FPS controller.

Fresh verification must match exact terminal, score, RNG, action hash and
calculation/control events. Natural replay must finish with the same score and
observed calculation rows, zero death rows and zero bomb-input rows. These checks
establish native execution agreement, not an independent physics/retail oracle.
Source, model, process receipts, timings and all failures normally stay local;
publish sanitized conclusions and maintained code/prose. The owner's explicit
request for the two successful complete 4A story captures is a narrow exception:
their [publication audit](../reports/native/online_story_4a_validation.json) retains
sanitized provenance, timings and native verification receipts. It does not add
retail playback or enforced-deadline evidence.

Final-stage practice recording and natural playback currently differ at the
clear-bonus branch. Native practice sets `isPracticeMode`; playback clears it and
uses a separate replay-practice predicate. Static inspection of the unchanged
retail 1.00d executable (SHA256
`330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`)
agrees with the native GUI branch: final-stage non-practice clear adds
`2500000*lives + 500000*bombs` before difficulty scaling. A separately checked
supplied Stage 6A tape predicts and reproduces the same 33750000-point Lunatic
offset seen in the Stage 6B control; it starts at the clear-bonus update. Position
also diverges later in the clear tail, where playback freezes input while live
control continues. This explains the observed mode difference without changing
the score gate, and does not certify a complete retail replay or all 25 fields.
Keep the failed equality results. Any non-practice final-stage checkpoint must be
named and evaluated separately rather than relabelling a failed practice result.

## Native fork planner and replay gate

The C++ replay suite requires the accepted corpus, manifest and schema from upstream
`02ff72f29e7b4928f21c5a2fb0f57ffa0cec5415`. Pass their locations explicitly; raw replay
captures and game data remain local. The default full gate pins the 108-entry manifest
and supported v4 schema, verifies replay hashes, creates a new output directory, and
requires natural successful captures with every expected row/fingerprint.

```sh
./build-headless/th08_replay_suite --checker build-headless/th08_replay_check --dat game_data_donottrack/th08.dat --manifest reference/config/replay-fixtures.json --schema reference/scripts/replay-external-schema.json --corpus reference/tests/fixtures/replays/th08-1.00d/accepted --output reports/local/fidelity --jobs 8
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/spell92 --stage 4b --spell 92 --difficulty 3 --seed 0 --rollout 64 --diversity-cell 16 --shoot 1 --checkpoint 60 --keep 12 --frames 10000 --seconds 180
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/spell92-verify --stage 4b --spell 92 --difficulty 3 --seed 0 --verify reports/local/spell92/plan.actions
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/story --stage 1 --story 1 --difficulty 3 --seed 0 --rollout 64 --beam 384 --diversity-cell 16 --checkpoint 120 --keep 64 --shoot 1 --frames 250000 --seconds 2400
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/story-resumed --stage 1 --story 1 --difficulty 3 --seed 0 --prefix reports/local/story/plan.actions --rollout 64 --diversity-cell 16 --checkpoint 120 --keep 64 --frames 350000 --seconds 1800
./build-headless/th08_plan_matrix --seed 0 --output reports/local/matrix.json
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/final-b --stage 1 --story 1 --difficulty 1 --shot 1 --seed 0 --final-route b --rollout 64 --beam 384 --diversity-cell 16 --checkpoint 120 --keep 64 --frames 350000 --seconds 3000
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/extra --stage extra --story 1 --difficulty 4 --shot 11 --seed 0 --frames 350000 --seconds 3000 --diversity-cell 16
./build-headless/th08_hazard_probe --dat game_data_donottrack/th08.dat --stage 4b --spell 92 --difficulty 3 --seed 0 --event-update 393 --leads 393,360,320,280,240,200,160 --output reports/local/spell92-probe.json
./build-headless/th08_planner_report --root reports/local/campaign --fidelity reports/local/fidelity/summary.json --known-binary-sha256 01cdab0637be646f45d3e0db12734837a979aa5a8e218fc8dabc7e427f0171e8 --replay-dir reports/native/replays --output reports/native/planner_current_summary.json
```

`reference/` above is a placeholder for supplied pinned corpus files, not a runtime
dependency. `--case ID` selects an explicitly labeled subset but still validates the
complete corpus. Explicit non-108 fixtures exist for synthetic protocol tests and do
not count as a full gate. `--rows FILE --schema FILE` independently fingerprints an
existing capture. One Session step can perform multiple replay calculations; capture
rows and API call counts are checked separately.

The current measured corpus result is **106/108**, not the handoff's recorded 108/108.
`lunatic-1-b` ends early at 36200 rows; `lunatic-11-a` has all 116281 rows but a different
fingerprint. Both reproduce with O0/O3 and a fresh pinned full native Debug build;
restoring the draw chain does not fix them. Their retail-divergent row and cause remain
unknown. Do not relax the gate or rewrite reference fingerprints. Matching projection
fields do not establish every internal state or every input sequence.

Planner reports include native attempts/completed updates, failed rollouts, checkpoint
retry costs, options and capacity. New reports hash DAT and executable once before
forking, outside native timing. The first recorded comparison campaigns predate this
metadata addition; the retained spatial-planner binary is identified separately in
their evidence. That explicit composer hash is specific to those archived campaigns;
it must not be assigned to other builds. New campaigns use producer-recorded hashes.
Concurrent campaign wall times are not isolated single-core benchmarks. The current
remote campaigns use Linux x86_64, AMD EPYC 9654 and GCC 13.3 Release, with contraction
disabled. Local diagnostic probes use AMD EPYC 7B12/GCC 12.2 Release. Independent jobs
have task-specific scratch/output directories, low CPU/I/O weights, bounded memory
and no additional swap; the shared host's other workloads are not modified.

The ledger requires a finished process, matching fresh terminal/RNG/checkpoint and
zero deaths before counting a completion. Practice/story cases additionally require
a natural `.rpy`, matching calculation-row count and final score, and explicit zero
`death_rows`/`bomb_input_rows`. These counters count observed rows, not death/bomb
events. Refreshing them on the four practice tapes preserved every original row byte.
Replay calculation rows and Session API calls are separate cost totals.

For continuous story, five native stage-control calls can complete without a
gameplay calculation. The observer is beside `gameplayFrameCounter++`, after the
setup early-return gates; the earlier trace/RNG hook stays at its original boundary.
Reports bind `frames = calculation_updates + control_updates` to explicit ordered
stage-transition events, and natural replay rows must equal the observed calculation
count. The measured 232408-action route has exactly 232403 calculations, with control
calls at 24158, 56541, 95948, 138845 and 178264. This is measured source behavior,
not permission to subtract the number of stages from an arbitrary replay.

For older completed campaigns, retain the original plan and same-binary verification.
A separate `<case>-accounting/verify.json` supplies missing observations from a new
fresh run, bound to exact raw action SHA, source/DAT/configuration, terminal, score
and RNG. Its executable and costs remain separate. New plan/verification pairs must
agree directly on counts and events. `th08_replay_check` also hashes its actual replay,
DAT and executable before capture. `--replay-dir` archives only verified whole-scene
originals and full action tapes, checks the replay hash, and emits both artifact hashes
in the ledger. Previously published originals remain under
[native replays](../reports/native/replays/). The owner additionally requested the
two complete online 4A story replay/action pairs and their publication audit.
All other new replays, action tapes and generated JSON stay under ignored
`reports/local/`, together with fresh-verification copies
and retained raw rows. Code and maintained prose continue through progress commits.
Git newline conversion is disabled for `.actions` and `.rpy` so
platform checkout settings preserve the exact bytes bound by these hashes.

Completed remote captures may be retained as `.rows.gz`. The storage audit checks
every restored raw SHA before removing its uncompressed copy; use
`gzip -dc capture.rows.gz > capture.rows` before a raw-row comparison. Later
successful captures retain raw SHA/size and native replay receipts, while their
reconstructible raw rows are removed. Replay/action files and failed diagnostic
captures remain available. The [route environment audit](../reports/native/planner_route_environment.json)
records retained evidence, cleanup receipts and bounded shared-host resources.

For new batches, pass repeatable `--group NAME` arguments; each named directory must
exist. Explicit groups require `<case>-verify.exit` and, for whole scenes,
`<case>-rpy.exit` to be zero in addition to their successful JSON reports. Legacy
default groups remain reproducible without these previously unrecorded sidecars.
The suffixes `-verify`, `-rpy` and `-accounting` are reserved for validation sidecars.
Use a local `--failed-actions-dir`, as in the commands below, to retain each failed
attempt's final emitted tape with a hash, parsed NMNB input/count checks and the
role `failed_search_prefix`. These tapes are execution-unverified prefixes, not
successful demonstrations or all rejected internal rollouts. Existing artifact
names are idempotent only for identical bytes; conflicts fail without overwriting.

For full-run baseline batches, add `--matrix reports/native/planner_route_matrix.json`.
The report checks the complete generated catalog, each attempt's checkpoint and
every successful native/replay stage path before archiving, and requires validation
process receipts in this mode. Its `coverage` object
binds the manifest SHA and counts distinct route IDs across all selected groups;
retries retain their separate attempt costs. `expected_combinations` includes the
whole manifest, while `observed_combinations` includes only IDs present in the
input groups. A zero attempt-level pending count does not imply full coverage.
Omitting `--matrix` preserves the historical report format.

For sparse random evaluations, pass `--sample FILE` instead of `--matrix`. A sample
manifest has `schema_version: 1`, `sampling_unit: "seed-case-pair"` and an explicit
`cases` array using the catalog descriptors, with each case's own seed. It may mix
seeds and omit unselected combinations. Optional `sample_phase` is `diagnosis` or
`holdout`; when present, every case must carry the same phase. The report checks
the canonical case IDs, initialization profiles and full stage paths, binds the
sample SHA, and reports pair coverage separately from distinct-seed coverage.
`verified_unique_seeds` counts seeds with at least one verified selected pair;
use pair coverage to assess completion of every planned run for a repeated seed.
The two manifest options are mutually exclusive. A zero pending count does not
mean that every planned sample pair has been observed.

```sh
./build-headless/th08_planner_report --root reports/local/seed-campaign --fidelity reports/local/seed-campaign/fidelity-v1/summary.json --group training-c16 --group training-c32 --group holdout-c16 --group holdout-c32 --group training-resume-c16 --group training-resume-c32 --group holdout-resume-c16 --group holdout-resume-c32 --replay-dir reports/local/seed-campaign/archive/replays --failed-actions-dir reports/local/seed-campaign/archive/failed-actions --output reports/local/seed-campaign/planner_seed_summary.json
```

The current full-run ledger uses all four portfolio groups:

```sh
./build-headless/th08_planner_report \
  --root reports/local/route-matrix-v2/results \
  --fidelity reports/local/route-matrix-v1/results/fidelity-v1/summary.json \
  --matrix reports/native/planner_route_matrix.json \
  --group matrix0-v2-c16 --group matrix0-v2-resume-c16 \
  --group matrix0-v2-c32 --group matrix0-v2-resume-c32 \
  --replay-dir reports/local/route-matrix-v2/archive/replays \
  --failed-actions-dir reports/local/route-matrix-v2/archive/failed-actions \
  --output reports/local/route-matrix-v2/planner_route_summary.json
```

The first random diagnosis ledger uses its frozen 12-pair sample:

```sh
./build-headless/th08_planner_report \
  --root reports/local/seed-generalization-v2/results \
  --fidelity reports/local/route-matrix-v1/results/fidelity-v1/summary.json \
  --sample reports/local/seed-generalization-v2/diagnosis.json \
  --group random1-diagnosis-c16 --group random1-diagnosis-resume-c16 \
  --group random1-diagnosis-c32 --group random1-diagnosis-resume-c32 \
  --replay-dir reports/local/seed-generalization-v2/archive/replays \
  --failed-actions-dir reports/local/seed-generalization-v2/archive/failed-actions \
  --output reports/local/seed-generalization-v2/diagnosis-summary.json
```

The [execution protocol](../reports/native/runtime_perf_protocol.json) fixes three
complete archived tapes, an incomplete prefix and three alternating paired repeats.
The [audit](../reports/native/runtime_perf_summary.json) records every timing sample,
raw receipt hash and normalized execution contract. Its baseline is native source
`0215b7f`; the default cache candidate is `2b30ad1`, both GCC 13.3 Release with the
same floating-point flags. An independent IPO build uses the baseline source and
`-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`. IPO is not enabled by default or combined
with the measured cache gain.

Build each source snapshot into its own measurement directory. For example, the
seed-4455 full-tape workload on the current build is:

```sh
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/perf-seed4455 --stage 1 --story 1 --difficulty 3 --seed 4455 --verify reports/native/replays/training-resume-c16--s4455.actions
```

Use fresh output directories for each repeat and check process exits as well as the
JSON. Verification consumes the entire supplied tape; search-only option defaults
in its receipt do not cap that tape. Compare every execution field after excluding
only native timing and binary identity. Require identical frames, terminal, source,
DAT/action hashes, score, RNG and ordered calculation/control events. The incomplete
prefix must retain exit 2, `verified:false` and no complete terminal.

The cache build and the separate IPO build reproduce all 108 baseline case tuples
without changing the two reference failures. Additional raw captures of both failing
fixtures match byte for byte, as do all 3706547 calculation rows from the 16 new
seed routes. The additional training-only planning pair uses the original cell16
seed-4455 prefix and unchanged 900-second continuation profile; its full actions and
all deterministic plan fields match after excluding timing/producer identity.
Native timing excludes file I/O; shared-host scope CPU includes compilation and
validation, and recorded capture/API counts are separate units. These are execution
agreement checks, not independent physics or retail proofs.

One full source/adapter differential additionally plays the archived
`story-prefix-v1--p132200-r64-b384-c16.rpy` through the pinned full native Debug/O0
build, retaining production draw callbacks with private Xvfb, dummy audio and
llvmpipe rasterization disabled. All 232403 calculation rows (23240300 bytes)
match the headless capture byte for byte: SHA256
`c560ef53295b9a62e949e8fd79fd5c2c6b576d137e3398b108caa4252bb913ea`.
Both captures end naturally with score 169789855, zero death/bomb-input rows and
the same six-stage route. The full-native executable SHA256 is
`9aae4ce60d1a5e8426b1675b7fe7504e0a74f7ed70610ada5c4d692e5a456b00`.
The additional validation costs 232403 calculations and 356.01 seconds including
debugger, display setup and trace I/O, outside the solver campaign totals. This
checks that adapter/profile for one tape; the two paths share source and numerical
behavior, so it is not an independent retail physics oracle.

The export fix's full Release native suite passes 36/36 CTests in 950.99 seconds,
including complete real-data scenes. Earlier receipt hardening passed 35/35 in
751.96 seconds before the export regression was added. Concurrent host workloads
make these suite wall times unsuitable as isolated solver benchmarks. The current
full 108-fixture run retains exactly the prior row counts, fingerprints and outcomes:
106 pass and the same two failures remain. No expected fingerprint is changed.
After the repair utility and its additional contracts, all 35 tests excluding the
unchanged long scene pass again in 21.67 seconds; the core suite passes 29/29.

Prefix continuation is a new planning episode over a natively reconstructed world,
not a stage reset. Its raw input hash and reconstruction costs remain in the ledger;
the final full tape must replay from its original start. Contract checks cover
chunked/malformed/bomb inputs, explicit budgets and terminal boundaries. Native
checks cover successful continuation, exact-terminal completion with no rollouts,
trailing-action rejection, a death at update 382 and budget exhaustion before update 1.
`--verify` executes the entire supplied tape and records its actual native cost;
search frame/time limits apply to planning episodes. Bound verification separately
with a process timeout when running a campaign.

The updated upstream polar math also changes four Stage6b collision values. The named
`stage6b-02ff72f-semantic-v1` contract keeps the old `stage6b-semantic-v1` fixture intact,
pins the new source/geometry, and retains strict fresh replay and all 18 latch probes.
Neither contract asserts universal cross-host float identity.

### Retail replay export identity

The first Wine audit found a real export defect: archived native replays declared
`0100d` but serialized executable size/checksum as `0/0`. The native loader's
`TH08_MODERN_PORT` version bypass accepted them. Retail 1.00d instead failed its
`th08_0100d.ver` whitelist and crashed while listing the replay; its unterminated
last-line traversal reached address 1 in `strncmp`. A known accepted retail replay
loaded and played under the same Wine configuration. Native round trips alone had
therefore missed an incompatible file header.

Recording now declares the pinned retail target explicitly: size `840704`, checksum
`2724749753`, version `0100d`. Reports keep that declaration separate from the native
producer's SHA256. It neither changes gameplay/RNG nor certifies retail execution.
`headless_replay_export_contract` first failed with serialized size zero, then passed
after the fix. It checks a complete fresh practice export, exact tape/replay outcome,
legacy repair row identity, idempotence, malformed input and refused output overwrite.

Preserve archived originals. To produce a separately named copy:

```sh
./build-headless/th08_replay_target reports/native/replays/story-v2--r64-b384-c16.rpy reports/local/story-retail-1.00d.rpy
```

This C++ tool accepts only a validated zero-identity or already-targeted 1.00d stream.
It changes the two decoded identity fields at `[0x124,0x12c)`, rebuilds compression
and the enclosing checksum, and refuses an existing output. All other decoded bytes,
stage offsets and trailing user data persist; already-targeted files copy byte for
byte. It reports hashes and never claims gameplay validation. The
[repair receipt](../reports/native/planner_story_retail_target.json) binds the original
to the separately saved
[retail-targeted copy](../reports/native/replays/story-v2--r64-b384-c16-retail-1.00d.rpy).
Both produce the same 232069 native calculation rows, SHA256
`ad7d90ea5d36904d75498c621f18d7be88ca4f42554eb9100ddd306a3eb1c3b5`.
Original archived replay hashes and the campaign ledger remain unchanged.

The separately recorded [retail audit](../reports/native/retail_story_validation.json)
plays this exact repaired file from Stage 1 through Final A in retail 1.00d under
Debian Wine 8.0. It observes the final stage-clear sequence, cumulative deaths/bombs
both zero, internal score `167865093` (display `1678650930`), and natural return to
the replay menu. The retail EXE SHA256 is
`330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`;
all 728064 loaded executable-code bytes match the original before and after playback.
No executable, gameplay memory or RNG patch is used.

This is a manual interoperability audit with read-only process sampling at a nominal
20 ms interval. The continuous sampler attached in Stage 2 at gameplay frame 14651;
the genuine Stage-1/frame-0 menu-start receipt is separate. Its original all-stage
coverage gate therefore reports `terminal_mismatch` (mask 118, missing Stage 1),
retained in the audit. Cumulative death/bomb counters persist through stage handoffs.
The separate terminal receipt observes the actual final clear and menu return;
replay's terminal branch does not set the normal game's `gameCleared` bit. It uses
`GUI_MSG_SHOW_STAGE_RESULTS` / `stageClearSequenceActive` instead. Retail also
continues fade calculations after the recorded input ends; these are not equated
with native capture rows. Recorded score and selected state restore at replay stage
handoffs, so matching terminal scores are not a complete physics differential.

To repeat the manual test, place an owned retail copy under ignored
`game_exe/retail-audit/game/`, put the repaired file in its `replay/` directory as
`th8_01.rpy`, and use a private 32-bit Wine prefix and display. Start windowed with
audio off, effects maximum, frameskip zero and slow mode off. Select Replay, the
first stage and the built-in Boss playback mode (2), which fast-forwards nonboss
sections without enlarging gameplay steps. The audit bounds its process group to
2 then 4 CPU cores, low CPU/I/O weight 10, nice 10, memory high/max 3/4 GiB and no
additional swap. Peak Wine-group memory is 429535232 bytes (about 410 MiB), with
zero memory-limit/OOM events. A prior black fullscreen startup was stopped before
the memory cap; that setup failure and the original header crash remain recorded.
The owned Wine/display processes are stopped after collecting terminal evidence.

Read-only probes are specific to the hashed x86 EXE: flags `0x0164d0b4`, current
stage `0x0164d2cc`, globals pointer `0x0160f510`, cumulative float deaths/bombs-used
at globals offsets `0x64/0x84`, and replay/title pointers `0x018b8a28/0x018bde08`.
No full per-frame retail projection or all-eleven-file retail validation is claimed.
Private raw game data, screenshots and diagnostic logs remain ignored.

## Local build and core checks

Requires CMake 3.16+, C++17 and OpenSSL development libraries. Linux x86_64 is tested.
Modern float32 with contraction disabled is the numerical profile; no fast-math.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
mkdir -p reports/local
```

The scenario CTest covers complete duration, seeded repeat/replay, phase/RNG continuity,
malformed checkpoints and the bounded-search counterexample/recovery. Planner tests
compare statuses, exact action/position bits, tie-breaking and attempted budgets with
the retained reference. Other tests guard their named component contracts.

## Reproduce the current solver evidence

Run serially for useful Release timings. Serialization occurs after timed execution.
Exit 0 means the report/check ran; inspect each outcome, since collision/search failure
is valid benchmark evidence and not a solved scene.

```sh
./build/th08_scenario_cases --scenario all --strategy all --seed 1 --duration 7200 --output reports/local/scenarios.json
./build/th08_scenario_cases --scenario all --strategy rolling-beam --seed 1 --duration 7200 --recover-goal 1 --output reports/local/recovery.json
./build/th08_scenario_cases --scenario relay --strategy rolling-beam --duration 72000 --recover-goal 1 --format tsv --output reports/local/long-relay.tsv
./build/th08_scenario_cases --scenario relay --strategy rolling-beam --visual-draws 0 --output reports/local/no-visual.json
./build/th08_spell_cases game_data_donottrack/th08.dat reports/local/id179 --verify-only
./build/th08_spell_cases game_data_donottrack/th08.dat reports/local/id179
./build/planner_bench > reports/local/planner_benchmark.json
```

ID179 verification checks exact events/terminal occupancy, RNG isolation, checkpoint
copy/resume and unknown-opcode rejection. The normal tool compares four strategies for
seeds 0, 1, 65535, storing twelve seed-qualified routes and a summary. It also checks illegal
and incomplete replay tapes. Raw assets are not extracted or committed.

The planner benchmark reference is the original sorting planner; its timing ratio
includes both heap selection and later early dedup improvements, not only the last
change. Report outcome/counters as well as time. Checked samples used Intel Xeon
Platinum 8573C, GCC 14.2, Release; timing is not portable or a guarantee.

## Exact input provenance

- DAT: 46,838,025 bytes, SHA256 `9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb`
- Component-oracle source: [N0zoM1z0/th08](https://github.com/N0zoM1z0/th08), pinned commit
  `a45e99fb1942714e6edded20847e32a654d56f97`
- Tracked native runtime: same upstream, branch `port/portable-64bit`, commit
  `02ff72f29e7b4928f21c5a2fb0f57ffa0cec5415`, adapted under `third_party/th08`.
  Earlier native reports identify the historical `861bec9` import and retain that profile
- Original preparation archive `TH08_AllCase_20260911.zip`: SHA256
  `8913fffc96824c27b681ff1b1133a4385f7c7ca3e8e377fede17e552e535b199`
- Older preparation reports mentioning a dirty worktree near `af72ca9` are historical
  evidence, not the pinned source or current runtime. `preparations/` remains unchanged

Legacy DAT analysis tools check the expected input hash. Native experiment tools
record the supplied DAT hash; compare evidence only for the stated input/profile.
Scenario reports retain relevant member/ANM/SHT hashes and instruction identities.
Source probe generators enforce per-file hashes before
extracting reviewed unchanged bodies; those maintained constants are the authoritative
file-hash list. Do not bypass a mismatch or use aggregate pass rates to hide it.

## Native headless reproduction and acceleration checks

The opt-in target requires SDL2, SDL2_image, SDL2_ttf and Fontconfig development
libraries (Debian/Ubuntu packages `libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
libfontconfig1-dev`). The CPU compatibility layer decodes resources; no video/audio
device or OpenGL renderer is initialized. No Python, Windows or Wine is used.

```sh
cmake -S . -B build-headless -DCMAKE_BUILD_TYPE=Release -DTH08_HEADLESS=ON -DTH08_HEADLESS_DAT="$PWD/game_data_donottrack/th08.dat"
cmake --build build-headless --parallel 2
ctest --test-dir build-headless --output-on-failure
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --spell-id 179 --difficulty 0 --seed 0 --strategy reactive --frames 2000 --actions reports/local/id179.actions --output reports/local/id179-native.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --spell-id 179 --difficulty 0 --seed 0 --frames 2000 --replay reports/local/id179.actions --output reports/local/id179-native-replay.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 195 --difficulty 4 --seed 0 --strategy spell-portfolio --frames 6000 --actions reports/local/id195.actions --output reports/local/id195-native.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 199 --difficulty 4 --seed 0 --strategy spell-portfolio --frames 5000 --actions reports/local/id199.actions --output reports/local/id199-native.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 201 --difficulty 4 --seed 0 --strategy spell-portfolio --frames 6000 --actions reports/local/id201.actions --output reports/local/id201-native.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 202 --difficulty 4 --seed 0 --strategy spell-portfolio --frames 6000 --actions reports/local/id202.actions --output reports/local/id202-native.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 204 --difficulty 4 --seed 0 --strategy spell-portfolio --frames 6000 --actions reports/local/id204.actions --output reports/local/id204-native.json
```

The real-data CTest runs Stage 1 to clear (22176 updates), ID179 through its original
wrapper/end (1292 updates, activation at 92) for seeds 0/1/65535, stationary collision
at 382, and retained/adapted boundaries for IDs 85, 89, 93, 198 and 201. IDs 85, 89,
93 and 198 complete for seeds 0/1/65535; ID201 preserves its generic WAIT collision at
362 and requires the seed-0 portfolio completion at 4292. Portfolio IDs 193/195/199
also run through their complete wrappers. ID202 retains its generic WAIT collision at
3265 and its WAIT-only portfolio collisions at 4204/4216/3613 for seeds 0/1/65535;
its original 12-update horizon and disabled future-ECL adapter are checked explicitly.
ID204 retains its generic relative-direction collision at 1205, requires complete
4832-update wrappers for seeds 0/65535, and preserves seed 1's collision at 3008.
ID32 preserves its generic 594 collision, checks the explicit 13-update/one-update-leg
profile and 192132 candidate evaluations for each complete 2372-update seed wrapper,
and runs Stage 3 through both its 7443 baseline collision and 39767 clear.
ID167 retains its generic 932 collision, requires all three 5492-update wrappers
with H12/four-update first leg and 444852 candidates. ID183 retains its generic 1041
collision and requires three complete 1292-update wrappers with H32/four-update leg
and 104652 candidates. It owns the single current continuous Stage6b gate: clear58853.
ID139 preserves its generic 5326 collision, requires all three 7292-update seed
wrappers, and runs Stage 6a through both its 48302 baseline collision and 61041 clear.
The native laser-item executable also checks 18 cancellation geometry cases, including
both source cancellation paths, angle quadrants, 32-unit spacing, exclusive end bounds,
item suppression and off-playfield X rejection.
A 10-update budget failure and the Stage 6b
reactive collision at update 854 remain covered.
Every execution tape replays in another process;
tests also reject unsupported input, excess tape and wrong-ID wrapper selection.
Generated tapes and `summary.json` are under `build-headless/headless-regression/`.
Without `TH08_HEADLESS_DAT`, these private-data tests are not registered.

The focused ID202 group can also run through the same generator, preserving its
baseline and all three failed seed wrappers rather than declaring the spell solved:

```sh
cmake -DEXECUTABLE="$PWD/build-headless/th08_headless" -DCOMPARE_EXECUTABLE="$PWD/build-headless-o0/th08_headless" -DDAT="$PWD/game_data_donottrack/th08.dat" -DWORK="$PWD/reports/local/id202-check" -DCASE_GROUP=id202 -P tests/headless_real_data.cmake
```

The 2026-10-02 Intel Xeon Platinum 8573C/GCC 14.2 run passed all 26 core tests and
the four focused ID202 scenes with fresh replay and native O0/O3 agreement. The normal
aggregate originally failed the unconditional historical Stage6b digest assertion:
this host produces 6279671846274327225, versus historical 4060221407534777929. Clean-main
and O0/O3 replay reproduce the current digest, while the 854/slot664 boundary, complete
collision object, RNG and feedback agree. The older profile did not identify solver
build/libm/CPU sufficiently to claim cross-environment float-bit equality. The precise
intermediate difference is not diagnosed.

The stage6b-semantic-v1 contract now guards DAT/profile/scene, terminal, RNG, feedback
and every collision field instead. It retains the old digest/report and emits
historical_digest_match plus numerical_profile_match=unverified; there is no compiler
whitelist or replacement digest. Strict current-run replay/O0/O3/diagnostic equality and
all 18 input-latch probes are unchanged. Negative DAT, RNG (including malformed fractional
integers), collision and replay mutations fail. This loses the unsupported historical
intermediate-trajectory oracle, so cross-host float drift outside the named boundary
is not proven absent. The historical aggregate remains an immutable provenance record.
`CASE_GROUP=id204` similarly runs the four ID204 fixtures and emits `id204-summary.json`.
Its baseline and seed-1 failure, as well as both complete seed wrappers, agree in fresh
processes and at native O0/O3 on this host. Both group selectors use the same scene/replay
checks as the full aggregate, whose historical digest remains provenance, with current-run equality still strict.

`CASE_GROUP=id32` runs the six ID32 wrapper/Stage3 cases and emits `id32-summary.json`.
All three seed wrappers and both baseline/complete stage tapes agree at O0/O3.
`CASE_GROUP=id167` now runs four wrapper fixtures and emits `id167-summary.json`.
The tracked five-case ID167 report is retained as preceding-checkpoint evidence,
including the then-genuine Stage6b ID183 collision. `CASE_GROUP=id183` runs five
current fixtures and emits `id183-summary.json`, including the new Stage6b clear.
All checked O3/O0 replays match; the full stage runs once in the aggregate.
For ID183 seed 0 on this GCC14 host, the complete 1292-update H32/81-path wrapper
uses 8894.1 ms of decision time; the failing 1041-update baseline uses 613.6 ms. These
different durations are not an equal-work performance comparison. Both exclude file
I/O and make the additional planning cost explicit.
`CASE_GROUP=id139` runs six baseline/complete scenes and emits `id139-summary.json`.
The checked report includes fresh O3 replay and separate O0 replay for every tape.
Pass `-DLASER_ITEMS_EXECUTABLE="$PWD/build-headless/th08_headless_laser_items"` to
include the focused native cancellation regression (the normal CTest already does).
An empty native `fsincos` stub previously caused undefined item geometry and an O0/O3
score difference at Stage 6a update 32432. Assigning both established sin/cos outputs
makes the long baseline and complete-stage tapes agree again: corrected digests are
`3641853114352264503` and `6502928606160698451`. The 18 geometry cases fail with both
pre-fix O0/O3 archives and pass after repair. This does not resolve or replace the
separate retained Stage 6b cross-host golden.

Reproduce the Stage 6b failure and compare the nine input directions locally:

```sh
mkdir -p reports/local/stage6b
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --difficulty 0 --seed 0 --strategy reactive --frames 50000 --actions reports/local/stage6b/baseline.actions --trace reports/local/stage6b/baseline.tsv --output reports/local/stage6b/baseline.json
./build-headless/th08_headless_probe --executable build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --difficulty 0 --seed 0 --actions reports/local/stage6b/baseline.actions --frame 854 --output-dir reports/local/stage6b/late
./build-headless/th08_headless_probe --executable build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --difficulty 0 --seed 0 --actions reports/local/stage6b/baseline.actions --frame 853 --through-frame 854 --output-dir reports/local/stage6b/early
```

The baseline intentionally exits 2. It hits fired, untransformed bullet slot 664:
player bounds `[373.174988,431.174988,374.825012,432.825012]` overlap hazard bounds
`[371.920898,428.774048,375.920898,432.774048]`. Movement consumes input 133 from
update 853, while update 854 samples 4165. Nine replacements at 854 all collide;
replacing 853 with left/up-left/down-left survives the observed update 854. The
real-data CTest guards these witnesses and verifies diagnostics leave the current
run’s trace unchanged (the historical 4060221407534777929 is separately labeled), and compares each branch tape at O0/O3 when
configured. It does not assert a whole-stage solution or an independent physics oracle.

The probe serializes its nine fresh processes and writes tapes, per-branch JSON/TSV
and `summary.json` into the output directory. It checks the projection at the common
prefix, rejects an out-of-tape frame and removes an old summary before a rerun. It
records actual total replayed updates, process wall time including initialization,
output and cleanup, and per-child maximum RSS in KiB. Local runs use approximately
73 MiB per child; use `ulimit -v 2097152` for a 2 GiB address-space cap if desired.
No remote host is needed. Repeated prefix work is explicit; no unsafe native snapshot
or shared action-dependent future is introduced to hide that cost.

The maintained seed-0 sweep enumerates 56 Easy standard/Extra practice checkpoints.
The Release `spell-portfolio` run completed 54 and retained 2 genuine collision
prefixes: bullet IDs 202/203. Bounded native-prefix repair additionally completes both,
so the combined documented recipe solves 56/56. All 56 rolling tapes
then agreed in a fresh process on the semantic fields used by `agree()` above. This is
broad algorithm evidence, while the smaller real-data CTest keeps the affected
transform/profile boundaries practical to rerun on every local change.
The ID204 change was compared with its preceding main checkpoint across all 56 cases:
the other 55 semantic reports and action tapes remained unchanged, and both versions'
tapes freshly replayed. Continuous Extra also remained at its earlier ID192 collision
on update 7424 at that historical checkpoint. The later ID192 change advances Extra below.

The ID139 comparison runs the previous portfolio and the new isolated profile on the
same corrected laser-item runtime: 49 versus 50 complete spells. All 55 unaffected
semantic records and literal action tapes agree; all 112 tapes freshly replay. Stage
6a changes from its preserved 48302 collision to a 61041 clear. Older aggregate
reports retain their original numerical/runtime profile rather than silently receiving
corrected item scores or digests.

The subsequent ID32 comparison changes only ID32 among 56 spells (50 to 51 clears):
all 55 unaffected semantic records and literal action tapes agree, and all 112 tapes
freshly replay. Continuous Stage3 changes from collision7443 to clear39767. The new
13-update horizon and 81 one-update-leg paths are isolated to ID32; generic projection
and tie-breaking are unchanged.

The subsequent ID167 comparison changes only ID167 among 56 spells (51 to 52 clears):
all 55 unaffected semantic records and literal action tapes agree, and all 112 tapes
freshly replay. Continuous Stage6b advances from ID167 at 42560 to the retained ID183
collision at 54401. This does not increase the stage-clear count.

The subsequent ID183 comparison changes only ID183 among 56 spells (52 to 53 clears):
all 55 unaffected semantic records and literal action tapes agree, and all 112 tapes
freshly replay. Continuous Stage6b advances from its preserved ID183 collision54401
to clear58853, adding the eighth complete stage. The prior ID167 report retains its
historical failure; the current single Stage6b fixture belongs to ID183.

The subsequent source/body comparison uses a complete separately compiled source snapshot
of preceding main 54f926d (original headers, Session, CLI and native units), rather than
an old-adapter/new-layout hybrid. It changes 53 to 54 clears, preserving all 53 successes.
Only ID192 and the still-failing ID203 prefix differ; 54 other semantic records and
literal tapes agree, including IDs201/202. Both versions of all 56 spells and Extra
freshly replay (114 tapes). Extra advances7424→67738, stopping in ID202.

ID192's focused five-record report preserves baseline 466, complete seeds 0/65535 at 3692,
seed 1 collision 1470 and actual Extra 67738. All agree with fresh O3 and O0 replays.
The H12/81-path profile costs 299052 candidates and 11074.8ms decision time for seed 0,
versus 4194 candidates/84.5ms for the 466-update baseline; these are unequal workloads.
At preceding checkpoint bd678, ID203's four-record report retained baseline 1711
and failures 1711/4823/2896 with bounded WAIT/vector projections; the current five-record
report below supersedes that partial evidence.
Native-linked regressions check WAIT/vector activation and final-clear boundaries,
DESPAWN's last lethal movement, 27 body source/lifecycle/clock guards, native movement
containment and the reciprocal-multiply size cancellation boundary. Both O0/O3 pass.
Independent native differential checks cover 23976 WAIT/vector updates and complete
ID192 tapes, including continuous Extra's delayed EndSpell-immunity transition.
That preceding bd678 normal CTest took 401.05s: 26 core tests passed; its native aggregate ended
at the unchanged historical Stage6b golden assertion described below, not a new gate.

The subsequent ID203 change keeps the rolling portfolio at 54/56 and adds one complete
seed 0 wrapper through bounded native-prefix repair. Compare against a complete separately
built bd678 source snapshot: the other 55 spell tapes/semantic results and continuous
Extra remain unchanged; the new rolling ID203 fails 787. Every comparison tape freshly
replays. The repaired 5492 tape is a separate verified outcome with full search costs,
not a replacement of the rolling failure record or a hardcoded route.
The final search used 91 candidates and 138198 native candidate updates, including 119908
replayed-prefix updates, in 384041ms. Initial execution used 787 updates/3919.03ms; the
built-in final replay used 5492 updates/875.446ms. Zero interrupted updates were charged.
The selected tape digest is 2771752472810487521 and independently agrees at native O0/O3.
The preceding f372 normal CTest took 799.97s and stopped at the old unconditional
Stage6b digest assertion. The latest expanded protocol suite
also passes separately. These host timings include process setup/artifacts and concurrent verification; they are
not the candidate policy's decision-only time or a performance guarantee.

The optional `headless_repair_protocol` test uses an explicitly synthetic child process
to reject malformed/duplicate/trailing JSON, wrong scene/prefix/hold/exit artifacts,
existing output directories and a TERM-ignoring timeout. It checks conservative charges
for semantically invalid children. Native policy tests check the pending-input step,
all nine first-action families, deterministic ties and 16533-expansion bound. The real-DAT
ID203 group checks baseline 1711, rolling 787 for three seeds, strict replay/prefix control,
and the complete bounded search with fresh O3/O0 replay. Its public JSON records include
child/repair executable SHA256 identities; `source_revision` remains the imported native
runtime provenance and does not identify a planner build. Historical numerical-profile limitations remain explicit under the v1 semantic contract.

`CASE_GROUP=id203` accepts `-DREPAIR_EXECUTABLE=$PWD/build-headless/th08_headless_repair`
to include the search (the normal CTest supplies it). The tracked five-case report
reuses that frozen successful search, then freshly replays its selected tape at O3/O0;
original search costs are retained. Optional `REPAIR_RESULT_DIR` requires both recorded
executable hashes to match and a complete result, and labels `search_reused`; fresh search
remains the default. Reproduce the complete solve with:

```sh
./build-headless/th08_headless_repair --executable build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 203 --difficulty 4 --seed 0 --frames 15000 --output-dir reports/local/id203-new-search
```

The output directory must not already exist. `summary.json` separates initial,
candidate/prefix and final-replay work; `processes.tsv` and `process-costs.json` retain
process outcomes and conservative unverified-update bounds. A budget-limited or malformed
run is not completion. All failed candidate reports/tapes remain available for diagnosis.

### Easy stage sweep

The 2026-10-02 local sweep used every supported stage entry, Easy, seed 0, the Release
O3 native profile and a 100000-update cap. Stage mode keeps
shooting enabled and advances original message scripts. Each execution wrote an action
tape and every tape agreed in a fresh process on the semantic fields and collision object
used by `agree()`:

```sh
mkdir -p reports/local/easy-stage-sweep
for stage in 1 2 3 4a 4b 5 6a 6b extra; do
  ./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage "$stage" --difficulty 0 --seed 0 --strategy spell-portfolio --frames 100000 --actions "reports/local/easy-stage-sweep/$stage.actions" --output "reports/local/easy-stage-sweep/$stage.json" || [ $? -eq 2 ]
  ./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage "$stage" --difficulty 0 --seed 0 --frames 100000 --replay "reports/local/easy-stage-sweep/$stage.actions" --output "reports/local/easy-stage-sweep/$stage-replay.json" || [ $? -eq 2 ]
done
```

| Stage | Outcome | Updates | Blocking spell | Decision ms | Execution ms | Peak bullets |
|---|---:|---:|---:|---:|---:|---:|
| 1 | complete | 24135 | - | 1828.4 | 4009.8 | 258 |
| 2 | complete | 32448 | - | 1581.4 | 3370.9 | 293 |
| 3 | complete | 39767 | - | 2195.0 | 3957.6 | 294 |
| 4a | complete | 22089 | - | 7354.6 | 11400.8 | 528 |
| 4b | complete | 43700 | - | 2675.4 | 5216.4 | 548 |
| 5 | complete | 43348 | - | 7025.9 | 10772.8 | 665 |
| 6a | complete | 61041 | - | 27644.8 | 33844.9 | 1201 |
| 6b | complete | 58853 | - | 22694.2 | 27796.8 | 1199 |
| extra | collision | 67738 | 202 | 32346.5 | 41409.4 | 1536 |

The corrected-runtime stage records, including the subsequent ID32 Stage3 and ID167/183
Stage6b and ID192 Extra reruns, cover 393119 updates and eight clears. All nine tapes freshly replayed.
These Intel Xeon Platinum 8573C/GCC 14.2 samples were run
alongside other verification; timings are workload records, not performance guarantees.
The earlier AMD EPYC/GCC12 five-clear sweep is superseded for current coverage. The
older 22176-update Stage 1 CTest uses the different `reactive` action tape and remains
a separate baseline.

Acceleration removes wall-clock waiting and presentation work while retaining every
original calc-chain update, timer increment and shared RNG consumer in that chain.
The virtual clock depends only on executed update count, never CPU throughput. Do not
accelerate by multiplying dt, subsampling actions/collisions, shortening spell timers,
or sharing action-dependent futures. These change the experiment's semantics.

Compare a second native library optimization level against the exact same tapes:

```sh
cmake -S . -B build-headless-o0 -DCMAKE_BUILD_TYPE=Release -DTH08_HEADLESS=ON -DTH08_HEADLESS_OPTIMIZATION=0
cmake --build build-headless-o0 --parallel 2 --target th08_headless
cmake -S . -B build-headless -DTH08_HEADLESS_COMPARE_EXECUTABLE="$PWD/build-headless-o0/th08_headless"
ctest --test-dir build-headless --output-on-failure -R '^headless_real_data$'
```

`TH08_HEADLESS_OPTIMIZATION` controls the imported game translation units (0..3),
not planner budgets or frame semantics. The checked GCC 12.2 O0/O3 tapes match all
reported semantic fields and every frame's diagnostic projection for these cases.
This checks acceleration within the native profile, not every spell or retail x87.
The original calc chain includes collision/feedback; the policy's extrapolation is
only a proposal, never the acceptance oracle.

Reports separate `simulation_ms` (calc-chain/queue updates minus timed native file
open/read/write/seek/stat/close wrappers), `file_io_ms`, `decision_ms` (observation and
policy/tape selection), `diagnostics_ms` (optional ring copies), and `execution_ms`
(whole loop including projection/trace work and I/O).
Initialization, DAT hashing, tape loading, output serialization and cleanup are outside
these loop measurements. CPU decoding during an update remains simulation work; compare
the same profile/tape. Run serially and record hardware/compiler/outcome/budget alongside
timing. The checked native host is Linux x86_64, AMD EPYC 7B12, GCC 12.2, Release.

Fresh-process replay checks execution agreement and RNG/feedback, not an independent
physics oracle. Original runtime globals are not yet branchable solver checkpoints.
Keep the scope in [Scenarios](SCENARIOS.md) when interpreting complete-scene reports.

## Optional pinned-source comparisons

Use a fresh ignored checkout directory; do not repurpose an existing working copy
that may contain local changes. Adjust the path below if `.cache/th08` already exists.

```sh
git clone https://github.com/N0zoM1z0/th08.git .cache/th08
git -C .cache/th08 checkout a45e99fb1942714e6edded20847e32a654d56f97
cmake -S . -B build-reference -DCMAKE_BUILD_TYPE=Release -DTH08_REFERENCE_SOURCE="$PWD/.cache/th08"
cmake --build build-reference --parallel 2
ctest --test-dir build-reference --output-on-failure
cmake --build build-reference --parallel 2 --target source_enemy_motion source_world_motion source_camera_particle source_spawn
./build-reference/source_spawn
```

The other three opt-in targets isolate comparisons already in `source_oracle`.
`source_spawn` separately compares 6000 allocation/post-store transactions with a
controlled immediate-ECL boundary. `source_effect_pool` checks effect allocation and
callbacks; passing DAT explicitly adds actual ANM/shared-pool cases. They do not execute
a complete original game world.

The source comparison chain covers collision predicates, all-seed RNG/selected ECL
random assignments, launch modes, fractional direction/acceleration clocks, transforms,
laser lifetime, slot selection, ANM scalar/control execution, enemy/world motion and
timeline behavior. Thin test adapters are scaffolding, not reconstructed complete actors.
Context layout/template ownership is also source-inspected and checked by native tests.

Native source equivalence is not retail executable, x87 or Direct3D equivalence. Fresh
replay shares the simulation implementation and therefore is not an independent physics
oracle. No original executable or running game was used for these results.

## Other retained tools and report maintenance

`th08_audit`, `th08_slices`, `th08_motion_cases`, `th08_animation_cases` and
`th08_timeline_cases` accept DAT and an output directory. They report structural or
restricted/component evidence, not whole spells. The two motion fixtures remain
useful regression baselines. `th08_first_spell` and entry/camera/effect tests are older
optional diagnostics, not mandatory solver checkpoints or the current roadmap.

Use `reports/local/` for experiments. Refresh only affected tracked outputs in
`reports/native/`, preserving their scope labels and input identities. Investigate
changed hashes, event counts, digests, routes or outcomes before publishing; timing-only
variation is expected. Preserve 64-bit digests as strings/integer-safe data.

The documentation CTest checks maintained local links/headings, selected report/status
counts and CMake's core-test count. It does not prove semantic correctness or inspect
historical preparations. Keep README, Status, Scenarios and generated evidence consistent.

Generated source-oracle translation units belong only in ignored build directories.
Their maintained generators share `source_probe_support.*`, not another tool's main().
Game DAT/EXE/assets are not distributed. Keep [third-party notices](THIRD_PARTY_NOTICES.md)
and the original preparation attribution; do not assign those artifacts a new license.

### Restored full verification (2026-10-03)

The H12/H32 repair adapter derives half-horizon holds from the selected failure's
homogeneous terminal unsafe run. It does not relabel ID202 as H32. The rebuilt
GCC14 O3/O0 run completes ID202 at 4712: 24 candidates, 102001 candidate updates,
100602 replayed-prefix updates, digest `7360258866840071988`. ID203 reproduces
5492 with 91 candidates and 138198 candidate updates. Both reports preserve their
rolling failures and independently replay at O0/O3. These two bounded repairs plus
the unchanged 54 rolling successes establish 56/56 seed-0 checkpoint recipes.
The fresh full CTest run passed 29/29 in 732.98 seconds, including native real-DAT,
O0 comparison and the historical Stage6b semantic-contract/18-probe checks.
Earlier timing paragraphs are historical measurements, not this run's timings.

The optional broad sweep now has a maintained producer. It records compact current
results, literal tape hashes and native executable identity while retaining the
full reports/tapes in the local output directory:

```sh
cmake -DEXECUTABLE="$PWD/build-headless/th08_headless" \
  -DDAT="$PWD/game_data_donottrack/th08.dat" \
  -DWORK="$PWD/reports/local/full-sweep" -P tests/headless_sweep.cmake
```

Its fixed 56-case rolling sweep and nine actual stage entries are independent of
the repair searches. A collision remains a collision in this report; adding two
isolated repair successes does not imply continuous Extra is complete.

A stage-sized Extra episode explicitly uses 144 candidates, 14400000 candidate
updates and 1800 search seconds, not the smaller isolated-spell budget. The first
fresh run used 9809223 candidate updates and 872386 ms of search, reaching ID203 at
69227 (`496550657910513129`, fresh O0 replay agrees). It is a failure, not stage clear.
To continue only an improved validated frontier without repeating earlier branches:

```sh
./build-headless/th08_headless_repair --executable "$PWD/build-headless/th08_headless" \
  --dat "$PWD/game_data_donottrack/th08.dat" --stage extra --difficulty 0 --seed 0 \
  --shoot 1 --frames 100000 --seconds 1800 --update-budget 14400000 \
  --resume-search "$PWD/reports/local/extra-episode1" \
  --resume-producer /path/to/exact/previous/th08_headless_repair \
  --output-dir "$PWD/reports/local/extra-episode2"
```

Keep the previous producer binary when rebuilding the search tool. Resume verifies
its hash, copies the selected artifacts, reconstructs the selected world afresh,
and reports prior/new/cumulative costs. An unchanged frontier is rejected rather
than silently starting the same finite proposal family again.

### Final seed-0 completion

The resumed Extra episode completes the actual carried-world stage at 92339,
ending after ID204, digest `15529738245795540729`. Its 87 candidates cost 6078636
native updates, with 6040785 repeated-prefix updates. Fresh source regeneration
used 69227 updates; final replay used 92339. Across both episodes the verified total
is 16117163 updates, with no interrupted uncertainty charge, and about 1591.15 seconds
of managed process wall time. Independent O0 replay agrees; those additional validation
costs and broad-sweep runs are separate from the managed episode totals.
The final integrated normal CTest passes 29/29 in 692.31 seconds.

After the rolling sweep and repaired searches, generate the current combined report:

```sh
cmake -DEXECUTABLE="$PWD/build-headless/th08_headless" \
  -DCOMPARE_EXECUTABLE="$PWD/build-headless-o0/th08_headless" \
  -DDAT="$PWD/game_data_donottrack/th08.dat" \
  -DSWEEP_DIR="$PWD/reports/local/full-sweep" \
  -DSPELL202_DIR="$PWD/build-headless/headless-regression/id202-repair" \
  -DSPELL203_DIR="$PWD/build-headless/headless-regression/id203-repair" \
  -DEXTRA_DIR="$PWD/reports/local/extra-episode2" \
  -DWORK="$PWD/reports/local/completion-verification" \
  -DOUTPUT="$PWD/reports/native/headless_current_summary.json" \
  -P tests/headless_completion.cmake
```

This verifies three repaired tapes afresh at O3/O0, checks binary/data identities,
and embeds the rolling failures, prior Extra search, hash-bound lineage, successful
searches and replay reports. The primary totals are 56 checkpoint recipes and nine
continuous stage recipes. The nested rolling-only totals remain 54/56 and 8/9.

### Multi-seed robustness baseline (2026-10-03)

The requested additional seeds were fixed before inspection: 1, 42 and 65535.
Each ran the same 56-checkpoint plus nine-continuous-stage rolling matrix and
frame caps, without automatic repair searches. All 195 fresh replays agree.
Use `-DSEED=<value>` with `tests/headless_sweep.cmake`; the default remains zero.

| Seed | Rolling spell clears | Rolling stage clears | Native updates including replay | Decision time, all 65 runs |
|---|---|---|---|---|
| 0 | 54/56 | 8/9 | 1160236 | 147.16 s |
| 1 | 51/56 | 7/9 | 1046086 | 113.64 s |
| 42 | 48/56 | 6/9 | 934060 | 100.44 s |
| 65535 | 50/56 | 5/9 | 916710 | 102.17 s |

These are rolling-only comparisons, not comparisons with seed zero's additional
repair wins. Earlier failed termination reduces work, so lower elapsed time is not
an efficiency improvement. Seed zero also ran alongside other verification;
these host timings are descriptive, not isolated performance benchmarks.

Retained spell failures (ID@update):
- Seed 1: 192@1470, 200@4161, 202@4216, 203@787, 204@3008
- Seed 42: 147@742, 192@727, 194@907, 200@1018, 201@2317, 202@4197, 203@787, 204@3008
- Seed 65535: 119@1511, 127@3964, 194@3236, 201@700, 202@3613, 203@787

Continuous-stage failures expose carried-world differences: ID127 in Stage6a
(seeds 1/65535), ID155 in Stage6b (42/65535), direct/imminent lasers in Stage4b
(42/65535), and different Extra failures. An isolated successful spell is not a
promise that the same spell clears inside a stage. ID201 seed 1 now completes on
this current native build; the earlier 3455 failure is historical evidence.

A diagnostic-only global H32 control, with the same nine focused constant paths,
avoids four local ID127/155 wall traps from identical replayed prefixes. It also
creates regressions: seed zero ID108/147 hit enemy bodies and ID175 hits an
unsupported polar transform. It is rejected as a general fix. No seed/frame or
new spell-ID branch is introduced to hide those failures. Seeds 7 and 2026 remain
reserved, uninspected holdouts until a justified candidate is frozen.


### Paused transform-contract experiment

The owner requested a tidy, published stopping point. The active source remains
at the reviewed body-observation/multi-seed checkpoint; the following experiment
was rejected rather than silently changing failure fixtures or tuning seeds.
The existing seed-zero completion report remains historical evidence for its
recorded native binary, not a newly regenerated search under this experiment.

Native-linked probes established three forecast defects:
- A VECTOR clearing at update 2 followed by a blocked POLAR produces x=2,4,8;
  the old unbounded vector projection gives 2,4,6. An allowed concurrent POLAR
  already changes the first update from predicted x=2 to actual x=4
- Native float recurrence gives x=8.480000496 at update 32 for vx=.1, ax=.01;
  the closed form gives 8.479999542. Exact float equivalence is not established
- WAIT timer 2 with the extra timer step and a subsequent POLAR gives x=1,2,6,
  whereas the nominal three-update linear certificate incorrectly gives x=1,2,3

The draft adds bounded VECTOR recurrence, explicit supported-clock checks and an
inactive-program linear certificate. It does not enable generic WAIT/POLAR or H32.
Focused core tests pass 26/26 and native bounds checks pass O3/O0 before the final
INT_MAX safe-prefix clamp; that final clamp has a focused O3 pass. The full normal
suite was **not** completed for this draft. Its old ID202 boundary assertion failed
because rolling collision moved from 4204 to 4212; it was not rebaselined.

The already-started four-seed, 65-route sweep completed with fresh replay agreement
for all 260 routes. Compared with the published rolling baseline:

| Seed | Draft spell clears | Draft stage clears | Published spell/stage clears |
|---|---|---|---|
| 0 | 54/56 | 8/9 | 54/56, 8/9 |
| 1 | 51/56 | 7/9 | 51/56, 7/9 |
| 42 | 47/56 | 6/9 | 48/56, 6/9 |
| 65535 | 49/56 | 5/9 | 50/56, 5/9 |

New failures are ID195 seed42 at 2752 and ID192 seed65535 at 2631. The unchanged
seed-zero bounded repair budgets still complete ID202 (4712 updates, six candidates,
25764 candidate updates) and ID203 (5492 updates, 77 candidates, 85144 candidate
updates), with built-in fresh replay. Independent O0 comparison of these draft
repair tapes was not run. No repaired Extra search or new completion claim follows.

[Compact results](../reports/experiments/transform-contract-rejected.json) record
binary/report hashes, failures, costs and verification limits. The accompanying
[archived patch](../reports/experiments/transform-contract-rejected.patch) applies
to fdc8f8fb0ccd4119e7c9cf0b17b5eb84961a9337, but is not compiled or applied.
The sweep binary precedes its final INT_MAX clamp; that edge change must not be
mistaken for a fully swept final binary. Do not apply it without integration review
and the same regression matrix. No DAT, extracted assets or action tapes are included.

Remaining work is bounded and separate: fix the demonstrated transform contracts
without losing successful scenes; cover lethal body motion only with valid
lifecycle/child/timeline certificates; improve the two late laser warnings without
hardcoded frames or unsupported geometry. Global H32 remains rejected. Holdout
seeds 7 and 2026 remain uninspected, and work is paused until requested again.
