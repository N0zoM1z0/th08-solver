# Validation and reproduction

## Build and tests

```sh
# Asset-free libraries and contracts (CI runs this, with and without sanitizers).
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure

# Native runtime, scoring tools and real-data regressions (needs your th08.dat).
cmake -S . -B build-native -DCMAKE_BUILD_TYPE=Release -DTH08_HEADLESS=ON \
  -DTH08_HEADLESS_DAT=$PWD/game_data_donottrack/th08.dat
cmake --build build-native --parallel 2
ctest --test-dir build-native -j2 --output-on-failure
```

Native execution needs SDL2, SDL2_image, SDL2_ttf and Fontconfig development
packages plus OpenSSL. The Wine observer needs `i686-w64-mingw32-g++` and
`-DTH08_WINE_SHADOW=ON`.

The Native suite has 24 tests and takes about 160 s with two jobs (November 15,
2026). The real-data tests cover: a complete small scoring run with fresh and
natural checks (`headless_shadow_style`), refinement and bomb-permission variants
that reuse it, the source capture audit on a complete story replay, true-RNG
branch/queue contracts and the common-start follower on a fixed spell 179 tape
(`tests/data/`), the capture-goal rejection fixture, and route/laser-item session
checks. Asset-free tests cover the policy, phase plans, frontier selection, wire
protocol, rollout/child contracts, replay-suite contract and this documentation
(links, anchors and prose spacing).

## Seed acquisition (Wine)

Every scoring episode starts from a seed read from the real game. Launch the owned
Wine retail game with the observer at the **pre-RegisterChain** boundary (the same
boundary at which a Native `Session` initializes) and with no Native producer:

- `TH08_SHADOW_UPDATES=1`, `TH08_SHADOW_OBSERVE_REPLAY=0`, `TH08_SHADOW_ADVANCE_DIALOGUE=1`
- `TH08_SHADOW_CHECKPOINT=8,4,1,-1,1` for Extra or `0,3,1,-1,1` for the story;
  the checkpoint selects the entry and never writes RNG
- `TH08_SHADOW_PRIOR_FINAL_A_CLEAR=1` for team stories (Final B available), `0` otherwise

After menu confirmation the observer writes `start.txt` atomically and exits after
one observed update. Check its fields, the pinned retail hash, launcher exit 0 and
the single 228-byte row, then bind the receipt hash into `native-entry.json`. A
failed startup is not a receipt. **Never choose or reroll a seed after viewing
Native scores**; all candidates of an episode reuse its seed.

| Episode (`reports/local/retail-seed-episodes-*`) | Shot | Seed | Entry | Seed receipt SHA-256 |
| --- | --- | ---: | --- | --- |
| `20261027/scarlet-extra-entry-v6` | Scarlet Team (2) | 9098 | Extra, stage 8 / difficulty 4 | `44955955273c6e1ad3f5f2ddc74eef01776243d80f5dc098ef9db27bff247fb0` |
| `20261027/scarlet-story-4a-finalb-entry-v1` | Scarlet Team (2) | 32796 | Stage 1 Lunatic, prior Final A clear | `ab5faf14bc8578a873f012b2560803bc277139ba822879b4d4c677fb9e68f638` |
| `20261026/extra-entry-v4` | Magic Team (1) | 46997 | Extra | `fcecb629fffb3a7973d37da14b73c6c61250db6e38a80e4995d4bee738a93c92` |
| `20261026/story-4a-finalb-entry-v1` | Magic Team (1) | 1805 | Stage 1 Lunatic, prior Final A clear | `88516cae49c9c04a2ebc0857f88eb81b3d0ed7277ff4c4c95a1a8b39d55ae133` |
| `20261028/sakuya-extra-entry-v1` | Solo Sakuya (8) | 53346 | Extra | `c583381fa5e4c50dd9474ced1e586abdd6f9f3c18b3c2d3ec12174dcb5faf2ec` |
| `20261028/sakuya-story-4a-finalb-entry-v3` | Solo Sakuya (8) | 24238 | Stage 1 Lunatic, natural Final B | `092e5c95fc986054633dc48b298cd64a6334f5f128b22e052b189614179f56b7` |

The observer and launcher binaries pinned for these receipts have SHA-256
`9647fef617cfef1611296cd4eb611c399db6e20aab439f027f91c07ead5bb24e` and
`2fdeee516adf32dd14b682256969edca63e06537be8f2c95920b239a9565ade1`.

## Running a candidate

Positional arguments are
`DAT OUTPUT_DIR STAGE DIFFICULTY SHOT SEED STYLE MAX_UPDATES STORY WALL_SECONDS`
(`STAGE` is the raw runtime index: 0 for a story start, 8 for Extra). The story
record's final producer, for example, ran:

