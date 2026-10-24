# th08-solver

C++17 simulation, offline planning and current-memory CPU control for Touhou 08. The target is continuous
No Miss No Bomb runs across all teams/solo shots, standard difficulties and Final
A/B routes, plus Extra, delivered as input tapes and real `.rpy` files.
The fork-based native planner completes seed-0 Lunatic Final A under the documented
native source profile, with fresh action and natural replay verification. One repaired
seed-0 Final A replay also clears NMNB in retail 1.00d under Wine. The full replay
fidelity gate remains 106/108. [Planner](docs/PLANNER.md) owns the approach.

The first frozen unseen-seed batch adds 16 complete native-profile NMNB routes:
eight randomly drawn seeds, both generic profiles, each through a separately
budgeted exact-prefix continuation. All 16 original 2400-second attempts exhausted
their budgets; the [seed ledger](reports/native/planner_seed_summary.json) retains
them and all continuation/validation costs. This sample does not prove every seed
or add retail validation for these new files.

The [retail-tested story replay](reports/native/replays/story-v2--r64-b384-c16-retail-1.00d.rpy) and
[full action tape](reports/native/replays/story-v2--r64-b384-c16.actions) are tracked.
The [retail audit](reports/native/retail_story_validation.json) records the original
export-header failure, identity-only repair and actual complete playback. The
[original native export](reports/native/replays/story-v2--r64-b384-c16.rpy) stays unchanged.
The [campaign ledger](reports/native/planner_current_summary.json) records their
configuration, hashes, verification and retained search costs.

## Current direction

