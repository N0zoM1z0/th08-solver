# th08-solver

> [!NOTE]
> **Native score attack: Extra 3,308,826,050 · complete Lunatic 6,653,194,550.**
> Scarlet Team, No Miss / No Bomb, with verified exported replays.
> Targets: **4B Extra · 7B complete Lunatic**, using stage/spell strategy composition. Scoring is paused after the latest batch handoff.

![Wine and Native shadow planning loop, animated schematic by Claude.](docs/media/shadow-mpc-online.gif)

C++17 Touhou 08 solver. Capture the real Wine seed, search Native trajectories for
the declared no-miss/bomb/capture goal and higher scores, then export `.rpy` files
for retail replay validation. Scoring uses source attack timing, item collection
and form control. Bomb permission and capture requirements are independent
[CLI controls](docs/VALIDATION.md#bomb-and-capture-constraints).
[How it works](docs/PLANNER.md#true-rng-shadow-runtime).

## Verified Native routes

Best published complete runs by route and shot. All are **NMNB** (No Miss,
No Bomb); captures are optional. Lunatic covers **1 → 2 → 3 → 4A → 5 → Final B**.
Each score links to its exact replay.

| Route | Shot | Score | Captures |
| --- | --- | ---: | ---: |
| Extra | Scarlet — Sakuya/Remilia | **[3,308,826,050](reports/native/replays/scarlet/extra/extra-scarlet-native-nmnb-3308826050-seed9098.rpy)** | 14/14 |
| Extra | Magic — Marisa/Alice | [3,013,913,970](reports/native/replays/magic/extra/extra-native-nmnb-3013913970-seed46997.rpy) | 14/14 |
| Complete Lunatic | Scarlet — Sakuya/Remilia | **[6,653,194,550](reports/native/replays/scarlet/lunatic/story-scarlet-native-nmnb-4a-finalb-6653194550-seed32796.rpy)** | 36/37 |
| Complete Lunatic | Magic — Marisa/Alice | [5,550,599,270](reports/native/replays/magic/lunatic/story-native-nmnb-4a-finalb-5550599270-seed1805.rpy) | 36/37 |

These have original-entry fresh execution and natural replay agreement, with
player name `ysopwnab`. Scarlet's story times out card 10; Magic's times out card 47.
The separate Magic **all-capture** story is
[5,107,553,670, 37/37](reports/native/replays/magic/lunatic/story-native-nmnb-4a-finalb-5107553670-seed1805.rpy).
[All selected progress replays](reports/native/README.md) preserve original seeds,
input lineage, verification tasks and hashes, including earlier records.

The October 27 [all-shot human references](https://maribelhearn.com/wr#IN) are
**3,178,460,830 Extra** and **7,053,332,460 Lunatic**, both solo Youmu.
Native Extra is 130,365,220 above its human reference; complete Lunatic is
346,805,450 below the next 7B milestone. Historical same-shot references remain in
[Status](docs/STATUS.md). All shots may be explored; Scarlet has priority.

## Original Wine results

These are the completed original-game validations. Newer Native scores above
have their own verification scope and do not inherit an earlier retail result.

| Route | Shot | Execution | Score | Captures |
| --- | --- | --- | ---: | ---: |
| Extra | Scarlet | Exported replay playback | [3,179,193,860](reports/native/replays/scarlet/extra/extra-scarlet-native-nmnb-3179193860-seed9098.rpy) | 14/14 |
| Complete Lunatic | Magic | Online controller | 2,189,965,070 | 33/37 |
| Extra | Magic | Online controller | 1,166,969,620 | 14/14 |

All three are NMNB. Scarlet playback uses no online controller; its exact score
and all 14 captures pass original Wine checks 1343/1367.
[Validation and provenance](docs/VALIDATION.md#scarlet-team-verified-native-baselines)
· [Original recordings with game audio](https://drive.google.com/drive/folders/11x9m65sysbsQZZYyHvNJS0-LQ3u2cOAu?usp=drive_link).
Online solving is paused. Scoring experiments use offline Native planning;
the owner-requested handoff also leaves that queue stopped.

## Follow the exploration

The [stage/spell search journal](docs/experiments/2026-10-28-stage-spell-search.md)
records source/human replay analysis, solo Sakuya trials, complete combinations
and failures, including Native-reward-ranked survival gains and early-road income negatives.
The [experiment index](docs/experiments/README.md) links every task's parameters,
actual bombs/captures, costs and provenance. Local phase scores become route
records only after complete execution and fresh replay checks.
[Composition/search issue 12](https://github.com/N0zoM1z0/th08-solver/issues/12)
tracks the approach; [Status](docs/STATUS.md) owns current work and next steps.

## Start here

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

This default build covers the asset-free libraries and tests. Enable
`TH08_HEADLESS` for the Native game tools as described in
[Validation](docs/VALIDATION.md#native-headless-reproduction-and-acceleration-checks).
Some record-setting scene profiles remain frozen local experiments; the supplied
replays can be checked without reproducing their private search.

Requires CMake 3.16+, C++17 and OpenSSL development libraries. Native game execution
also needs SDL2/image/ttf, Fontconfig and your own ignored `th08.dat`.
[Build, replay verification and scoring commands](docs/VALIDATION.md).

| Document | Purpose |
| --- | --- |
| [Experiment journal](docs/experiments/README.md) | Per-task hypotheses, parameters, provenance, costs, failures and decisions |
| [Status / handoff](docs/STATUS.md#operational-resume-contract) | Current state, experiment rules, resource limits and resumption plan |
| [Planner](docs/PLANNER.md) | Policy mechanisms, Native safety and online protocol |
| [Validation](docs/VALIDATION.md) | Reproduction, selected replay provenance and tests |
| [Architecture](docs/ARCHITECTURE.md) · [Scenarios](docs/SCENARIOS.md) | Code ownership, checkpoints and terminal contracts |
| [Evidence index](reports/native/README.md) | Published historical artifacts and their producers |
| [Engineering rules](AGENTS.md) · [Third-party notices](docs/THIRD_PARTY_NOTICES.md) | Contribution and attribution rules |

[Extra scoring issue](https://github.com/N0zoM1z0/th08-solver/issues/11) · [Online story milestone tag](https://github.com/N0zoM1z0/th08-solver/tree/lunatic-nmnb-4a-finalb-20261024)

The [legacy subset catalog](reports/native/archive/README.md#legacy-subset-catalog)
has a separate scope from Native scoring. New run artifacts and preparations stay
local; selected replays are published with explicit provenance.