```sh
th08_shadow_style th08.dat OUT 0 3 2 32796 score-harvest 220000 1 1800 \
  --allow-bombs 0 --require-all-captures 0 --verify-captures 0 \
  --held-recovery 1 --baseline-recovery 1 --recovery-fire capture \
  --search-budget-ms 30000 --verify-horizon 128 --publish-horizon 64 \
  --lead-updates 1 --nominal-warm-updates 1 --replan-updates 32 \
  --policy-horizon 24 --policy-beam 64 --economic-proposals 4 --economic-movement 3 \
  --priced-collection 1 --upper-stage 1 --upper-flow 1 --collection-bias 5 \
  --human-bias 5 --survival-bias 5 --attack-pacing 1 --farm-charge-fire 1 \
  --capture-factor 1.2 --capture-padding 120 --hit-income 1 --drop-target 1 \
  --drop-follow 1 --parent-chain-bias 100 --target-score 7000000000 \
  --prefix-actions PARENT/plan.actions --prefix-updates 156833 --checkpoint-updates 1024
```

The output directory must not exist. It receives `result.json`, `plan.actions`,
`native.rpy` (complete runs only) and `score-audit.json`. Exit 0 is a complete run
at or above `--target-score`; exit 5 is complete but below target; exit 2 is a
declared boundary (`--stop-after-card`) or budget; other exits are failures.
Always read `result.json` (`outcome`, `bombs_used`, captures, per-stage results);
a wrapper's exit code is not enough.

Run owners under `prlimit --as=4294967296` and `nice`, at most two at a time on
this host, and abort below 8 GiB host `MemAvailable` (see [AGENTS.md](../AGENTS.md)).

## Verification gate

A complete candidate becomes a route result only after both checks:

```sh
# Fresh: re-execute the exact tape from the original entry in a new process.
th08_shadow_follow --dat th08.dat --actions OUT/plan.actions --output fresh.json \
  --stage 1 --story 1 --difficulty 3 --shot 2 --seed 32796 --prior-final-a-clear 1 \
  --input-profile sampled-gameplay --frames 400000
# Natural: play the exported replay through the original replay path.
th08_replay_check th08.dat OUT/native.rpy natural.rows --score-audit natural-audit.json
```

(`--stage extra --difficulty 4 --prior-final-a-clear 0` for Extra.) Require:
fresh `outcome` complete with a terminal projection identical to the producer;
natural `finished` and `natural`; natural end score equal to the producer's raw
score; identical capture outcomes; zero death and bomb rows; player name bytes
`79736f70776e6162` (`ysopwnab`). These checks share one implementation and
prove determinism, not retail physics.

## Records and selected replays

| Route | Score | Captures | Seed | Owner inputs | Replay |
| --- | ---: | --- | ---: | ---: | --- |
| Scarlet Extra | 3,308,826,050 | 14/14 | 9098 | 76,725 | [rpy](../reports/native/replays/scarlet/extra/extra-scarlet-native-nmnb-3308826050-seed9098.rpy) |
| Scarlet complete Lunatic | 6,653,194,550 | 36/37 (card 10 timeout) | 32796 | 159,445 | [rpy](../reports/native/replays/scarlet/lunatic/story-scarlet-native-nmnb-4a-finalb-6653194550-seed32796.rpy) |

[progress.tsv](../reports/native/replays/progress.tsv) is the identity table for all
51 selected progress replays (hashes, producer, constraints). Only a fresh-verified
improvement of the overall Extra or complete-Lunatic record adds a new file; add it
with `git add -f` and a `progress.tsv` row. To check a record replay:

```sh
th08_replay_check th08.dat \
  reports/native/replays/scarlet/lunatic/story-scarlet-native-nmnb-4a-finalb-6653194550-seed32796.rpy \
  story.rows --score-audit story-audit.json
```

Replay rows also allow a replay to be converted back into an owner input tape
for continuation; see [Status](STATUS.md#resuming-from-the-records).

Native exports report slowdown as NaN: headless stepping never runs the FPS path
that fills the lag counters, so the writer divides 0/0. Replay sites show `nan%`.
Gameplay playback ignores this field.

## Retail playback

Qualified replays are played in the owned retail 1.00d game under Wine with the
observer in replay-only mode, which records a 228-byte row per calculation and
writes no gameplay state. Export identity is fixed: version `0100d`, executable size
`840704`, checksum `2724749753`. The Scarlet Extra 3,179,193,860 replay passed
complete retail playback on October 27, 2026: score, zero deaths and bombs, and all
14 capture flags observed across 72,214 callbacks. The newer Native records have
not been played in retail yet.
