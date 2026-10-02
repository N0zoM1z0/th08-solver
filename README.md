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

The verified handoff includes 7200/72000-frame synthetic runs, a real-DAT controlled
ID179 Easy survival segment, and a geometry-derived recovery for a reproduced beam
search failure. See [current results and remaining work](docs/STATUS.md).

**Complete offline spell solutions: 0.** This is the conservative source-faithful
whole-world coverage count. The separately verified controlled ID179 survival
profile omits graze/score/item feedback and retail visual RNG ordering; it is not
an original full-stage or practice-capture equivalence claim.

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

The owner is taking over local development. Automated TH08 feature work is paused
at this handoff; the next steps below are recommendations, not background tasks.

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
