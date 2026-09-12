# th08-solver

Offline analysis, modeling and solving components for Touhou 08: Imperishable Night
1.00d. Maintained code, tools and tests use **C++17**, with no Python dependency.
Performance and readability are requirements; numerical shortcuts need independent
correctness evidence. The current phase does not launch the game.

**Complete offline spell solutions: 0.** All 317 DAT members are decoded and all
222 original spell IDs are indexed, but complete worlds are not implemented.
Two source-driven 600-frame particle fixtures have independently replayed routes;
they are not complete spell solutions.

## Read the project

| Document | Purpose |
|---|---|
| [Current status](docs/STATUS.md) | Implemented layers, exact baselines and missing integration |
| [Architecture and methods](docs/ARCHITECTURE.md) | Design rationale, code map, state ownership and algorithm contracts |
| [Complete-coverage roadmap](docs/COVERAGE.md) | All-case acceptance, dependencies and the first complete-world target |
| [Validation and reproduction](docs/VALIDATION.md) | Test profiles, pinned-source oracle, DAT audits and report refresh |
| [Documentation guide](docs/README.md) | Specialized evidence, performance, regression and maintenance references |

The first complete-world target is Wriggle IDs 2..5 through their actual practice
entry, followed by separately verified stage inheritance. The all-spell objective
also requires the remaining indexed families and their relevant entry/state variants.

## Build and run

Requires CMake 3.16+, a C++17 compiler and OpenSSL development libraries. Linux is
tested; other platforms and retail x87 numerical equivalence are not verified.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure

./build/th08_audit game_data_donottrack/th08.dat reports/local/review
./build/th08_slices game_data_donottrack/th08.dat reports/local/review
./build/th08_motion_cases game_data_donottrack/th08.dat reports/local/review
./build/th08_animation_cases game_data_donottrack/th08.dat reports/local/review
./build/th08_timeline_cases game_data_donottrack/th08.dat reports/local/review
```

Supply your own DAT under the ignored `game_data_donottrack/` directory. Native
tools verify its hash and write reports without extracting game assets to disk.
The default ANM audit supplies no RNG; random instructions stop for missing context.
Default CI uses neither private DAT nor the optional reconstruction checkout.

For ASan/UBSan, pinned reconstruction setup and independent component-oracle targets,
follow [Validation](docs/VALIDATION.md) and [Build artifacts](docs/BUILD_ARTIFACTS.md).
Do not enable fast-math. Use `reports/local/` for experiments; refresh tracked
`reports/native/` intentionally using the documented verification procedure.

## Repository map

| Path | Purpose |
|---|---|
| `include/th08/`, `src/` | Current parsing, execution, motion, geometry and planner components |
| `tools/` | Native audit/report tools and hash-checked source-oracle generators |
| `tests/` | Unit, ownership, source-comparison, regression and documentation checks |
| `benchmarks/` | Reproducible component performance comparisons |
| [reports/native/](reports/native/README.md) | Current generated structural/execution baselines and scoped measurements |
| [docs/](docs/README.md) | Current design, status, acceptance criteria and evidence |
| `preparations/` | Original materials preserved unchanged; not current implementation status |
| `.cache/`, `build*/`, `reports/local/` | Ignored reference checkout, generated build files and experiments |

Maintained documentation and code are English; original preparation artifacts retain
their original language and attribution. See [Provenance](docs/PROVENANCE.md) and
[Third-party notices](docs/THIRD_PARTY_NOTICES.md) for the evidence/distribution boundary.
