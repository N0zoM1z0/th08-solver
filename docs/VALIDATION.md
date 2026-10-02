# Validation and reproduction

Reviewed: 2026-09-12. The current suite adds owned practice-entry prefix tests,
a real DAT first-blocker diagnostic and an opt-in source spawn-order comparison.
Existing broad component/report baselines remain unchanged by this integration.
No game, port, controller or Python process is needed.

## Current verification profiles

Core CTest cases: 25

| Profile | Expected suite | What it establishes |
|---|---|---|
| Native build without reconstruction | 25 core tests | Unit, parser, ownership, deterministic regressions and documentation checks; no private game data |
| Release with pinned reconstruction | 27 tests: core plus `source_oracle` and `source_effect_pool` | The above plus extracted native source-body comparisons |
| Debug with ASan/UBSan, without reconstruction | 25 core tests | Instrumented core behavior and ownership |
| Opt-in component source oracles | Four independently built executables | Enemy motion, world motion, camera particles and spawn ordering; not extra default CTests |
| Native DAT audit tools | Explicit commands below | Real-data structural and restricted execution baselines, plus two model routes |

The public [CI workflow](../.github/workflows/ci.yml) runs `RelWithDebInfo` with
sanitizers OFF and ON, without private DAT or `TH08_REFERENCE_SOURCE`. A green CI
run does not establish DAT, optional source-oracle or complete-spell validation.
ASan/UBSan options are currently wired for non-MSVC compilers only.

## Documentation maintenance baseline

The review follows the current user request and [engineering contract](../AGENTS.md).
The earlier corrections below are documentation/report maintenance, not gameplay
coverage. The separate practice-entry implementation does not invalidate or upgrade
those historical component baselines into complete worlds.

| Required element | Established evidence and correction | Result |
|---|---|---|
| Current, reviewable state | Replaced the chronological status list and conflicting next-step queue with a capability ledger and one coverage roadmap | Current baseline stated explicitly |
| Accurate scope | Corrected the source report's predicate-only label; distinguished per-regression exclusions from separately implemented transform coverage | Minor wording drift corrected; no new execution claim |
| Reproducible evidence | Reran native tests/audits and refreshed report samples; linked exact producers and input identities | Component evidence remains independently checkable |
| Historical integrity | Kept preparation archives, regression counterexamples, comparison baselines and ignored scratch untouched | No loss of historical evidence |
| Explicit unfinished work | Recorded missing world owner, spawn/child/effect/player/damage/terminal integration and all-case acceptance | Complete coverage remains unverified, not implicitly passed |

The main risk of stale wording is overstating world completion or confusing old
timings with current measurements. The proportionate correction is precise labels,
shared document ownership, refreshed evidence and a regression check, not deleting
useful historical baselines. A behavior change, new terminal-world proof or failing
reproduction requires reassessing the corresponding row and coverage claim.

## Core and instrumented builds

```sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DTH08_REFERENCE_SOURCE=
cmake --build build-core --parallel 2
ctest --test-dir build-core --output-on-failure

cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug \
  -DTH08_SANITIZERS=ON -DTH08_REFERENCE_SOURCE=
cmake --build build-sanitize --parallel 2
ctest --test-dir build-sanitize --output-on-failure

clang-format --dry-run --Werror include/th08/*.hpp src/*.cpp tools/*.cpp \
  tests/*.cpp tests/*.hpp benchmarks/*.cpp
```

Use distinct build directories for distinct profiles. Clearing the reference option
above avoids retaining an older CMake cache setting unintentionally. An optional
reference build adds two CTests, not a different core suite.

## Pinned native source oracle

Prepare a separate checkout if it does not already exist; do not overwrite an
existing working copy or discard local changes:

```sh
git clone --filter=blob:none --no-checkout https://github.com/N0zoM1z0/th08.git .cache/th08
git -C .cache/th08 checkout --detach a45e99fb1942714e6edded20847e32a654d56f97
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DTH08_REFERENCE_SOURCE="$PWD/.cache/th08"
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/source_oracle reports/native/source_oracle.json
```

The native generator verifies file hashes, extracts unchanged bodies into ignored
build translation units, and supplies narrow adapters. The comparison chain and
excluded behavior are in [Provenance](PROVENANCE.md). Original-source warnings are
not suppressed by changing the reference bodies. An adapter pass establishes only
its declared native profile, not retail executable equivalence.

