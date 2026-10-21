# th08-solver

Offline C++17 simulation and planning for Touhou 08. The target is a continuous
Lunatic story No Miss No Bomb run, delivered as an input tape and a real `.rpy`.
The fork-based native planner is being integrated and verified; full-story success
is not yet established. [Planner](docs/PLANNER.md) owns the approach.

## Current direction

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
`preparations/` is preserved original research, not current instructions or verified
coverage. Game assets, generated binaries/source and experiments stay ignored.
