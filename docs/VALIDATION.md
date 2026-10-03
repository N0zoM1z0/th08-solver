# Validation, provenance and reproduction

Core CTest cases: 26

Normal core tests require no game assets or reconstruction. Optional source comparisons
add two CTests (28 total). A native build with private DAT adds one full-scene CTest.
Existing public CI runs sanitizer OFF/ON without DAT/source;
it does not establish real-DAT scenario success. Keep new verification proportional to
the changed behavior; no elaborate sanitizer infrastructure is required.

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
  `861bec908b84fa4658382d7526e5a0075f520846`, adapted under `third_party/th08`
- Original preparation archive `TH08_AllCase_20260911.zip`: SHA256
  `8913fffc96824c27b681ff1b1133a4385f7c7ca3e8e377fede17e552e535b199`
- Older preparation reports mentioning a dirty worktree near `af72ca9` are historical
  evidence, not the pinned source or current runtime. `preparations/` remains unchanged

DAT tools check the expected input hash. Scenario reports retain relevant member/ANM/SHT
hashes and instruction identities. Source probe generators enforce per-file hashes before
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
prefixes: bullet IDs 202/203. Bounded native-prefix repair additionally completes ID203,
so the combined documented recipe solves 55/56. All 56 rolling tapes
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
