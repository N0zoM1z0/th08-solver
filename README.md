# th08-solver

> [!NOTE]
> **Lunatic NMNB story clear: 1 → 2 → 3 → 4A → 5 → Final B (Kaguya).**
> Marisa/Alice, played online in Wine with game audio. Extra also clears NMNB online.
> **Native NMNB scores: Extra 3,087,585,240 (Scarlet, 14/14); Lunatic story 6,439,636,250 (Scarlet, 36/37).**
> Selected progress: [Extra 3.088B replay](reports/native/replays/scarlet/extra/extra-scarlet-native-nmnb-3087585240-seed9098.rpy) · [Story 6.440B replay](reports/native/replays/scarlet/lunatic/story-scarlet-native-nmnb-4a-finalb-6439636250-seed32796.rpy) · [All replays and provenance](reports/native/README.md) · [Videos](https://drive.google.com/drive/folders/11x9m65sysbsQZZYyHvNJS0-LQ3u2cOAu?usp=drive_link)

![Wine and Native shadow planning loop, animated schematic by Claude.](docs/media/shadow-mpc-online.gif)

C++17 Touhou 08 solver. Capture the real Wine seed, search Native trajectories for
the declared no-miss/bomb/capture goal and higher scores, then export `.rpy` files
for retail replay validation. Scoring uses source attack timing, item collection
and form control. Bomb permission and capture requirements are independent
[CLI controls](docs/VALIDATION.md#bomb-and-capture-constraints).
[How it works](docs/PLANNER.md#true-rng-shadow-runtime).

| Marisa/Alice result | Our NMNB score | Human reference¹ | Captures |
| --- | ---: | ---: | ---: |
| Online Lunatic 4A → Final B | 2,189,965,070 | Alan: 6,168,365,790 | 33/37 |
| Online Extra | 1,166,969,620 | ASL: 3,021,639,920 | 14/14 |
| **Native Extra** | **3,013,913,970** | ASL: 3,021,639,920 | **14/14** |
| **Native Lunatic 4A → Final B** | **5,550,599,270** | Alan: 6,168,365,790 | **36/37** |
| Native story, all captures | 5,107,553,670 | Alan: 6,168,365,790 | 37/37 |

The latest Native results have fresh execution and natural replay agreement;
their player name is `ysopwnab`. The Magic story best times out47; the
all-capture best retains37/37. The owner-selected progress replays, including the
newer Scarlet and Magic improvements, are published by shot and route. Their exact identities are in
[Validation](docs/VALIDATION.md#october27-verified-native-progress).
The requested Native milestones, Extra above3B and complete Lunatic above5B,
are achieved. The owner now targets the highest human scores across all12 shots:
Extra above3,178,460,830 and Lunatic above7,053,332,460, first reaching6.5B on the
complete Lunatic route. These October27 [all-shot records](https://maribelhearn.com/wr#IN)
are both solo Youmu; the table above preserves the earlier Magic Team references.
All shots may now be explored, with the owner's preferred **Scarlet Team
(Sakuya/Remilia)** taking priority. Its verified Native results are
**Extra3,087,585,240 (14/14)** and **complete Lunatic6,439,636,250 (36/37)**,
both No Miss/No Bomb, at their own original retail seeds9098/32796.
Scarlet now owns both overall Native records; its story times out10.
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
| [Experiment journal](docs/experiments/README.md) | Per-task hypotheses, parameters, provenance, costs, failures and decisions |
| [Status / handoff](docs/STATUS.md#operational-resume-contract) | Current state, experiment rules, resource limits and resumption plan |
| [Planner](docs/PLANNER.md) | Policy mechanisms, Native safety and online protocol |
| [Validation](docs/VALIDATION.md) | Reproduction, selected replay provenance and tests |
| [Architecture](docs/ARCHITECTURE.md) · [Scenarios](docs/SCENARIOS.md) | Code ownership, checkpoints and terminal contracts |
| [Evidence index](reports/native/README.md) | Published historical artifacts and their producers |
| [Engineering rules](AGENTS.md) · [Third-party notices](docs/THIRD_PARTY_NOTICES.md) | Contribution and attribution rules |

[Extra scoring issue](https://github.com/N0zoM1z0/th08-solver/issues/11) · [Online story milestone tag](https://github.com/N0zoM1z0/th08-solver/tree/lunatic-nmnb-4a-finalb-20261024)

Legacy subset complete offline spell solutions: 0. This is separate from Native
runtime results. New run artifacts and preparations stay local; selected replays
are published with explicit provenance.
