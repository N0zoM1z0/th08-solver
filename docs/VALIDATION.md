# Validation, provenance and reproduction

Core CTest cases: 25

Normal core tests require no game assets or reconstruction. Optional source comparisons
add two CTests (27 total). A native build with private DAT adds one full-scene CTest.
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
```

The real-data CTest runs Stage 1 to clear (22176 updates), ID179 through its original
wrapper/end (1292 updates, activation at 92) for seeds 0/1/65535, stationary collision
at 382, a 10-update budget failure and the Stage 6b reactive collision at update 854.
Every execution tape replays in another process;
tests also reject unsupported input, excess tape and wrong-ID wrapper selection.
Generated tapes and `summary.json` are under `build-headless/headless-regression/`.
Without `TH08_HEADLESS_DAT`, these private-data tests are not registered.

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
real-data CTest guards these witnesses, verifies diagnostics leave the original
`4060221407534777929` trace unchanged, and compares each branch tape at O0/O3 when
configured. It does not assert a whole-stage solution or an independent physics oracle.

The probe serializes its nine fresh processes and writes tapes, per-branch JSON/TSV
and `summary.json` into the output directory. It checks the projection at the common
prefix, rejects an out-of-tape frame and removes an old summary before a rerun. It
records actual total replayed updates, process wall time including initialization,
output and cleanup, and per-child maximum RSS in KiB. Local runs use approximately
73 MiB per child; use `ulimit -v 2097152` for a 2 GiB address-space cap if desired.
No remote host is needed. Repeated prefix work is explicit; no unsafe native snapshot
or shared action-dependent future is introduced to hide that cost.

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
