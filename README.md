# th08-solver

> [!NOTE]
> **Native score attack: complete Lunatic 6,653,194,550 · Extra 3,308,826,050**
> (Scarlet Team, No Miss / No Bomb, verified exported replays).
> Targets: 7 B complete Lunatic (above the 7,053,332,460 all-shot human reference) and 4 B Extra.

A C++17 offline score-attack solver for Touhou 08 (Imperishable Night). It runs the
real game code headlessly, searches input sequences whose every committed step is
backed by a complete true-RNG Native future, and exports ordinary `.rpy` replays.

1. **Seed**: read the RNG seed from the owned retail game in Wine at the
   pre-RegisterChain boundary, then keep it fixed for the episode.
2. **Search**: `th08_shadow_style` runs the game one input at a time; a heuristic
   policy proposes candidates and forked children verify each one over a complete
   horizon with the true RNG ([Planner](docs/PLANNER.md)).
3. **Verify**: re-execute the tape in a fresh process and play the exported replay
   naturally; both must match the producer exactly ([Validation](docs/VALIDATION.md)).

## Records

| Route | Shot | Score | Captures |
| --- | --- | ---: | ---: |
| Complete Lunatic 1→2→3→4A→5→Final B | Scarlet (Sakuya/Remilia) | **[6,653,194,550](reports/native/replays/scarlet/lunatic/story-scarlet-native-nmnb-4a-finalb-6653194550-seed32796.rpy)** | 36/37 |
| Extra | Scarlet (Sakuya/Remilia) | **[3,308,826,050](reports/native/replays/scarlet/extra/extra-scarlet-native-nmnb-3308826050-seed9098.rpy)** | 14/14 |
| Complete Lunatic | Magic (Marisa/Alice) | [5,550,599,270](reports/native/replays/magic/lunatic/story-native-nmnb-4a-finalb-5550599270-seed1805.rpy) | 36/37 |
| Extra | Magic (Marisa/Alice) | [3,013,913,970](reports/native/replays/magic/extra/extra-native-nmnb-3013913970-seed46997.rpy) | 14/14 |

The Scarlet Extra 3,179,193,860 replay also passed complete retail playback in Wine.
All selected replays are indexed in [reports/native](reports/native/README.md);
uploads are on [Silent Selene](https://www.silentselene.net/replays/user/Kitauji).

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

That covers the asset-free libraries and tests. Add `-DTH08_HEADLESS=ON` (and
`-DTH08_HEADLESS_DAT=path/to/th08.dat` for the real-data tests) to build the game
runtime and scoring tools. Requires CMake 3.16+, a C++17 compiler and OpenSSL; the
Native build also needs SDL2, SDL2_image, SDL2_ttf, Fontconfig and your own `th08.dat`.

## Documents

| Document | Purpose |
| --- | --- |
| [Status](docs/STATUS.md) | Current results and workspace state |
| [Roadmap](docs/ROADMAP.md) | Diagnosis of the score gap and the ordered plan for the next work |
| [Scoring](docs/SCORING.md) | TH08 score mechanics from source and the whole-run model |
| [Lessons](docs/LESSONS.md) | What the first campaign tried, what worked and what did not |
| [Planner](docs/PLANNER.md) | Search loop, proposals, phase plans, goals and checkpoints |
| [Architecture](docs/ARCHITECTURE.md) | Code map, ownership and update-order contracts |
| [Validation](docs/VALIDATION.md) | Build, seed acquisition, run and verification commands |
| [Experiment records](docs/experiments/README.md) | Raw per-task ledger and results |
| [Engineering rules](AGENTS.md) · [Third-party notices](docs/THIRD_PARTY_NOTICES.md) | Contribution rules and attribution |
