# th08-solver

Offline C++17 simulation and planning for Touhou 08. The immediate goal is to run
complete, reproducible spell/scenario segments without starting the game, expose
algorithm failures, and improve measured solve cost.

## Current direction

- Start from an explicit spell/scenario checkpoint. Menus, practice preludes,
  camera and rendering are not prerequisites
- Replace identified visual RNG consumers with seeded, recorded hooks when useful;
  label this a controlled profile. Keep gameplay RNG and action-dependent state honest
- Test whole continuous scenarios, not only short safe horizons. Carry bullets,
  RNG and relevant actor state across transitions; replay executed actions from scratch
- Compare simple baselines and alternative planners under stated budgets. Search
  exhaustion is not a proof of impossibility, and a short route is not a full solution

The tracked TH08 runtime now runs complete real-data scenes natively on Linux without
a display, Wine or a real-time frame limiter. A spell-aware hazard portfolio completes
45 of the 56 enumerated Easy/Extra spell checkpoints at seed 0, while the simpler
reactive policy still provides a stable Stage 1 baseline. Fresh-process action replay
checks every sweep result. Existing controlled/synthetic planners remain available for
algorithm comparisons. See [current results and remaining work](docs/STATUS.md).

**Legacy subset complete offline spell solutions: 0.** Historical subset-engine reports
retain that count. The native headless profile separately verifies 45 complete spell
checkpoints in the portfolio sweep and one complete stage, including native
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

Each step still executes one original calc-chain update. Acceleration removes waiting;
it does not enlarge time steps or skip gameplay frames. [Validation](docs/VALIDATION.md)
includes complete-spell, replay and optimization comparison commands.

## Documentation map

| Document | Single responsibility |
|---|---|
| [Engineering rules](AGENTS.md) | Scope, code quality, tests and change discipline |
| [Status](docs/STATUS.md) | Verified progress, limits and next useful work |
| [Architecture](docs/ARCHITECTURE.md) | Code map, ownership and correctness contracts |
| [Scenarios](docs/SCENARIOS.md) | Checkpoints, RNG policy, exact profiles and terminal semantics |
| [Validation](docs/VALIDATION.md) | Reproduction, input provenance and evidence boundaries |
| [Report index](reports/native/README.md) | Generated evidence and its producer |
| [Third-party notices](docs/THIRD_PARTY_NOTICES.md) | Required attribution/license |

Old entry-first roadmaps were consolidated or removed; Git history retains them.
`preparations/` is preserved original research, not current instructions or verified
coverage. Game assets, generated binaries/source and experiments stay ignored.
