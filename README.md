# th08-solver

Offline analysis, modeling, and solving components for Touhou 08: Imperishable Night 1.00d.
The current phase does not launch the game. Maintained components, tools, and tests use
**C++17**, with no Python build or runtime dependency. Performance and readability
are both requirements: contiguous data, explicit ownership, reproducible benchmarks,
and independent reference comparisons.

## Build and run

Requires CMake 3.16+, a C++17 compiler, and OpenSSL development libraries.
Linux is tested; other platforms have not been verified.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure

./build/th08_audit game_data_donottrack/th08.dat reports/native
./build/th08_slices game_data_donottrack/th08.dat reports/native
./build/geometry_bench reports/native/geometry_benchmark.json
```

Supply your own DAT under the ignored `game_data_donottrack/` directory.
Tools verify its SHA-256 before analysis. They write summaries, source indices,
and execution statuses, without extracting game assets to disk.

## Current capabilities

- Decode all 317 archive members and hash each decoded payload.
- Parse 24 ECL files, 1,449 subprograms, and 36,661 nonterminal instructions;
  validate 2,182 jump targets.
- Preserve 431 spell-start occurrences and all 222 original spell IDs without
  merging main-game, practice, or difficulty sources.
- Attempt restricted scalar scheduling for every subprogram under five difficulty
  masks and three alignment overrides: 21,735 combinations.
- Verify real Wriggle sub40/41/42 scheduling, request counts, and ordered emission digests.
- Query bullet and laser geometry using an owned, contiguous spatial index;
  propose finite-horizon beam paths and replay them against an unindexed reference.

Complete ECL worlds, enemy/bullet lifecycles, ANM, damage, RNG consumption chains,
and complete spell routes remain unimplemented. `RETURNED_SLICE` means a restricted
subprogram returned; `BOUNDED_PREFIX` means the requested horizon was reached.
**Verified complete spell solutions: 0.** The planner currently uses synthetic
collision-window fixtures, not a complete Reisen spell.

## Repository map

| Path | Purpose |
|---|---|
| `include/th08/` | Resource, emitter, geometry, and planner interfaces |
| `src/` | Native DAT/ECL parsing and restricted scheduling |
| `tools/` | Native auditing, matrix execution, and source-oracle generation |
| `tests/` | Boundary, ownership, differential, and planner contract tests |
| `benchmarks/` | Repeatable performance experiments with explicit scope |
| `reports/native/` | Generated indices, execution matrices, benchmarks, and oracle results |
| `docs/` | Architecture, evidence, status, and implementation sequence |
| `preparations/` | Original conversations, reports, and archives, tracked unchanged |
| `.cache/` | Ignored local reference source and historical experiments |

Start review with [Architecture](docs/ARCHITECTURE.md), [Status](docs/STATUS.md),
and [Provenance](docs/PROVENANCE.md). Historical Python files remain inside the original
preparation archives; maintained tools neither import nor invoke them. Maintained
documentation and code are English. Original source materials retain their original language.

## Optional reconstruction-source oracle

```sh
git clone --filter=blob:none --no-checkout https://github.com/N0zoM1z0/th08.git .cache/th08
git -C .cache/th08 checkout --detach a45e99fb1942714e6edded20847e32a654d56f97
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DTH08_REFERENCE_SOURCE="$PWD/.cache/th08"
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/source_oracle reports/native/source_oracle.json
```

The native generator verifies the complete hashes of two reference source files,
then extracts unmodified collision functions into the build directory. This comparison
does not launch the game or establish original x87/Windows bitwise equivalence.

## Development checks

```sh
clang-format --dry-run --Werror include/th08/*.hpp src/*.cpp tools/*.cpp tests/*.cpp benchmarks/*.cpp
cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug -DTH08_SANITIZERS=ON
cmake --build build-sanitize --parallel 2
ctest --test-dir build-sanitize --output-on-failure
```

Do not enable `-ffast-math`: changing floating-point semantics has not been justified.
Performance measurements do not replace correctness comparisons.
