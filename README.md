# th08-solver

> [!NOTE]
> **Lunatic NMNB story clear: 1 → 2 → 3 → 4A → 5 → Final B (Kaguya).**
> Marisa/Alice, played online in Wine with game audio. Extra also clears NMNB online.
> **Native Extra score milestone: 2,564,311,730, NMNB and 14/14 captures.**
> [Highest Extra replay](reports/native/replays/extra-native-nmnb-2564311730-seed19732.rpy) · [Story replay](reports/native/replays/shadow-recording-v3--story-4a-finalb-d3-s1-seed40755.rpy) · [Videos and replays](https://drive.google.com/drive/folders/11x9m65sysbsQZZYyHvNJS0-LQ3u2cOAu?usp=drive_link)

![Wine and Native shadow planning loop, animated schematic by Claude.](docs/media/shadow-mpc-online.gif)

C++17 Touhou 08 solver. A Native shadow follows actual Wine inputs, forks game
states with their own RNG, and returns verified future inputs while Wine plays.
Scoring uses source-directed attack timing, item collection and form control.
[How it works](docs/PLANNER.md#true-rng-shadow-runtime).

| Marisa/Alice result | Our NMNB score | Human reference¹ | Captures |
| --- | ---: | ---: | ---: |
| Online Lunatic 4A → Final B | 2,189,965,070 | Alan: 6,168,365,790 | 33/37 |
| Online Extra | 1,166,969,620 | ASL: 3,021,639,920 | 14/14 |
| **Native Extra, frozen prototype** | **2,564,311,730** | ASL: 3,021,639,920 | **14/14** |

The Native best has fresh execution and natural replay agreement. Main's complete
new-seed result is 2,453,582,120; final-spell reliability and online scoring remain
open. The Native best is 84.86% of ASL's reference.
¹ [Human score-attack records](https://maribelhearn.com/wr#IN), 2026-10-25 snapshot;
ASL's replay uses one bomb. [Status](docs/STATUS.md) owns the
results, failed trials, costs, unresolved problems and proposed next steps.

## Start here

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

Requires CMake 3.16+, C++17 and OpenSSL development libraries. Native game execution
also needs SDL2/image/ttf, Fontconfig and your own ignored `th08.dat`.
[Build, replay verification and scoring commands](docs/VALIDATION.md).

| Document | Purpose |
| --- | --- |
| [Status / handoff](docs/STATUS.md) | Current evidence, experiments, problems and resumption plan |
| [Planner](docs/PLANNER.md) | Policy mechanisms, Native safety and online protocol |
| [Validation](docs/VALIDATION.md) | Reproduction, selected replay provenance and tests |
| [Architecture](docs/ARCHITECTURE.md) · [Scenarios](docs/SCENARIOS.md) | Code ownership, checkpoints and terminal contracts |
| [Evidence index](reports/native/README.md) | Published historical artifacts and their producers |
| [Engineering rules](AGENTS.md) · [Third-party notices](docs/THIRD_PARTY_NOTICES.md) | Contribution and attribution rules |

[Extra scoring issue](https://github.com/N0zoM1z0/th08-solver/issues/11) · [Online story milestone tag](https://github.com/N0zoM1z0/th08-solver/tree/lunatic-nmnb-4a-finalb-20261024)

Legacy subset complete offline spell solutions: 0. This is separate from Native
runtime results. New run artifacts and preparations stay local; selected replays
are published with explicit provenance.