Independent Release and sanitizer component targets are documented in
[Build artifacts](BUILD_ARTIFACTS.md). Their generators and cases are tracked;
regeneration must not depend on a previous temporary translation unit.

## Refresh real-data baselines

Run these serially after the build/tests, with the hash-pinned DAT supplied locally:

```sh
./build/th08_audit game_data_donottrack/th08.dat reports/native
./build/th08_slices game_data_donottrack/th08.dat reports/native
./build/th08_motion_cases game_data_donottrack/th08.dat reports/native
./build/th08_animation_cases game_data_donottrack/th08.dat reports/native
./build/th08_timeline_cases game_data_donottrack/th08.dat reports/native
./build/timeline_tests game_data_donottrack/th08.dat
./build/practice_entry_tests game_data_donottrack/th08.dat
./build/th08_first_spell game_data_donottrack/th08.dat .cache/th08 reports/native
./build/th08_animation_cases game_data_donottrack/th08.dat reports/local/anm-seed0 0
./build/th08_animation_cases game_data_donottrack/th08.dat reports/local/anm-seed65535 65535
./build/geometry_bench reports/native/geometry_benchmark.json
./build/planner_bench reports/native/planner_benchmark.json
./build/bullet_slots_bench reports/native/bullet_slots_benchmark.json
```

The slice tool also writes the emitter microbenchmark. Seeds in the two extra ANM
profiles are reset independently per script; they must not replace the unseeded
baseline or be presented as world RNG state. Repeat relevant DAT tools with the
sanitizer binaries into `reports/local/`, keeping their slower measurements separate.

Review diffs before committing refreshed reports. A timing-only change is expected
to vary; a changed member hash, event digest, route, payload or status requires an
explanation or regression. Existing deterministic TSV baselines remain unchanged
by the new entry-prefix report. See the [report index](../reports/native/README.md).

The first-spell diagnostic checks DAT/member and thirteen relevant source hashes, then
runs a supplied-gate prefix with no RNG/player defaults. Its process exit0 means
report generation succeeded; its JSON status remains `MISSING_ENTRY_STATE`.
The entry tests also check masks1/2/4/8 on the native DAT. Repeat both commands with
sanitizer binaries into `reports/local/`; their new deterministic reports must match.
The opt-in `source_spawn` comparison checks6000 spawn transactions against unchanged
SpawnEnemy1/2 bodies with controlled immediate-ECL outcomes, not complete source RunEcl.

## Keep claims current

The C++ `documentation` test reads only maintained Markdown, selected tracked JSON
counts and the core CTest count supplied by CMake. It checks local inline link
targets, empty leaf headings, current spell counts and the declared suite size.
It deliberately does not crawl external URLs, inspect historical preparations,
validate Markdown anchors, or infer semantic correctness from matching numbers.

Before committing, check that Status, Architecture, Coverage and report scope labels
agree with the code. Preserve the distinction between structural parsing, restricted
execution, fixture routes, complete worlds and original-game validation. Unknown
effects, context and RNG consumers remain blockers, not undocumented NOPs.

## Effect51 integration checks

[Effect51 entry](EFFECT51_ENTRY.md) describes the focused source comparison and
DAT regression commands. `source_effect_pool` is a second optional default CTest
when the pinned reconstruction is supplied. Pass DAT explicitly to also compare
the actual script73 time-zero projection. The core suite has24 tests and the
reference suite26. Run `effect_entry_tests th08.dat` for supplied ID2 Easy state; the existing
`practice_entry_tests th08.dat` retains four-mask missing-context coverage.
The first-spell command now also writes `first_spell_effect51_summary.json` for a
clearly labeled supplied-state prefix; it is not an actual initialized-world run.

`practice_camera_tests th08.dat` checks the source-derived Stage1 camera, including
freeze, non-unit refusal, retargeting and the first world-effect stop. The updated
`source_effect_pool th08.dat` additionally compares512 mixed effect51/62 allocation
calls. The supplied first-spell checkpoint now has12 background62 slots plus16
ECL effect51 slots; RNG remains256 draws at the supplied immediate-ECL boundary.

## Continuous controlled runs

`scenario` is one focused CTest covering phase/RNG continuity, a complete 7200-frame
route, seeded repeat/replay and a finite-horizon failure with an escape witness.
Use [Continuous scenarios](SCENARIOS.md) for CLI reproduction and scope. It requires
no DAT. Record outcomes separately from runtimes; search exhaustion is an expected
benchmark result, not a successful solution or proof that the scene is impossible.