The owner now authorizes a [true-RNG shadow runtime and Wine planning loop](https://github.com/N0zoM1z0/th08-solver/issues/8).
Common-start synchronization, true-RNG branch verification and a forward-only
Wine action bridge are implemented. After six complete stage-practice clears,
the frozen client completes an actual Lunatic story NMNB with Marisa/Alice
(shot 1), through natural 4A to Final A: real seed33280, 231737 calculations and
five original stage handoffs. The game continues while native branches plan;
the live owner never rewinds or waits for a search. Fresh Native following of
all 231742 actual receipts agrees. Average gameplay rate is 58.565 FPS under the
recorded software-rendering profile; strict per-frame deadlines and broader seed
reliability remain open in [Status](docs/STATUS.md). Numeric digest drift stays
visible, and meaningful gameplay/RNG/lifecycle fields remain checked. Stage 4B
is deferred by the owner. This separate native-lookahead profile
preserves the existing current-memory controller's zero-future-update contract.
The [October 24 preparation](preparations/10.24-with-claude-gpt/PUBLICATION.json)
preserves audited public research copies and identities of unchanged local originals.

[Issue #9](https://github.com/N0zoM1z0/th08-solver/issues/9) records the execution flow,
resource evidence and deployment options. The owner keeps the current Wine setup
and requests a complete Stage 4A practice recording with real game audio first;
full-story recording and Windows migration wait for that viewing review.

The existing [memory-based online controller](https://github.com/N0zoM1z0/th08-solver/issues/5)
tracks standalone-stage coverage, action-dependent failures and the
offline-teacher/student loop. Its historical 4B/Extra work and natural-4A captures
remain separate from the current shadow profile. Reading current
process memory is permitted; image recognition is not required. `th08_online` consumes
owned current observations without future native steps or rewinds. Its first harness
measures latency and deadline misses but waits for each decision, so completion does
not establish a 60 FPS controller.
An optional deterministic planner-delay model reuses checked owned plans and
reports executor/search timing separately. Its searches still run synchronously;
[Planner](docs/PLANNER.md) defines the latch, cancellation and timing contracts.
The bounded [issue #6](https://github.com/N0zoM1z0/th08-solver/issues/6) experiment
adds a current-memory motion hypothesis, reproducible native branch labels and a
small C++-trained value ranker. Its frozen comparison finds an encouraging spell
signal with regressions and uncertainty; the complete Stage 5 sample passes all
three arms. [Status](docs/STATUS.md) owns the counts, costs and evidence limits.
Selected compact [learned artifacts](models/README.md)
are preserved with exact identities and provenance; raw training/run data stay local.
The next frozen comparison completes all 172 trials. On 24 new Extra 202 seeds,
the opt-in generic forecast/search profile completes 23/24, baseline 14/24 and
each compact value model 9/24. These are spell proxies; the options and models
remain opt-in. Exact decoder/surface acceleration preserves the replay gate.
A separate wall-paced executor exposes Native stage-initialization costs as well
as decision deadlines; [Status](docs/STATUS.md) records the paired results, retained
failure and remaining full-story timing limit.

Two of three historical Lunatic Final A stories through natural 4A strictly verify
under the documented uniform current-memory controller profile. Story diagnosis
retains a Stage 5 failure; [Status](docs/STATUS.md) owns the exact version and
validation. These captures do not establish a current-main controller completion.
The owner-requested [seed-0 replay](reports/native/replays/online-story-4a-v1--s0.rpy)
and [seed-29768 replay](reports/native/replays/online-story-4a-v1--s29768.rpy), full
action tapes and [publication audit](reports/native/online_story_4a_validation.json)
are tracked. Both unchanged files now complete NMNB in unmodified retail 1.00d
under Wine, with matching scores and natural replay-menu return; [Status](docs/STATUS.md)
records the retained observer-mask failure and manifest-derived audit. These are
playback checks, without an enforced 60 FPS controller or current-main story claim.

The preceding random-seed evaluation is complete: all 12 diagnosis and 12 held-out
cases pass fresh native/replay checks under the frozen offline solver. This finite
24-seed sample is separate from online capability. The seed-0 coverage run was
stopped after all 36 double-team combinations completed; four active solo attempts
were interrupted. The [108-case catalog](reports/native/planner_route_matrix.json)
remains the coverage reference. Its [original protocol](reports/native/planner_route_protocol.json)
retains the baseline profiles, unlock history and bounded continuation costs.
The [published route ledger](reports/native/planner_route_summary.json) retains a
24-combination snapshot. Apart from the explicitly requested online 4A captures above,
new replays, action tapes and experiment JSON stay in
ignored `reports/local/`; [Status](docs/STATUS.md) owns current verified coverage.
The report checks complete stage paths against the manifest and counts distinct
combinations across retries. The first sparse random batch freezes 24 independent
seeds with one complete run each: 12 diagnosis cases and 12 held-out cases, rotating
all shots through Lunatic and Extra. Diagnosis runs first; a checked phase gate
freezes the evaluated version and budgets before releasing holdouts. An incomplete
or invalid diagnosis case requires review. [Planner](docs/PLANNER.md) owns the method.

The native runtime is both forecaster and collision oracle. Forked rollouts record
native hazards; beam search proposes paths; fresh native rollouts verify each
committed prefix. Checkpoint processes own complete branch state for backtracking.
Approximation belongs to proposal search. Runtime failures never count as safety.
The earlier kernels and spell-specific policies below remain historical baselines.

- Start from an explicit spell/scenario checkpoint. Menus, practice preludes,
  camera and rendering are not prerequisites
- Replace identified visual RNG consumers with seeded, recorded hooks when useful;
  label this a controlled profile. Keep gameplay RNG and action-dependent state honest
- Test whole continuous scenarios, not only short safe horizons. Carry bullets,
  RNG and relevant actor state across transitions; replay executed actions from scratch
- Compare simple baselines and alternative planners under stated budgets. Search
  exhaustion is not a proof of impossibility, and a short route is not a full solution

The earlier tracked TH08 runtime ran complete real-data scenes natively on Linux without
a display, Wine or a real-time frame limiter. The rolling hazard portfolio completes
54 of the 56 enumerated Easy/Extra spell checkpoints at seed 0; bounded fresh-process
native-prefix repair adds IDs202/203 for 56/56. The rolling portfolio clears
Easy Stages 1, 2, 3, 4a, 4b, 5, 6a and 6b continuously from their explicit stage starts.
Two bounded native-search episodes additionally clear continuous Extra at 92339 updates.
Fresh-process action replay checks every sweep result. Existing controlled/synthetic
planners remain available for algorithm comparisons. See
[current results and remaining work](docs/STATUS.md).

**Legacy subset complete offline spell solutions: 0.** Historical subset-engine reports
retain that count. The native headless profile separately verifies 56 complete spell
checkpoints with the documented portfolio/repair recipe and all nine stage entries, including native
graze/score/item feedback. Its platform and numerical profile are explicit; retail
Windows equivalence is not established.

## Take over locally

Requires CMake 3.16+, a C++17 compiler and OpenSSL development libraries. Linux
x86_64 is tested; portability and retail x87 equivalence are not established.

```sh
git clone https://github.com/N0zoM1z0/th08-solver.git
cd th08-solver
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
mkdir -p reports/local
./build/th08_scenario_cases --scenario all --strategy rolling-beam --recover-goal 1 --output reports/local/scenarios.json
```

Place your own `th08.dat` in ignored `game_data_donottrack/`, then:

```sh
./build/th08_spell_cases game_data_donottrack/th08.dat reports/local/id179
```

For the native runtime, also install the SDL2, SDL2_image, SDL2_ttf and Fontconfig
development packages. These support CPU resource decoding; headless execution creates
no window or audio device. Build and run a complete stage:

```sh
cmake -S . -B build-headless -DCMAKE_BUILD_TYPE=Release -DTH08_HEADLESS=ON -DTH08_HEADLESS_DAT="$PWD/game_data_donottrack/th08.dat"
cmake --build build-headless --parallel 2
ctest --test-dir build-headless --output-on-failure
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 1 --strategy reactive --frames 30000 --actions reports/local/stage1.actions --output reports/local/stage1.json
```

Each Session step calls the original calc chain. Replay fast-forward can restart that
chain internally, so API calls and recorded gameplay calculations are counted separately.
Acceleration removes waiting without enlarging gameplay time steps.
[Validation](docs/VALIDATION.md) includes complete-spell, replay and optimization commands.

## Documentation map

| Document | Single responsibility |
|---|---|
| [Engineering rules](AGENTS.md) | Scope, code quality, tests and change discipline |
| [Planner](docs/PLANNER.md) | Native branching, verification and experiment protocol |
| [Status](docs/STATUS.md) | Verified progress, limits and next useful work |
| [Architecture](docs/ARCHITECTURE.md) | Code map, ownership and correctness contracts |
| [Scenarios](docs/SCENARIOS.md) | Checkpoints, RNG policy, exact profiles and terminal semantics |
| [Validation](docs/VALIDATION.md) | Reproduction, input provenance and evidence boundaries |
| [Report index](reports/native/README.md) | Generated evidence and its producer |
| [Third-party notices](docs/THIRD_PARTY_NOTICES.md) | Required attribution/license |

Old entry-first roadmaps were consolidated or removed; Git history retains them.
`preparations/` preserves historical research, not current instructions or verified
coverage. Public handoff copies redact private links/paths; the
[offline preparation manifest](preparations/10.21-with-claude-important/PUBLICATION.json)
and [online preparation manifest](preparations/10.22-with-claude-online/PUBLICATION.json)
record original/public hashes, with exact inputs preserved locally. The online
handoff ZIP retains its source patch, research notes and analysis scripts; generated
models, results, replays and action tapes remain in the complete local extraction.
Historical analysis scripts are not a maintained runtime dependency. Game assets,
generated binaries/source and new experiments stay ignored.
