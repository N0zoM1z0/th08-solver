# Validation, provenance and reproduction

Current scoring mechanisms are in [Planner](PLANNER.md#spell-capture-and-score-objective);
[Status](STATUS.md) owns outcomes, unresolved work and proposed experiments.
This page owns reproducible commands, exact artifact identities and test contracts.
Historical component commands below retain their named build/profile assumptions.

## Native score-first workflow

The owner selected Native exploration for proper Magic Team Extra and complete
Lunatic4A→FinalB. Require NMNB,14/14 or37/37 captures and a displayed score
strictly above3,021,639,920 or6,168,365,790 respectively before retail Wine replay
validation. Live Wine solving is paused. Native budgets are explicit offline
compute limits; they need not meet an online frame deadline.

New scoring exports default to `ysopwnab`, the owner's selected eight ASCII
bytes. The original `ReplayData::playerName[8]` is followed by reserved and
practice-mode bytes; longer strings are rejected instead of overwriting mode.
`--replay-name` accepts1..8 identifier bytes. The checker reports the eight bytes
as `player_name_hex` (`79736f70776e6162` for this name), avoiding arbitrary binary
or legacy-encoded text in JSON. Historical selected replay bytes remain unchanged.

Use `--require-all-captures 1 --target-score DISPLAYED` for the active campaign.
The report separates game completion, capture coverage, exported replay and
`objective_met`. Exit3 means capture failure; exit5 means a complete qualifying
NMNB/capture run below the explicit score target. Such below-target replays may
be exported for Native progress, but do not pass the retail-playback gate.

## Selected Native progress replays

The owner selected both October26 Native progress files for publication. They
retain the original writer's exact bytes, player name `ysopwnab`, proper
non-practice entry and SampledGameplay. No prefix repair is used. Both have
exact fresh41-word terminal agreement and natural original-writer replay with
zero death/bomb-input rows, matching score and all eligible captures.

| Field | Extra | Lunatic4A→FinalB |
| --- | --- | --- |
| Replay | [Extra2,589,417,330](../reports/native/replays/extra-native-nmnb-2589417330-seed10025.rpy) | [Story3,890,487,560](../reports/native/replays/story-native-nmnb-4a-finalb-3890487560-seed40755.rpy) |
| Displayed / stored score |2,589,417,330 /258941733 |3,890,487,560 /389048756 |
| Captures |14/14 |37/37 |
| Initial stage / difficulty / shot / seed |8 /4 /1 /10025 |0 /3 /1 /40755 |
| Prior FinalA clear |false |true |
| Owner updates / solve seconds |69381 /448.547 |119637 /479.403 |
| Replay bytes |26995 |35865 |
| Replay SHA-256 |`5fb88138ce1a301b9695f1b3ebff2039fb36eb046242bbff7c7a590c6de76898` |`6b9bb1a5158946194c336240b2f4209cdf407e298e62cfc1386e0790fb3e9ce4` |
| Natural calculation rows |69377 |119724 |
| Natural25-field row SHA-256 |`eb25c17721fe088bca05e9bfe6a4d2ceb93684173538f92aa2caa4a481223d56` |`903b45fb0945a89ddde66c198ddd4058c738b9412669db0b9f2e5a9fe1fe98bd` |

The common profile uses score-harvest, held/baseline recovery,500ms total/60ms
proposal, priced collection/upper-stage positioning and collection/human bias5.
Extra uses H256/publication128 and pacing1.5*HP/DPS+240. Story uses
H128/publication64 without ordinary pacing. Both retain the original lead8;
these are offline compute profiles. The producers are main parent `d434e7e`
plus this Native replay-name/objective change. Frozen solver SHA-256:
`f03447bad8b1699d483be000c628bdbedffce094995537207583235257250ada`;
source patch SHA-256:
`c5760b747bc4d33b15c42ec78c4669b80817ea216de8571b5449c534ae1ad4e4`.
Fresh follower SHA-256:
`8c591082ee7d9623c133e26af4af91f80034ae52a5af273226dedf96cda4789b`;
checker SHA-256:
`326e24173f55613a8ac782f93c222124b2557d112d2bb12d5f943fa96a7bea92`.
The DAT and1.00d export target are the same as the v15 section below.

Complete local manifests, tapes, reports and rows remain under
`reports/local/native-score-offline-20261026/{extra-native-baseline-v1,story-native-price-v1}/`.
Only the two selected replay files are newly published. Both are below the human
score target, so retail Wine playback waits. Reproduce natural verification:

```sh
mkdir -p reports/local/native-progress-check
build-extra-live/th08_replay_check game_data_donottrack/th08.dat \
  reports/native/replays/extra-native-nmnb-2589417330-seed10025.rpy \
  reports/local/native-progress-check/extra.rows \
  --score-audit reports/local/native-progress-check/extra-audit.json \
  --require-all-captures
build-extra-live/th08_replay_check game_data_donottrack/th08.dat \
  reports/native/replays/story-native-nmnb-4a-finalb-3890487560-seed40755.rpy \
  reports/local/native-progress-check/story.rows \
  --score-audit reports/local/native-progress-check/story-audit.json \
  --require-all-captures
```

For fresh story tape following, add `--prior-final-a-clear 1` as well as
`--stage 1 --story 1 --difficulty 3 --shot 1 --seed 40755` and the sampled input
profile. Omitting this history follows6A instead of the producer's6B. The first
incorrect verification is retained locally; the corrected configuration agrees.

This Native-first progress passes the normal Release Native/DAT suite56/56
in970.10s, core36/36 in8.37s and focused writer/name/capture checks2/2 in39.85s.

## Selected Extra score replay

The owner-selected [highest Extra replay](../reports/native/replays/extra-native-nmnb-2564311730-seed19732.rpy)
is an unchanged original-writer export of the frozen v15 Native prototype:

| Field | Value |
| --- | --- |
| Displayed / stored score | 2,564,311,730 /256431173 |
| Outcome | Complete NMNB, all 14 eligible spells captured |
| Entry | Raw stage 8, difficulty 4, shot 1, seed 19732, story=true, priorFinalA=false, SampledGameplay |
| Owner updates | 68429, from complete Extra entry without prefix repair |
| Replay | 26949 bytes; SHA-256 `2bf469a6cbe59732f53b2a7b2ef208d3b450fc272e4c9c172eff244502b0c2d4` |
| Natural playback | 68425 calculation rows, replay input clock 67925, no death/bomb-input rows, matching terminal score and 14/14 capture outcomes |
| Complete 25-field row SHA-256 | `f172b6a60cc440696fd2f4d9015c2ae4a4848b766a4ddb6a8745da0bb47d834a` |
| Recorded full action tape SHA-256 | `c66646b84430d4e3b881bca7a6667772f5b32c9bd3261b5f0b1eebfb2f0bfd53` |
| Frozen solver binary SHA-256 | `740826437ca21e952e6ff29d55496a766ee089721d162570669aa3e89dfa68a6` |
| Frozen/fresh replay checker SHA-256 | `ee9fd522cb1795d7750227ffe5206eb66d691dcafd4ddd060486864d7501aba3` |
| DAT SHA-256 | `9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb` |
| Export compatibility target | Retail 1.00d, size 840704, checksum 2724749753, version 0100d |

The producer manifest records main parent `576c5f1` **plus local prototype edits**;
that parent alone is not the solver identity. The complete local manifest is
`reports/local/score-styles-20261025/extra-flow-v15/manifest.json`, task 207;
prototype source snapshots are in `alpha-v15-src/`. Policy source SHA-256:
`a3b5cf8d300287b8a4762995c12a9a989e7126f51b7a5b21fa50e593ef5ec691`;
Native adapter SHA-256:
`f00d548075dffe1dce60ba95bbdcc8465b71306dcf97a62e525d1e2ef0e1f743`.

The explicit profile is 256-update verification,128 publication,8-update lead,
250 ms total search,60 ms proposal, held recovery and neutral baseline movement
recovery preserving primary fire intent. Collection uses source pickup reach+2,
no-boss upper-stage positioning and 5-point collection/economic human preferences;
active graze goals take priority. Capture reserve is 1.5*observedHP/DPS+240.
Source gauge warmup/income sampling and the corrected 20-pixel ordinary graze
expansion are enabled. The original 408.479 s solve, rejected work, fresh verification
and weaker/failed attempts are charged separately in Status.

The October 26 handoff repeats natural replay, every spell outcome/bonus and fresh
41-word terminal agreement on the exact recorded action tape. Calculation rows,
Session API calls, replay clock and original owner updates are different units;
they are not expected to share one count. Fresh execution/natural replay verify
Native execution and export agreement. This selected run has **no actual online
Wine scoring run or retail playback validation**. It remains a frozen prototype
result; current-main searches can choose a different action-dependent future.

Handoff checks pass Release Native/DAT CTest **56/56 (933.85 s)** and core
**36/36 (8.49 s)**. Fresh tape execution costs29.622s in this verification;
the original38.097s follow remains part of the original experiment's accounting.
Documentation path/heading checks and14 local chapter-anchor checks pass.
The fresh checks and publication receipt stay in
`reports/local/handoff-cleanup-20261026/`; only the selected `.rpy` is newly published.

Reproduce the tracked replay from a Release headless build and owned DAT:

```sh
mkdir -p reports/local/extra-replay-check
build-headless/th08_replay_check game_data_donottrack/th08.dat \
  reports/native/replays/extra-native-nmnb-2564311730-seed19732.rpy \
  reports/local/extra-replay-check/extra.rows \
  --score-audit reports/local/extra-replay-check/audit.json \
  --require-all-captures > reports/local/extra-replay-check/result.json
sha256sum reports/local/extra-replay-check/extra.rows
```

Require exit 0, natural completion, zero death/bomb rows, raw terminal 256431173,
14/14 captures and the exact row hash above. To watch in an owned retail game,
copy the unchanged file into an unused `replay/th8_XX.rpy` slot. Retail viewing is
a separate check, not a property established by the export target header.

## Resume Extra scoring on main

Build with the private DAT configuration described under
[local build](#local-build-and-core-checks) and
[Native reproduction](#native-headless-reproduction-and-acceleration-checks).
Use a fresh output directory for each attempt. This is the integrated seed 10025
profile that previously completed at 2,453,582,120, NMNB/14/14:

```sh
build-headless/th08_shadow_style game_data_donottrack/th08.dat \
  reports/local/extra-main-next 8 4 1 10025 score-harvest 100000 1 900 \
  --verify-horizon 256 --publish-horizon 128 --held-recovery 1 \
  --attack-pacing 1 --baseline-recovery 1 --search-budget-ms 250 \
  --priced-collection 1 --upper-stage 1 --collection-bias 5 --human-bias 5 \
  --capture-factor 1.2 --capture-padding 120
```

For the integrated v15-style seed 19732 profile, change the seed and use
`--capture-factor 1.5 --capture-padding 240`. Its last main run **failed 13/14**;
this command does not promise the selected prototype's2,564,311,730. Freeze
source/binary/options and report every attempt. Wall-time budgets, timeouts and
proposal ordering can change the trajectory. Prefer a recorded bounded failure
investigation over repeatedly searching until only a favorable tape is reported.

After a complete solve, verify full inputs and the original-writer replay:

```sh
build-headless/th08_shadow_follow --dat game_data_donottrack/th08.dat \
  --actions reports/local/extra-main-next/plan.actions \
  --output reports/local/extra-main-next/fresh-follow.json \
  --stage extra --difficulty 4 --shot 1 --seed 10025 --story 1 \
  --input-profile sampled-gameplay --frames 100000
build-headless/th08_replay_check game_data_donottrack/th08.dat \
  reports/local/extra-main-next/native.rpy reports/local/extra-main-next/natural.rows \
  --score-audit reports/local/extra-main-next/natural-audit.json \
  --require-all-captures > reports/local/extra-main-next/natural.json
```

Require matching all 41 terminal words, score and every capture outcome/bonus.
A failed or incomplete solve cannot certify a successful full run; its partial
score stays labeled as such. The synchronous Native owner waits during planning,
so its throughput is not actual online FPS. The live `th08_shadow_online` CLI
now exposes pacing, reserve, price/staging/form, baseline recovery and total
search budget. For example, after launching an isolated Wine observer/owner:

```sh
build-issue1/th08_shadow_online game_data_donottrack/th08.dat RUN_DIRECTORY \
  reports/local/extra-live/comparison.json mpc 2400 behavioral h256-held \
  score-harvest --attack-pacing 1 --baseline-recovery 1 --search-budget-ms 250 \
  --lead-updates 24 --publish-horizon 128 --priced-collection 1 --upper-stage 1 \
  --collection-bias 5 --human-bias 5 --capture-factor 1.2 --capture-padding 120
```

Create the output parent first. `RUN_DIRECTORY` must contain the real observer's
versioned start, journal and mapped command file. This is the failed wine-64
profile, not a complete online result. Lead is an immutable1..32-input prefix;
publication must exceed lead and fit both the complete certificate and128-input
wire capacity. Invalid/duplicate controls are rejected before waiting for an
owner. Search telemetry is `comparison.json.searches.tsv`; preserve it with the
report, original RNG start, frozen binary/source/options and actual inputs.
The owner continues while branches run. Completion, NMNB and capture/export
checks are required separately from favorable average FPS or publication slack.
With `--baseline-recovery 1`, `--recovery-style expert|zen` selects the alternate
movement policy, and `--recovery-fire capture` enables new fire in that recovery
instead of preserving the primary pacer's fire intent. Defaults remain
baseline/preserve. These controls keep the original total budget and forced lead;
their extra Native work is charged. Wine-66 is a separately recorded trial of
expert/capture, not an established replacement for the complete online PB.

The Native harness fixes proposal H24/beam 192/cell 16/guard 16 and an 8-update
search interval. Optional verification 64/128/256, publication 64/128, held recovery,
pacing, capture reserve, price target, stage anchor and form controls are reported
independently. No source edits are needed for later ablations. Current work
prioritizes higher valid score; no ablation matrix is required for this handoff.

## Selected online recordings and replay provenance

These are preserved online/media results with their own profiles. They do not
become the producer of the selected2.564-billion Native replay above.

The owner-requested [recorded Stage 4A replay](../reports/native/replays/shadow-recording-v2--stage4a-d3-s1-seed34235.rpy)
corresponds to the actual split-capture Lunatic Marisa/Alice practice, real seed34235.
The original Native replay writer consumes a fresh execution of all 43597 actual
sampled input/control receipts; each receipt agrees with the Wine journal before
export. Natural replay then completes all 43597 calculation rows, zero deaths/bomb
inputs, raw end score27231727 and the pinned 1.00d executable target. Its replay
input clock is 43528, a different unit from calculation receipts. This is an
export of verified actual inputs, not a different searched route; its producer
metadata is Native and its playback clock is not the video's wall clock.

Replay SHA-256: `0e670b832b595bb153429407d17c03d280450dcfe19dbcca002145de2029a92d`.
Original matched video SHA-256: `7f5f4d2938840971b7eeb23c05d56d4ba86c25096ab16a1f95a2e0b9ed4b31a0`.
The original video is 770.566 seconds, H.264 640x480 with actual AAC stereo game
audio; videos remain local. A later compressed delivery retains this case identity.
The compact 640x480 fast-CRF26/AAC128k delivery is 191516144 bytes over 770.584
seconds, with SHA-256 `003be7ce892e8e1f366eb89cc9454d78175198eae585d25b43c0eeeeb4d971c8`.
Its complete decode passes; capture frame timing is retained apart from four
duplicate-timestamp drops required for the encoded delivery.
Copy the replay into an unused `replay/th8_XX.rpy` slot in an owned game copy.
The [standalone Final B replay](../reports/native/replays/shadow-recording-v3--finalb-d3-s1-seed29085.rpy)
corresponds to real seed29085, Lunatic shot1, the `fbb7758` client and V2 dialogue
advancement. Fresh following and export agree on all 30333 actual receipts,
zero miss/bomb, zero drift/player error and raw live score93642342. The 8845-byte
replay naturally completes NMNB with the pinned 1.00d target. Natural playback
reports 30330 rows, replay input clock29829 and end score127392342: the original
final-practice/playback clear-bonus offset is 33750000, as documented below.
This is not a row-for-row terminal or score equality claim; the file preserves
the verified actual inputs through the original writer. Actual retail playback
of this selected file remains unchecked.

Final B replay SHA-256: `e565f51c06e210e3242d1e136133184023863ca170b7b83d2810dbad5993011c`.
Matched capture-quality MP4 SHA-256: `9f94a8e74b56836211bab62a3a06d763577e33512a43004557b936b41d71fff8`.
The MP4 is 523.983 seconds/672570952 bytes, with its capture video stream copied
and audio aligned by trimming 10816 samples at 48 kHz. Its complete decode passes.
The 20-second spell158 Kaguya preview starts at movie second198, remains 640x480,
and is 6748573 bytes, SHA-256 `b8eb00563589ff2fceec2d6377e4acaa8b95d14ba94d62bcb5bd90081eacc4ac`.
The [full 4A-to-Final-B replay](../reports/native/replays/shadow-recording-v3--story-4a-finalb-d3-s1-seed40755.rpy)
corresponds to the actual complete Lunatic shot1 story, real seed40755, identical
`fbb7758` client, prior-Final-A-clear and dialogue advancement recorded in V2.
Fresh following and export agree on all 121533 actual receipts, 121528 calculations
and five handoffs, zero miss/bomb/player-position error. Numeric drift remains
telemetry. The 34489-byte replay naturally completes NMNB through raw stages
0/1/2/3/5/7, 121654 playback rows and raw end score218996507, matching the live
terminal. Its per-stage input clocks are 14049/15458/14962/24775/21583/29811;
these are separate from receipt/calculation counts and video wall time. This is
the original writer's export of verified actual inputs, without another search.
Selected Stage2 also completes actual normal-mode retail replay timing with
all 15216 callback packets identical across the four renderer configurations and
no death/bomb rows. A held-out normal-mode Final B playback completes with 29501
callbacks, stage-complete set, no death/bomb rows and matching score 218996507.
These are selected-stage playback checks. Full retail playback remains unchecked.

Story replay SHA-256: `202622b4e52e3cedc959f5f41d3e745b8f9d51faad33cf09571834f884758c2a`.
Matched original MP4 SHA-256: `ac7fe1f8abdeb0ecee88d5b7e7b2e26cfa76ae0ee20b56076263dbd9b77fbe03`.
It is 2129.716 seconds/2608210712 bytes, with capture video copied and audio
aligned by trimming 21242 samples at48kHz. Complete media decode passes. The
compact full-story delivery retains 640x480 and capture cadence, fast CRF26 and
AAC128k from aligned original PCM: 2129.717 seconds/683523714 bytes, SHA-256
`615c78f3c0cdfe2c9c7f8536c573355941a9c3e018c9c94f37bac8de4fbfee9c`.
Its full video/audio decode passes, with eight duplicate-timestamp drops and no
interpolation. The stopped first-clear
seed24474 attempt is not published as a completed route.

`th08_shadow_export DAT RECORDING_DIRECTORY OUTPUT.rpy` reads the versioned start
record and requires the owner's Complete marker plus an exact-length journal.
It freshly executes every actual input with SampledGameplay, compares each
receipt under the declared behavioral contract and requires a complete NMNB
terminal (Final B for the second-clear story). Only then does a child invoke the
original Native replay writer. It rejects incomplete/stopped/dead recordings and
existing output paths. Export and fresh natural replay are execution/index checks;
they do not establish independent physics equivalence or actual retail playback.

The owner's un-compacted Extra delivery is the complete actual `wine-35` movie:
`reports/local/shadow-runtime-v1/wine-35/extra-baseline-online.mp4`,836.633 seconds,
1128157155 bytes,640x480 H.264 and48kHz stereo AAC. No second video encode is used.
Its SHA-256 is `eec2c0a6e9ca064a78e7db45f83341e6d050844b5167029e7bc9275460d312c5`.
Both its video and the original Matroska capture have canonical Annex-B stream
SHA-256 `4f7faee6c923b16d974196ce692b13f11479b5d5c3829d4795d620f738d4fd8f`.
Audio alignment trims13995 original48kHz samples. Complete video/audio decode
passes. The matching local `complete-extra.rpy` has SHA-256
`dea524c41bf88b9355ff7d6cd1a978515e1a702ec47ae5d8e47597589104b058`;
the movie and replay remain ignored, with the earlier live/following evidence in Status.

## Source capture audit and Native scoring contracts

The October25 read-only source-lifecycle audit repeats both preserved natural
replays. All121654 story rows and46497 Extra rows remain byte-identical to the
original captures: row SHA-256 respectively
`17815533a8e343a54de382f108e88a89cebf184e434e57b877f0c9fbec33330e` and
`452fc7cc120a6da7c81532eb98f19c47b185bd2fd7e505d82a69016fffd81074`.
Story's missing Last Spells are one-based cards13/32; ordinary cards6/10 end with
timer-transition failure. The other33 captures include Kaguya's timed Last Spells.
Extra captures14/14, including card205. Bonus award values come from `EndSpell`,
not from an OCR estimate or a positive score delta that might include item pickups.
Stored/displayed terminal scores are218996507/2189965070 and116696962/1166969620.
These observations describe the matching Native playback and its published state
projection; replay mode deliberately does not update persistent capture history.

Score auditing preserves the existing replay-check stdout/row interface:

```sh
build-issue1/th08_replay_check game_data_donottrack/th08.dat \
  reports/native/replays/shadow-recording-v3--story-4a-finalb-d3-s1-seed40755.rpy \
  reports/local/story.rows --score-audit reports/local/story-score.json \
  --require-all-captures
```

Exit3 means natural playback succeeded but complete capture coverage failed;
exit2 means playback/reporting failed. The report uses source card catalogues for
visited stages, retains skipped Last Spells and reports each lifecycle outcome
and bonus. `observations` counts replay calculation rows in this mode, not batched
Session calls. Start/end timestamps count GameManager calculations. The two
focused real-data regressions cover the old story's negative capture case and a
complete Stage1 scoring4/4 case, fresh execution and natural replay.

The bounded Native comparison tool is:

```sh
build-issue1/th08_shadow_style game_data_donottrack/th08.dat \
  reports/local/score-trial 0 3 1 23035 score 70000 0 180 \
  --collect-items 1 --aim-boss 1
```

Arguments after the output directory are raw stage, difficulty, shot, seed,
proposal preference, maximum owner updates, story flag and wall seconds. The
directory must be fresh. Preferences include baseline/zen/expert/sniper/graze and the score family
(`score`, `score-survival`, `score-harvest`, `score-precollect`, `score-flow`).
The score base enables current-item collection and literal boss aiming.
Optional `--collect-items 0|1` and `--aim-boss 0|1` independently override those
two controls. The result records both resolved booleans and the score-goal
revision, so later ablation runs need no source edits. No new ablation matrix is
required for the current score-improvement campaign.
`score-survival` enables the separate `--survival-focus 0|1` control, recording
form-goal revision2. `--survival-bias VALUE` records the nonnegative prior strength
(default0.01). `score-harvest` adds quiet nonspell boss collection and an existing
edge prior; `score-precollect` additionally prepares current unconditional enemy
drops before firing. `--boss-collect 0|1` and `--precollect 0|1` independently
override these collection controls; both require item collection enabled. Policy
contracts cover closing the boss gate, pre-kill fire release in both decisions
and branch proposals, restoring fire at the collection line, and clearing a
finished collection target. `--verify-horizon 64|128|256`, `--publish-horizon 64|128` and
`--held-recovery 0|1` make the search/publication/recovery choices explicit; all
resolved values, failed branches and retained-window counts are reported. The
read-only score audit counts physical graze awards separately from displayed
graze. Its frozen negative replay regression checks their source bounds and the
unchanged complete25-field row hash.
Per-spell audits now record source-boundary score, collected-orb, point-count and
point-value snapshots, plus physical/ordinary/suppressed graze counts. The start
hook follows preceding-reward settlement; a focused fast-transition regression
guards against charging that bonus to the new spell. Fields remain additive in
audit schema1 and no instrumentation changes gameplay/RNG.
`score-flow`/`--survival-flow 0|1` exposes the separate source-survival stream
proposal, with its resolved strength and flow-goal revision recorded. The policy
contract checks that ordinary attacks keep the harvest result and that an active
survival proposal travels toward an observed ordinary stream with focus.
Mirroring a distant stream must change its proposed endpoint; this regression
fails when the macro omits the source's20-pixel graze expansion. Native safety
verification remains unchanged, and the corrected complete-duration trial's
13/14 failure remains recorded in Status.
Default Native budgets are H64/lead8/110ms/60ms; overrides are explicit.
`--attack-pacing` has a separate contract for excluding source damage reduction
from mature DPS, waiting for extreme-youkai state, profitable/unprofitable orb
samples, bounded unproductive warmup, capture deadlines and fire intent in both
decisions and retained proposals. `--attack-focus-bias`, `--baseline-recovery`
and `--search-budget-ms` are independent reported Native experiment controls.
The complete local prototype and every weaker/failed trial remain in Status;
fresh terminal/replay agreement is not an independent physics oracle.
The price/staging contract checks the source pickup reach and whole proposed
pickup position, collection human preference, empty-field stage positioning,
boss-registration exit, and survival-graze priority over general human preference.
An initial stationary-first-action assertion was rejected: beam proposals can
move before returning to the target. The retained contract checks economic
endpoint/phase behavior without prescribing that incidental first movement.
The optional `--capture-factor`/`--capture-padding`, price height, stage anchor
and two human preferences are independently reported. Complete-duration local
cases retain both higher scores and weaker outcomes; read-only price probes
require byte-identical original25-field replay rows before interpreting values.
The tool records all rejected work,
timeouts, owner/branch Native CPU excluding file I/O, movement/form metrics and
terminal41-word projection. Periodic Z advances dialogue. Story mode declares
prior FinalA history except Extra. Output is a tape, source score audit, complete
result and original-writer replay for a successful terminal. A synchronous owner
pauses during planning; this tool never supplies online FPS evidence.
Fresh `th08_shadow_follow` accepts `--input-profile sampled-gameplay` and
`--prior-final-a-clear 1` for that exact execution contract. The live client also
accepts a final optional proposal-preference argument after its existing horizon
profile and records its name/revision. Baseline remains unchanged by default.

Core CTest cases: 36

Normal core tests require no game assets or reconstruction. Optional source comparisons
add two CTests (38 total). Native builds add process, semantic, hazard-probe and
unsupported-instruction and online-policy contracts; private DAT enables the
complete-scene, online-stage/replay, shadow-follow and planner-prefix CTests (54 total,
including the score-audit and complete style-scene regressions).
An optional `TH08_SHADOW_FAILURE_TRACE` CMake FILEPATH enables one additional
retained-journal regression (55 total) against the private seed42386 Stage4B trace.
`TH08_SHADOW_PULSE_TRACE` adds the private seed56891 move-then-stop regression
(56 total with both traces). It reconstructs all21120 actual receipts, requires
a fatal held direction and a surviving256-update16-step pulse, compares all256
rows with a literal Native tape and checks the actual lead/parent remain unchanged.

The common-start shadow follower compares independently initialized Native
processes under one configuration and the actual input tape. It performs no native
future steps or rewinds, waits for each receipt, and does not enforce a live clock.
The 41-word projection includes current scalar state, RNG/feedback, and hashes of
the declared actor/bullet/laser views; it is not complete world serialization.

```sh
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --difficulty 0 --spell-id 179 --seed 0 --strategy spell-portfolio --frames 2000 --actions reports/local/id179-shadow.actions --output reports/local/id179-shadow-control.json
./build-headless/th08_shadow_follow --dat game_data_donottrack/th08.dat --stage 6b --difficulty 0 --spell 179 --seed 0 --actions reports/local/id179-shadow.actions --output reports/local/id179-shadow-follow.json
ctest --test-dir build-headless -R 'shadow|documentation' --output-on-failure
```

The real-data regression compares all 1292 updates, rejects an explicitly different
follower seed at update zero, and compares 32000 historical story inputs through
the natural first handoff (31999 calculations). The partial story reports `budget`,
not complete. Reports bind DAT/binary/actions hashes. This validates Native process
synchronization only; replayed input is not a new solver or live Wine result.
The same real-data test also verifies the complete true-RNG native branch,
rejects a fatal stationary candidate, checks parent world/RNG/filesystem isolation
and executes every accepted action through the original Session. Held-tail verification is compared with literal Native execution for the same
complete64-input action tape, including a released shoot bit and parent isolation.
The optional retained4B test reconstructs all34256 original receipts: the same
32-input prefix dies at62 with a Policy tail and survives64 when held. It checks
the full prefix and parent state, with its diagnostic timeout recorded separately
from live search budgets. The core wire
contract also checks bounded publication of a longer successful certificate,
including unpublished-tail validation and rejection of a fatal longer future.
It also crosses an earlier publication boundary using the remaining successful
certificate after a fresh extension fails, preserving queued inputs and original
rollout counters. Fatal or malformed retained certificates cannot extend a queue. It
tests late-prefix agreement, stale receipt rejection and preservation of
an existing plan after a failed proposal. Neither self-comparison is an
independent physics oracle. A guided child also executes the complete 1292-update
scene from an eight-input fixed prefix: every remaining input comes from a
branch-owned callback observing the advanced world. Its packets must match the
fixed-action reference, and callback counters/state must remain unchanged in the
parent. This guards callback update order and fork isolation, not Policy quality.
The core scheduling regression retains the already-verified queue while the
owner advances after an unsuccessful nominal extension. That same plan stays
ineligible for reuse after death or timeout; a new certificate origin enables
reuse again. Advancing the current receipt alone must not restore eligibility.

The optional local Wine bridge requires a 32-bit MinGW compiler and the pinned
retail 1.00d EXE (`330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`,
840704 bytes). Keep a private copy of the game/config/score and a dedicated Wine
prefix. It attaches only to the new process created by its launcher. Its three
in-memory hooks are declared; no gameplay RNG value is written. Core/native builds
do not require Wine or MinGW unless the observer option is enabled.

```sh
cmake -S . -B build-headless -DTH08_HEADLESS=ON -DTH08_WINE_SHADOW=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-headless --parallel 2
export WINEPREFIX="$PWD/game_exe/owned-shadow-prefix" WINEARCH=win32 WINEDEBUG=-all
export TH08_SHADOW_CHECKPOINT='5,3,0,107,0' TH08_SHADOW_UPDATES=6000
run="$PWD/reports/local/wine-shadow-example"
game="$PWD/game_exe/owned-shadow-game"
mkdir "$run"
launcher=$(winepath -w "$PWD/build-headless/wine-shadow/launch.exe")
observer=$(winepath -w "$PWD/build-headless/wine-shadow/observer.dll")
executable=$(winepath -w "$game/th08.exe")
destination=$(winepath -w "$run")
(cd "$game" && wine "$launcher" "$executable" "$observer" "$destination") &
./build-headless/th08_shadow_online game_data_donottrack/th08.dat "$run" "$run/result.json" mpc 300
```

Start a normal game in the owned window. The optional checkpoint is applied only
before original gameplay registration: zero-based stage, difficulty, shot, spell
or -1, story 0/1. Unset it to preserve the actual menu-selected configuration.
The observer records the real seed before registration, then publishes only
actual gameplay/control updates. Use a new output directory each time; existing
journals/command files are deliberately rejected. The mapped command file requires
local coherent x86 Linux/Wine memory; remote SSH transport is not implemented.

The client accepts mode `follow` for receipt-only Native comparison, including an
already finished observation journal; mode `mpc` adds branch-owned futures.
The wall-seconds argument caps total command wall seconds, including menu/start
waiting. Exit 0 is the actual case terminal; 3 is projection mismatch, 4 is death,
5 is a budget/stop boundary and 1 is runtime/transport failure. Actual sampled
inputs export beside the output as `.actions`. Reports bind start/trace/actions,
DAT and Native binary hashes, configuration, budgets, proposal/native attempts,
charged interrupted work, stale results, input fallbacks and actual owner clocks.
The trace's calculation time brackets the original hooked calculation function,
including input-hook calls within it. Observer time covers projection/cache
preparation after that call, stopping before trace encoding, `WriteFile`, atomic
publication and outcome handling. Inter-receipt periods include work between those
boundaries, such as rendering, limiting, scheduling and journal writes; they do
not attribute a slow frame to one component. Total elapsed time starts at the
first hooked calculation's entry and ends at the last projection timestamp.
`TH08_SHADOW_RENDER_TIMING=1` adds an independent `render-times.bin` journal
without changing the existing228-byte receipt format. Its16-byte little-endian
header contains uint32 magic0x31544d52, version1, sample size64 and reserved0.
Each sample has eight uint64 words: receipt sequence, raw stage, elapsed ns,
Present-end period ns, BeginScene-to-EndScene ns, original Present-call ns, scene
count and flags (1 no draw,2 failed Present,4 repeated receipt). The first period
is unavailable. Draw includes submission/synchronization; Present includes backend
waits. Remaining period time includes original pacing, other callbacks, scheduling
and preceding profiling writes. These are measured boundaries, not GPU-only costs.
The core wire contract also checks the disabled legacy refresh setting and the
exact128-receipt refresh boundary, including spacing after a failed attempt.

`TH08_SHADOW_OBSERVE_REPLAY=1` requires render timing and rejects history/dialogue
flags and non-replay gameplay. It forwards original input calls and lets the
original replay loader restore recorded RNG/inputs. Its `replay-timing.txt` marker
and absence of `start.txt` distinguish it from a common-start live run. Normal
retail replay can restart the calculation chain during dialogue; its sequence
counts observed callbacks, not certified live calculations. Discontinuities are
retained in `replay-clock-events.tsv`. Ordinary live cursor checks remain strict.
A selected-stage replay timing run has no Native planner or capture process; record
its playback mode, software-renderer threads, environment and terminal separately.
The selected Stage2 comparison uses Wine8, llvmpipe with two renderer threads,
private game audio, no planner/capture, and the same story replay hash above.
Only the per-process `vblank_mode` setting changes from unset to0. Mean
presentations/s improves56.167→59.343; mean Present time7.900→2.342ms.
All15216 packets agree exactly, including input and all41 fields. The
[Mesa option documentation](https://dri.freedesktop.org/wiki/ConfigurationOptions/)
defines0 as disabling swap-interval synchronization. This remains an opt-in
measured environment choice; local slow windows and scheduling tails remain.
A further matched control changes only renderer threads: LP1 reaches41.045/s,
whereas LP4 reaches59.997/s with mean Present0.043ms. Both match all15216 LP2
packets exactly. LP4 has a59.811/s slowest complete5-second window and18.729ms
period p99. It uses additional rendering workers while solver concurrency stays
fixed. [Mesa documents LP_NUM_THREADS as the renderer thread count](https://docs.mesa3d.org/envvars.html#lp-num-threads).
A held-out normal-mode Final B check uses the unchanged LP4/vblank0 profile,
original pacing and 640x480, private game audio and no planner/capture. Its 29500
Present samples average 59.95552/s; the slowest complete 5-second window is 58.00598/s
and period p99 is 19.81930ms. There are no repeated receipts, absent draws or failed
Present calls. The 29501 callback packets end at stage-complete, score 218996507 and
zero death/bomb violations. This is held-out rendering and replay evidence, rather
than an additional solver clear or every-frame60Hz guarantee.
The buffered solver profile separately declares512 verification,128 publication,
lead32,480ms total and128-receipt fresh-planning intervals. Keep this latency and
compute budget distinct from the original110ms/lead8 controls.

Optional trailing arguments `[behavioral|exact] [h32|h64|h64-held|h128-held|h256-held|h256-pulse|h512-buffered|h128-recorded|h64-recorded]` select comparison and
planning profiles (defaults `behavioral h32`).
The recorded mode uses128 verification/publication, lead8 and110ms total, with
Native conditional hazard recording and independent full-path verification.
`recorded_*` fields separate geometry/search work and timeouts; its recording
updates are also charged in `proposal_*` totals. The real-data contract checks
literal replay, immutable lead, parent RNG/filesystem isolation and the retained
4B failure; the ordinary current-memory Policy remains unchanged.
The `h64-recorded` portfolio preserves H64 nominal guidance, then verifies128
updates for a recorded repair and preserves its longer literal continuation.
Publication remains64. JSON separates base/maximum verification, publication,
recording horizon and profile revision. The API also supports independently
verified shorter windows; the regression rejects overstating their length.
Live comparison records numeric bullet/laser digest drift, player differences up
to 1/64 pixel and inactive spell/route metadata; meaningful gameplay fields remain
checked. It does not modify either simulation or collision handling. Exact mode
retains bitwise projection diagnosis. Keep launcher/DLL/config identities with the
local run receipt as well. New logs,
tapes and diagnoses stay under ignored `reports/local/`.
The opt-in `h64` bundle, named held-recovery variants and nominal-prefix extension are specified in
[Planner](PLANNER.md#true-rng-shadow-runtime). Do not combine its results with
default-profile runs or attribute differences solely to horizon. Its reports
separate branch simulation, guidance, proposal work, conservative timeout charges
and actual unqueued-update streaks; these are costs, not a 60 FPS guarantee.

Core checks reject late generations, torn publications, altered immutable leads,
unsupported bomb bits and edits to the owner's private accepted copy. The complete
Native scene also compares future-lead proposal inputs with the same Policy only
after actual execution reaches that lead, guarding history, latch and private tie
RNG isolation. Rendered Wine spell completions and retained story mismatches are
separate manual real-data evidence in [Status](STATUS.md). Same-update stopped
diagnostics use raw float bits: the client writes `.bullets.tsv` beside its report;
the observer retains `retail-bullets.tsv` from the post-calc phase, before drawing.
Neither a partial story nor a complete spell establishes a continuous 60 FPS run.
The same real-data regression executes a complete first story stage in both
Controller and SampledGameplay profiles, checks the control-only handoff, then
requires the new stage's first sample/latch to be respectively zero or the
explicit supplied input. A core cursor regression covers the intervening retail
loading callbacks so that the control receipt cannot disappear again.
Existing public CI runs sanitizer OFF/ON without DAT/source;
it does not establish real-DAT scenario success. Keep new verification proportional to
the changed behavior; no elaborate sanitizer infrastructure is required.

`lzss_decode` compares the headless field reader against an independent one-bit
scalar oracle. It checks persistent nonzero dictionaries, generated streams and
byte truncations, sequential overlapping/ring-wrap copies, end markers and exact
output guards. The output-capacity obligation and zero-padded EOF remain the
native caller contract. A separate real-DAT audit checks the actual exported
Encode/Decode entry points against unchanged renamed legacy code: all 317 entries
and the file table have identical bytes, declared lengths and complete final
dictionaries after each decode. This is codec equivalence, not a gameplay oracle.

`headless_surface_copy` calls the production D3DX entry point through owned CPU
surfaces with padded pitches and change-notification counters. It checks every
16-bit pixel of the two eligible formats against direct identity and the forced
original conversion path, plus 32-bit alpha, untouched padding/source, self-copy
overlap, rectangles/clamping, color keys, scaling, X formats, invalid filters and
zero dimensions. The fast path is headless-only and requires separate owned
surfaces, equal positive full dimensions, no rectangles/key and exact filter 3;
all other cases keep their original behavior.

`model_artifacts` checks the complete saved-table file set, SHA-256, byte lengths
and data-row counts against [the model manifest](../models/artifacts.tsv).
The native online-policy contract additionally loads all three historical packs
through the production parser. Artifact identity and successful parsing do not
establish a disjoint evaluation split or learned-policy quality; [model provenance](../models/README.md)
records the inherited unknowns.

The branch-value core contract checks normalized logistic probabilities, extreme
logits, exact model round trips, immutable output snapshots and strict rejection
of truncated models, invalid scales/features, duplicate labels and failed or
incomplete native windows. The independent separable CLI fixture must fit and
reproduce byte-identical weights; overlapping train/tune seeds and native faults
must fail before creating a model. These synthetic checks validate the trainer
and format, not gameplay quality.
The online-policy contract verifies that candidate export, a constant model and
zero model weight preserve ordinary actions/depths and proposal counts. Complete
H24 samples own 23 newly submitted inputs (the old latch owns movement one), keep
shooting and contain only finite features. Partial/failed searches export none.
`headless_value_collection_real_data` compares exact nominal tapes, observation/
actor projections, terminal, score, RNG and calculation counts with/without
independent Native branch sampling on both 107 and Extra 202. An initial false
startup terminal is guarded by the required 640 nominal updates and positive
branch count. Teacher futures stay outside policy features.
The maintained collector writes `branch-inputs.tsv` with each exact submitted
sequence and prefix hash; its real-DAT contract also requires one sidecar row per
branch. The first frozen collection predates that sidecar and retains its exact
producer binary/source and parent tapes instead; the model card preserves this
provenance distinction.

The online-policy lifetime regression preserves source dispatch order: an active
gate precedes masked-record skipping, WAIT clears after program dispatch, and a
DESPAWN still collides in its activation callback. An independently stepped
three-position corridor checks the inclusive cutoff, longest safe partial depth,
unavoidable latched first hit, unknown-clock fallback and disabled metadata/tie
transparency. Native known-seed pilots and fresh action execution are separate
checks of the enabled proposal profile; neither this narrow program hypothesis
nor agreement between two Native executions is an independent physics oracle.

The complete online-stage CLI regression also executes a generated idle prefix
without Policy decisions. Native `player_state==2` is an immediate NMNB failure
during the deathbomb window, before the `deaths` counter increments. Exit 4 and
`outcome:death` therefore remain valid with `deaths:0`; the additive
`final_player_state` field makes that boundary explicit. Consumers must retain
this failure rather than require a positive life/death counter.

The shared spatial selector's `plan_contract` regression compares partial rank
selection against the frozen former full-sort algorithm in 420 deterministic
cases. It checks every retained float field, action, violation count, parent,
output position and bin statistic across ties, signed zero, infinite scores,
negative coordinates, focus bins and different widths/cell sizes. Both fresh and
reused flat-table workspaces must match, including growth, shrinking and empty
input between searches. Hash collisions still require full bin equality. Invalid
cells and non-finite coordinates still fail. Performance checks additionally require
identical full action bytes and behavioral reports before comparing timings.
The spawn-grid contract checks clipped edges, row separation, signed zero and a
large/small-term sum whose rounding changes under reassociation. A second contract
warms search storage, takes an immediate latched-collision return, then compares
the next full decision with an observer-only control, including selection counts
and exact action/depth. These guard per-decision grid and scratch-storage reuse.
The movement decomposition contract compares the frozen original move with
prepared displacement plus position addition/clamping in 204800 cases: all 256
low input-bit combinations (with unrelated high bits set), twenty speed/multiplier
profiles, ten positions and four rounding modes. It requires identical position
float bits and floating exception flags, covering signed zero, subnormals,
negative multipliers and bounds. Complete action/behavior comparisons additionally
guard integration of per-search displacement reuse into candidate enumeration.
The online AABB predicate is compared with the frozen former short-circuit
expression in 403116 finite-geometry cases across four rounding modes. Fixtures
cover all sixteen rejection combinations, signed-zero permutations, subnormals,
touching/degenerate bounds and deterministic finite bit samples. Boolean results,
standard floating exception flags, preexisting flags, errno and rounding mode must
agree. This checks the proposal predicate; native collision physics stays separate.
The half-pixel key contract compares the optimized path with the original
`lround`/mask expression at every fast-range rounding boundary and its adjacent
floats, plus one million finite bit samples (26165824 comparisons). Four rounding
modes additionally check exact values, exception flags, errno and unchanged mode,
including signed zero, subnormals and large/special-value library fallback.
Optional component clocks are checked against an unprofiled policy through
consecutive observations, a lethal root, recovery and duplicate-decision rejection.
Actions, depth, proposal counts and fallback work must agree; rejected duplicate
calls clear the caller-owned timing sample. The real-data contract also compares
profiled/unprofiled budget tapes and checks a complete prefix with zero decisions
and zero live-native time. `--profile-components 1` emits separate, explicitly
instrumented scopes as described in [Planner](PLANNER.md).

## Current-memory CPU controller

Build `th08_online` with the native Release configuration described in the README.
The owner permits current process memory, including current transform and laser
lifetime state. This baseline is `native-memory-kinematic-v1`, not an image-only
agent. It makes no future Session updates and does not rewind the live game.

```sh
./build-headless/th08_online --dat game_data_donottrack/th08.dat --out reports/local/online-stage2 --stage 2 --difficulty 3 --seed 0 --shot 0 --horizon 32 --beam 192 --reach-filter 1 --frames 60000 --seconds 900 --log-every 1
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/online-stage2-verify --stage 2 --difficulty 3 --seed 0 --shot 0 --verify reports/local/online-stage2/online.actions
./build-headless/th08_replay_check game_data_donottrack/th08.dat reports/local/online-stage2-verify/th8_verify.rpy reports/local/online-stage2.rows
ctest --test-dir build-headless -R headless_online --output-on-failure
```

For the opt-in physical clock/worker harness, a short run explicitly remains a
budget outcome. It takes approximately six seconds for 360 API ticks:

```sh
./build-headless/th08_online_live --dat game_data_donottrack/th08.dat --out reports/local/live-smoke --stage 1 --story 1 --final-route a --difficulty 3 --seed 0 --shot 1 --horizon 24 --beam 192 --diversity-cell 16 --spawn-guard 16 --reach-filter 1 --hard-phantoms 1 --motion-envelope 1 --value-model models/branch-value-v1/weights.tsv --value-weight 0.25 --history-queue 4 --scheduler-tolerance-us 0 --frames 360 --seconds 10
ctest --test-dir build-headless -R 'online_live|headless_value_collection' --output-on-failure
```

Live core checks cover absolute tick arithmetic, cutoff/origin/phase rejection,
bounded consecutive observations, worker faults, discarded-work accounting and
expired-history agreement with a serial Policy. The one-frame Native CLI check
uses a declared generous owner tolerance solely to verify serialization/input
accounting, not performance. It requires JSON-escaped literal TSV model metadata
and rejects zero queue capacity before Native initialization. Physical timing and
complete routes require separately frozen experiments with their actual tolerance.

Natural replay `rows` counts recorded gameplay calculations; `stages[].frames`
is the last observed `ReplayManager::frameCounter`, a replay input clock. They
are different units. `ReplayTraceRecord` runs before input-enabled/clear gates
that can prevent clock advancement. The complete Stage 2 regression, for example,
has 32993 rows and last input-clock value 32924. Validate total row coverage against
the tape/calculation count, plus natural terminal, score, ordered stage path and
zero death/bomb rows. Do not require the last input-clock value to equal total rows
or patch it with a guessed constant offset. Fresh action agreement, natural replay
and independent retail comparison remain separate evidence levels.

The optional branch-value experiment uses separate offline collection and C++
training tools. Use new output paths; neither collector episodes nor saved model
snapshots are overwritten. The collector's `--stage` is the native index 0..8
(1/2/3/4A/4B/5/6A/6B/Extra), unlike the online CLI's named stage convention:

```sh
./build-headless/th08_value_collect --dat game_data_donottrack/th08.dat --out reports/local/branch-labels --stage 5 --difficulty 3 --spell 107 --seed 42 --shot 0 --frames 20000 --stride 128 --checkpoints 32 --seconds 1800
./build-headless/th08_value_train reports/local/train.tsv reports/local/tune.tsv reports/local/new-value-weights.tsv > reports/local/value-training.json
./build-headless/th08_online --dat game_data_donottrack/th08.dat --out reports/local/value-stage5 --stage 5 --difficulty 3 --seed 42 --shot 0 --horizon 24 --beam 192 --reach-filter 1 --hard-phantoms 1 --motion-envelope 1 --value-model models/branch-value-v1/weights.tsv --value-weight 0.25 --frames 100000 --seconds 1800
```

The first command supplies one parent episode, not disjoint train/tune datasets.
Construct whole-parent/seed splits before collection, preserve both header lines
when merging TSV rows, and freeze model/weight before independent evaluation.
Every sampled prefix adds separately reported native updates and wall cost.
The saved first model reproduces byte-for-byte when the maintained trainer fits
the audited original train/tune files locally. This checks reproduction, not
generalization. Its [model card](../models/branch-value-v1/README.md) owns label,
split and input-profile assumptions.

Stage names use the repository convention `1/2/3/4a/4b/5/6a/6b/extra`, rather than
the preparation prototype's zero-based numeric CLI. `--story 1 --final-route a|b`
uses the existing native dialogue/unlock-history checkpoint contract. For a
continuation, pass `--prefix` with its original checkpoint; frame budgets include
that prefix, and observer history is warmed without replaying past search ties.
Only one-update commits and the kinematic forecast are supported. Outcomes have
distinct exits: complete 0, setup/output error 2, budget 3, death 4, runtime error 5.
Output directories must be empty; failed tapes remain evidence, not successes.

Optional `--planner-lag 1|2` uses the deterministic delay model described in
[Planner](PLANNER.md). `--executor-check`, `--emergency-horizon` and
`--emergency-beam` declare its check/rescue budgets; warm/held/tail combinations
are rejected. Lag zero remains the default. Policy regressions cover the one-input
latch at adoption, early/punctual adoption, newly visible hazards, pending-result
cancellation, actual input/position/movement/phase changes, skipped decisions,
an unavoidable first update and both shorter/longer rescue depths. Full Stage 2
keeps `c4acd20a…`; complete lag-zero 6B tapes also retain their parent identities.
Lag-one/two window repetitions must match actions and deterministic event counts,
while excluding diagnostic times/overruns from equality. New complete runs still
require the same fresh action and natural replay gates. Separate executor/planner
statistics in a synchronous driver do not certify unpaused deadlines.

The optional lazy-index regression compares eager and delayed construction through
normal reuse, a new incoming hazard and a phase invalidation. Actions, depths,
selection/search counts and executor events must agree while at least one update
builds fewer indices. The hazard must also force a previously omitted suffix to
be constructed before replacement search. Component clocks must remain disjoint.
Three alternating spell-166 pairs per model and two complete 6B candidates retain
the eager lag-two tapes and all event counts; the complete tapes pass fresh action
execution and natural replay. This changes index scheduling, not the forecast or
the native collision predicate.

The opt-in hard-phantom contract compares a disabled fired bullet with the same
ordinary real bullet, with and without reach pruning. The no-model default must
still match an absent ghost. The prior must preserve ordinary search behavior,
leave disabled spawning bullets unchanged, retain conservative root failure and
respect a ghost-to-real velocity discontinuity. A model-loaded control checks
that hard promotion replaces the phantom-only soft obstacle. Native paired-bank
outcomes, default-mode action agreement and fresh replay checks are separate gates;
these synthetic comparisons alone establish neither survival nor model quality.
The pilot's prespecified practice tapes pass fresh native action execution, but
the current verifier exports a replay only when `spell < 0`. Practice receipts
leave `rpy` empty. The attempted natural replay check is retained as a failed
harness assumption, so this pilot has native-proxy evidence without a natural or
retail replay claim. Such tapes require an appropriate export/validation path
before promotion to certified expert routes.

The optional current-state motion envelope is checked against all modeled switch
times with repeated float additions in all four rounding modes. It keeps the
ordinary forecast when no alternative is supplied, rejects non-finite inputs and
requires explicit hard-phantom mode. A focused policy fixture exposes a restored
flight direction with reach-filter agreement. `headless_online_motion_real_data`
executes the complete 107 checkpoint that previously died after that mode change,
then freshly verifies its complete action tape, terminal, score, RNG and update
counts. This is a native-practice regression, without a natural/retail claim or
proof that the conditional motion hypothesis covers every native transform.

Optional `--risk-dir` requires all four files: `risk_table_merged.tsv`,
`risk_backoff_seg.tsv`, `risk_backoff_global.tsv`, `nowarn_merged.tsv`.
`--risk-phantoms 1` additionally uses collision-disabled bullets as soft risk
unless the fired-bullet hard prior replaces that path.
The maintained line parser fixes the preparation's cross-row backoff parsing, so
those results need new reproduction rather than assigning old outcome/timing
claims to the corrected model. New model hashes and training lineage belong in
the local experiment protocol. No learned model is required by the complete-stage
CTest. Its unit companion also rejects truncated/duplicate risk rows, checks
latched movement and prefix velocity history, and compares pruned/unpruned search
with interleaved independent policy instances.
The transform-history regression observes a stopped relative turn followed by its
final constant-velocity flight. Estimating continuing acceleration across that
mask change incorrectly declares the first latched update lethal; resetting the
rate history retains a complete escape, with reach-filter agreement. An unchanged
active vector transform still accelerates from rest and keeps its genuinely
unavoidable latched hit. No zero-speed or seed-specific strategy is substituted.

The October 23 exact-kernel gate compares 200000 prepared laser rotations against
the frozen original formula, including signed zeros, subnormals, large/axis angles
and all four rounding modes, with batch exception/errno agreement. Bounded spatial
selection adds 2100 dense/fallback comparisons to the frozen full-sort oracle,
preserving all retained float bits, source ties, parent/action indices and counts.
The complete Stage 2 CTest retains action SHA `c4acd20a…`; normal 40-test native/DAT
coverage passes. Optional `TH08_ONLINE_ARCH=native` and `x86-64-v3` also pass local
policy contracts; this is numerical evidence for those builds, not a flag guarantee.
The proposal-set regression separately compares first-key ownership with the
frozen original `lround`/mask formula in all four rounding modes. It covers cyclic
boundaries, signed zero, subnormals, focus separation, undersized/invalid hints,
migration of populated bins, independent instances and repeated layer resets.
Native complete-stage and prefix pairs also require zero unexpected domain
expansions; their original action, selection-count and terminal gates stay active.

`th08_spell_bank TRIALS SEEDS CASES PROTOCOL BASELINE CANDIDATE` evaluates a paired
spell-practice bank against declared case/seed files, not only available outputs.
SEEDS contains one native seed per line. CASES has the TSV header
`stage difficulty shot spell` (tab-separated; stage uses the report's zero-based
entry index). TRIALS has `profile stage difficulty shot spell seed outcome frames
actions_sha256`, also tab-separated. Every declared pair must exist exactly once;
unexpected cases/seeds/profiles, duplicate or truncated rows and runtime/setup
outcomes reject the summary. Budget outcomes remain non-completions and are counted
separately. The report binds all four input files by SHA, gives Wilson 95% intervals
and the exact two-sided McNemar discordance test, and makes no equivalence claim.
The standalone contract includes a completely missing case, missing pair, duplicates,
invalid identities/outcomes and known interval/binomial-tail values. Spell practice
remains a proxy; it does not establish complete-story probability.

The independent Release campaign freezes shot 0, Lunatic, seed 0, non-practice
6B story entry, H64/beam192/cell16/guard16, reach filter and soft phantom option.
It tests both no-model and the unchanged four-file risk model. Actual spell-166
prefixes have 42681/45032 updates; each measured continuation has 2400 decisions.
Warming does not replay tie RNG, so compare each continuation with its own frozen
control rather than demanding the original full run's suffix. Three alternating
orders separate laser caching, bounded selection and each CPU target; component
profiling is separate. Portable and native complete runs compare all actions and
non-timing behavioral fields against full controls. Six fresh action executions
and natural replays agree on score/RNG/calculations and 380292 rows, with zero
death/bomb rows. Raw rows are removed only after retaining hashes/byte counts.
This is execution agreement, not independent physics, retail or unpaused timing
validation. [Status](STATUS.md) owns measured results and remaining work.

Optional `--spawn-guard 16` adds actor-origin soft risk, `--diversity-cell 16`
retains spatial/focus beam alternatives, and `--queued-shots 1` uses positive
bounded shot prefixes from current main program cursors. All default to zero/off;
record each setting and compare complete outcomes rather than combining the best
seed from different profiles. Queued coverage is partial and counts unsupported
preview records; zero records is not an empty-future certificate. The policy
contract also checks early avoidance of a queued birth, the unavoidable latched
update, soft actor risk without a learned table and deterministic spatial retention.
`--queued-lasers 1` independently enables the existing timed direct-ECL warning
decoder; reports count warnings and unsupported dynamic/variable geometry. Its
regression checks escape before a rotated beam appears, the already-latched first
update and rejection of a reversed lethal interval. These forecasts are partial;
actual native execution, fresh actions and natural replay remain the outcome gates.
`--held-fallback 1` adds eighteen checked constant-input proposals after an
incomplete beam. Its synthetic two-beam regression retains a complete rightward
escape that the default beam prunes at depth eight, checks reach-filter agreement
and counts the extra forecast work. A lethal first latched update still returns
depth zero. Complete-world outcomes are evaluated separately from this proposal
contract.

The optional terminal-tail regression keeps H8 branching/depth while previewing
a birth at update nine through eight extra held-input checks. It verifies early
escape, reach-filter agreement, stopping at a complete first suffix, separate
query counts, a lethal latched update and rejection of H+T above 200. The narrow
laser-arc regression retains a clear first update that the previous cosine/dot
tolerance rejects, with 10001 independent static source-predicate samples across
the angular interval. Companion checks cover wide arcs, full-circle reach and
the opposite ray at near-zero reach. These geometry contracts are separate from
full-world outcomes.

The optional `--spawn-clock 1` regression requires an escape before a supported
animation completes, checks reach pruning and rejects a purported spawn clock on
a fired bullet. A late first snapshot with one animation call left must account
for both fractional and full displacement, including an unavoidable latched hit.
Unsupported timing retains the named legacy hypothesis. The pure ANM companion
checks all three terminal opcodes, next-call/null-cursor order and refusals for
unknown, masked, malformed, blocked, negative and fractional clocks. These timing
contracts do not certify velocity changes at activation or later native state.

The optional `--warm-fallback 1` regression uses two crossing linear bullets and
a one-node beam. Rechecking the previous plan suffix restores a full 24-update
proposal after ordinary replanning discards it at depth 13. The same check covers
reach pruning, skipped decisions, changed latched focus, a new forecast collision,
depth ties, an already-latched death and the exact additional-query counts. These
are synthetic proposal contracts; complete native outcomes remain separate gates.

`--direct-turn-envelope 1` tests a current-memory angular uncertainty model.
Its policy contract includes opposite-turn escapes from signed extrapolation,
sixteen orientations around the full circle, reachable-envelope agreement and
pre-birth cue timing through the input latch. Raw-batch tests cover difficulty,
both forward branch paths, truncation, variable writes and backward-loop rejection.
Reports add `turn_hint_records`, `unsupported_turn_hints` and
`turn_envelope_frames`; these count a partial heuristic, not certified future
geometry. Observe both complete outcomes and controller latency.

The queued pooled-laser regression starts from a harmless birth update and
requires movement before the first long active hitbox. A one-update-late snapshot
cannot recover that escape through the latched input. It also checks reach-filter
agreement and rejects a purported spawn whose timer has already advanced. Source
preview contracts remain independently exercised by the native planner tests.

The relative-turn policy regression warms a stationary velocity history, then
installs an owned source turn. It requires an early escape, counts only the
bounded kernel updates, checks reach-filter agreement and the first input latch,
and rejects aimed/inactive state masquerading as an independent relative turn.
The existing bullet-motion source differential remains a separate kernel check.

For an always-shoot student's offline teacher queries, add `--shoot 1
--quiet-retries 0` to `th08_plan` with the original checkpoint, immutable
`--prefix` and explicit total frame/time budgets. Inspect every generated suffix
input's shoot bit, the byte-identical prefix, `options.quiet_retries:0` and
`quiet_windows:0`; a zero quiet-window counter alone does not cover checkpoint
or short retry delays. The prefix CTest completes a real spell from a supplied
quiet input, retains that input exactly, checks every continuation input and
freshly verifies the result. The default planner retains adaptive quiet retries.
Matching output actions does not remove the offline teacher's privileged future
rollouts or establish online completion.

`online.json` binds executable, DAT, exact action/prefix bytes and each risk file;
the local protocol additionally binds the source archive and environment. Decision
timing includes observation copying, policy and input decoration, excluding native
execution and file serialization. Prefix reconstruction contributes to native and
whole-loop costs; setup is separate. Buffered `decisions.tsv` records predecision
state and the submitted input when requested. Reports retain mean, p99, p99.9,
maximum and misses against the configured deadline. `forward-measured` waits for
late decisions and explicitly reports `deadline_enforced:false`. A passing stage
or a low mean is not proof of a real 60 FPS controller.

Fresh verification must match exact terminal, score, RNG, action hash and
calculation/control events. Natural replay must finish with the same score and
observed calculation rows, zero death rows and zero bomb-input rows. These checks
establish native execution agreement, not an independent physics/retail oracle.
Source, model, process receipts, timings and all failures normally stay local;
publish sanitized conclusions and maintained code/prose. The owner's explicit
request for the two successful complete 4A story captures is a narrow exception:
their [publication audit](../reports/native/online_story_4a_validation.json) retains
sanitized provenance, timings and native verification receipts. It does not add
retail playback or enforced-deadline evidence at its publication date. The later
manual retail audit in [Status](STATUS.md) covers these exact two original files;
the frozen publication JSON is not refreshed with new experiment artifacts.

Final-stage practice recording and natural playback currently differ at the
clear-bonus branch. Native practice sets `isPracticeMode`; playback clears it and
uses a separate replay-practice predicate. Static inspection of the unchanged
retail 1.00d executable (SHA256
`330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`)
agrees with the native GUI branch: final-stage non-practice clear adds
`2500000*lives + 500000*bombs` before difficulty scaling. A separately checked
supplied Stage 6A tape predicts and reproduces the same 33750000-point Lunatic
offset seen in the Stage 6B control; it starts at the clear-bonus update. Position
also diverges later in the clear tail, where playback freezes input while live
control continues. This explains the observed mode difference without changing
the score gate, and does not certify a complete retail replay or all 25 fields.
Keep the failed equality results. Any non-practice final-stage checkpoint must be
named and evaluated separately rather than relabelling a failed practice result.

## Native fork planner and replay gate

This section reproduces the preserved earlier offline portfolio and its report
producers. Its named profiles, build identities and timing results are historical;
the current Extra scoring entry points are at the top of this page.

The C++ replay suite requires the accepted corpus, manifest and schema from upstream
`02ff72f29e7b4928f21c5a2fb0f57ffa0cec5415`. Pass their locations explicitly; raw replay
captures and game data remain local. The default full gate pins the 108-entry manifest
and supported v4 schema, verifies replay hashes, creates a new output directory, and
requires natural successful captures with every expected row/fingerprint.

```sh
./build-headless/th08_replay_suite --checker build-headless/th08_replay_check --dat game_data_donottrack/th08.dat --manifest reference/config/replay-fixtures.json --schema reference/scripts/replay-external-schema.json --corpus reference/tests/fixtures/replays/th08-1.00d/accepted --output reports/local/fidelity --jobs 8
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/spell92 --stage 4b --spell 92 --difficulty 3 --seed 0 --rollout 64 --diversity-cell 16 --shoot 1 --checkpoint 60 --keep 12 --frames 10000 --seconds 180
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/spell92-verify --stage 4b --spell 92 --difficulty 3 --seed 0 --verify reports/local/spell92/plan.actions
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/story --stage 1 --story 1 --difficulty 3 --seed 0 --rollout 64 --beam 384 --diversity-cell 16 --checkpoint 120 --keep 64 --shoot 1 --frames 250000 --seconds 2400
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/story-resumed --stage 1 --story 1 --difficulty 3 --seed 0 --prefix reports/local/story/plan.actions --rollout 64 --diversity-cell 16 --checkpoint 120 --keep 64 --frames 350000 --seconds 1800
./build-headless/th08_plan_matrix --seed 0 --output reports/local/matrix.json
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/final-b --stage 1 --story 1 --difficulty 1 --shot 1 --seed 0 --final-route b --rollout 64 --beam 384 --diversity-cell 16 --checkpoint 120 --keep 64 --frames 350000 --seconds 3000
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/extra --stage extra --story 1 --difficulty 4 --shot 11 --seed 0 --frames 350000 --seconds 3000 --diversity-cell 16
./build-headless/th08_hazard_probe --dat game_data_donottrack/th08.dat --stage 4b --spell 92 --difficulty 3 --seed 0 --event-update 393 --leads 393,360,320,280,240,200,160 --output reports/local/spell92-probe.json
./build-headless/th08_planner_report --root reports/local/campaign --fidelity reports/local/fidelity/summary.json --known-binary-sha256 01cdab0637be646f45d3e0db12734837a979aa5a8e218fc8dabc7e427f0171e8 --replay-dir reports/native/replays --output reports/native/planner_current_summary.json
```

`reference/` above is a placeholder for supplied pinned corpus files, not a runtime
dependency. `--case ID` selects an explicitly labeled subset but still validates the
complete corpus. Explicit non-108 fixtures exist for synthetic protocol tests and do
not count as a full gate. `--rows FILE --schema FILE` independently fingerprints an
existing capture. One Session step can perform multiple replay calculations; capture
rows and API call counts are checked separately.

The current measured corpus result is **106/108**, not the handoff's recorded 108/108.
`lunatic-1-b` ends early at 36200 rows; `lunatic-11-a` has all 116281 rows but a different
fingerprint. Both reproduce with O0/O3 and a fresh pinned full native Debug build;
restoring the draw chain does not fix them. Their retail-divergent row and cause remain
unknown. Do not relax the gate or rewrite reference fingerprints. Matching projection
fields do not establish every internal state or every input sequence.

Planner reports include native attempts/completed updates, failed rollouts, checkpoint
retry costs, options and capacity. New reports hash DAT and executable once before
forking, outside native timing. The first recorded comparison campaigns predate this
metadata addition; the retained spatial-planner binary is identified separately in
their evidence. That explicit composer hash is specific to those archived campaigns;
it must not be assigned to other builds. New campaigns use producer-recorded hashes.
Concurrent campaign wall times are not isolated single-core benchmarks. The current
remote campaigns use Linux x86_64, AMD EPYC 9654 and GCC 13.3 Release, with contraction
disabled. Local diagnostic probes use AMD EPYC 7B12/GCC 12.2 Release. Independent jobs
have task-specific scratch/output directories, low CPU/I/O weights, bounded memory
and no additional swap; the shared host's other workloads are not modified.

The ledger requires a finished process, matching fresh terminal/RNG/checkpoint and
zero deaths before counting a completion. Practice/story cases additionally require
a natural `.rpy`, matching calculation-row count and final score, and explicit zero
`death_rows`/`bomb_input_rows`. These counters count observed rows, not death/bomb
events. Refreshing them on the four practice tapes preserved every original row byte.
Replay calculation rows and Session API calls are separate cost totals.

For continuous story, five native stage-control calls can complete without a
gameplay calculation. The observer is beside `gameplayFrameCounter++`, after the
setup early-return gates; the earlier trace/RNG hook stays at its original boundary.
Reports bind `frames = calculation_updates + control_updates` to explicit ordered
stage-transition events, and natural replay rows must equal the observed calculation
count. The measured 232408-action route has exactly 232403 calculations, with control
calls at 24158, 56541, 95948, 138845 and 178264. This is measured source behavior,
not permission to subtract the number of stages from an arbitrary replay.

For older completed campaigns, retain the original plan and same-binary verification.
A separate `<case>-accounting/verify.json` supplies missing observations from a new
fresh run, bound to exact raw action SHA, source/DAT/configuration, terminal, score
and RNG. Its executable and costs remain separate. New plan/verification pairs must
agree directly on counts and events. `th08_replay_check` also hashes its actual replay,
DAT and executable before capture. `--replay-dir` archives only verified whole-scene
originals and full action tapes, checks the replay hash, and emits both artifact hashes
in the ledger. Previously published originals remain under
[native replays](../reports/native/replays/). The owner additionally requested the
two complete online 4A story replay/action pairs and their publication audit.
All other new replays, action tapes and generated JSON stay under ignored
`reports/local/`, together with fresh-verification copies
and retained raw rows. Code and maintained prose continue through progress commits.
Git newline conversion is disabled for `.actions` and `.rpy` so
platform checkout settings preserve the exact bytes bound by these hashes.

Completed remote captures may be retained as `.rows.gz`. The storage audit checks
every restored raw SHA before removing its uncompressed copy; use
`gzip -dc capture.rows.gz > capture.rows` before a raw-row comparison. Later
successful captures retain raw SHA/size and native replay receipts, while their
reconstructible raw rows are removed. Replay/action files and failed diagnostic
captures remain available. The [route environment audit](../reports/native/planner_route_environment.json)
records retained evidence, cleanup receipts and bounded shared-host resources.

For new batches, pass repeatable `--group NAME` arguments; each named directory must
exist. Explicit groups require `<case>-verify.exit` and, for whole scenes,
`<case>-rpy.exit` to be zero in addition to their successful JSON reports. Legacy
default groups remain reproducible without these previously unrecorded sidecars.
The suffixes `-verify`, `-rpy` and `-accounting` are reserved for validation sidecars.
Use a local `--failed-actions-dir`, as in the commands below, to retain each failed
attempt's final emitted tape with a hash, parsed NMNB input/count checks and the
role `failed_search_prefix`. These tapes are execution-unverified prefixes, not
successful demonstrations or all rejected internal rollouts. Existing artifact
names are idempotent only for identical bytes; conflicts fail without overwriting.

For full-run baseline batches, add `--matrix reports/native/planner_route_matrix.json`.
The report checks the complete generated catalog, each attempt's checkpoint and
every successful native/replay stage path before archiving, and requires validation
process receipts in this mode. Its `coverage` object
binds the manifest SHA and counts distinct route IDs across all selected groups;
retries retain their separate attempt costs. `expected_combinations` includes the
whole manifest, while `observed_combinations` includes only IDs present in the
input groups. A zero attempt-level pending count does not imply full coverage.
Omitting `--matrix` preserves the historical report format.

For sparse random evaluations, pass `--sample FILE` instead of `--matrix`. A sample
manifest has `schema_version: 1`, `sampling_unit: "seed-case-pair"` and an explicit
`cases` array using the catalog descriptors, with each case's own seed. It may mix
seeds and omit unselected combinations. Optional `sample_phase` is `diagnosis` or
`holdout`; when present, every case must carry the same phase. The report checks
the canonical case IDs, initialization profiles and full stage paths, binds the
sample SHA, and reports pair coverage separately from distinct-seed coverage.
`verified_unique_seeds` counts seeds with at least one verified selected pair;
use pair coverage to assess completion of every planned run for a repeated seed.
The two manifest options are mutually exclusive. A zero pending count does not
mean that every planned sample pair has been observed.

```sh
./build-headless/th08_planner_report --root reports/local/seed-campaign --fidelity reports/local/seed-campaign/fidelity-v1/summary.json --group training-c16 --group training-c32 --group holdout-c16 --group holdout-c32 --group training-resume-c16 --group training-resume-c32 --group holdout-resume-c16 --group holdout-resume-c32 --replay-dir reports/local/seed-campaign/archive/replays --failed-actions-dir reports/local/seed-campaign/archive/failed-actions --output reports/local/seed-campaign/planner_seed_summary.json
```

The current full-run ledger uses all four portfolio groups:

```sh
./build-headless/th08_planner_report \
  --root reports/local/route-matrix-v2/results \
  --fidelity reports/local/route-matrix-v1/results/fidelity-v1/summary.json \
  --matrix reports/native/planner_route_matrix.json \
  --group matrix0-v2-c16 --group matrix0-v2-resume-c16 \
  --group matrix0-v2-c32 --group matrix0-v2-resume-c32 \
  --replay-dir reports/local/route-matrix-v2/archive/replays \
  --failed-actions-dir reports/local/route-matrix-v2/archive/failed-actions \
  --output reports/local/route-matrix-v2/planner_route_summary.json
```

The first random diagnosis ledger uses its frozen 12-pair sample:

```sh
./build-headless/th08_planner_report \
  --root reports/local/seed-generalization-v2/results \
  --fidelity reports/local/route-matrix-v1/results/fidelity-v1/summary.json \
  --sample reports/local/seed-generalization-v2/diagnosis.json \
  --group random1-diagnosis-c16 --group random1-diagnosis-resume-c16 \
  --group random1-diagnosis-c32 --group random1-diagnosis-resume-c32 \
  --replay-dir reports/local/seed-generalization-v2/archive/replays \
  --failed-actions-dir reports/local/seed-generalization-v2/archive/failed-actions \
  --output reports/local/seed-generalization-v2/diagnosis-summary.json
```

The [execution protocol](../reports/native/runtime_perf_protocol.json) fixes three
complete archived tapes, an incomplete prefix and three alternating paired repeats.
The [audit](../reports/native/runtime_perf_summary.json) records every timing sample,
raw receipt hash and normalized execution contract. Its baseline is native source
`0215b7f`; the default cache candidate is `2b30ad1`, both GCC 13.3 Release with the
same floating-point flags. An independent IPO build uses the baseline source and
`-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`. IPO is not enabled by default or combined
with the measured cache gain.

Build each source snapshot into its own measurement directory. For example, the
seed-4455 full-tape workload on the current build is:

```sh
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/perf-seed4455 --stage 1 --story 1 --difficulty 3 --seed 4455 --verify reports/native/replays/training-resume-c16--s4455.actions
```

Use fresh output directories for each repeat and check process exits as well as the
JSON. Verification consumes the entire supplied tape; search-only option defaults
in its receipt do not cap that tape. Compare every execution field after excluding
only native timing and binary identity. Require identical frames, terminal, source,
DAT/action hashes, score, RNG and ordered calculation/control events. The incomplete
prefix must retain exit 2, `verified:false` and no complete terminal.

The cache build and the separate IPO build reproduce all 108 baseline case tuples
without changing the two reference failures. Additional raw captures of both failing
fixtures match byte for byte, as do all 3706547 calculation rows from the 16 new
seed routes. The additional training-only planning pair uses the original cell16
seed-4455 prefix and unchanged 900-second continuation profile; its full actions and
all deterministic plan fields match after excluding timing/producer identity.
Native timing excludes file I/O; shared-host scope CPU includes compilation and
validation, and recorded capture/API counts are separate units. These are execution
agreement checks, not independent physics or retail proofs.

One full source/adapter differential additionally plays the archived
`story-prefix-v1--p132200-r64-b384-c16.rpy` through the pinned full native Debug/O0
build, retaining production draw callbacks with private Xvfb, dummy audio and
llvmpipe rasterization disabled. All 232403 calculation rows (23240300 bytes)
match the headless capture byte for byte: SHA256
`c560ef53295b9a62e949e8fd79fd5c2c6b576d137e3398b108caa4252bb913ea`.
Both captures end naturally with score 169789855, zero death/bomb-input rows and
the same six-stage route. The full-native executable SHA256 is
`9aae4ce60d1a5e8426b1675b7fe7504e0a74f7ed70610ada5c4d692e5a456b00`.
The additional validation costs 232403 calculations and 356.01 seconds including
debugger, display setup and trace I/O, outside the solver campaign totals. This
checks that adapter/profile for one tape; the two paths share source and numerical
behavior, so it is not an independent retail physics oracle.

The export fix's full Release native suite passes 36/36 CTests in 950.99 seconds,
including complete real-data scenes. Earlier receipt hardening passed 35/35 in
751.96 seconds before the export regression was added. Concurrent host workloads
make these suite wall times unsuitable as isolated solver benchmarks. The current
full 108-fixture run retains exactly the prior row counts, fingerprints and outcomes:
106 pass and the same two failures remain. No expected fingerprint is changed.
After the repair utility and its additional contracts, all 35 tests excluding the
unchanged long scene pass again in 21.67 seconds; the core suite passes 29/29.

Prefix continuation is a new planning episode over a natively reconstructed world,
not a stage reset. Its raw input hash and reconstruction costs remain in the ledger;
the final full tape must replay from its original start. Contract checks cover
chunked/malformed/bomb inputs, explicit budgets and terminal boundaries. Native
checks cover successful continuation, exact-terminal completion with no rollouts,
trailing-action rejection, a death at update 382 and budget exhaustion before update 1.
`--verify` executes the entire supplied tape and records its actual native cost;
search frame/time limits apply to planning episodes. Bound verification separately
with a process timeout when running a campaign.

The updated upstream polar math also changes four Stage6b collision values. The named
`stage6b-02ff72f-semantic-v1` contract keeps the old `stage6b-semantic-v1` fixture intact,
pins the new source/geometry, and retains strict fresh replay and all 18 latch probes.
Neither contract asserts universal cross-host float identity.

### Retail replay export identity

The first Wine audit found a real export defect: archived native replays declared
`0100d` but serialized executable size/checksum as `0/0`. The native loader's
`TH08_MODERN_PORT` version bypass accepted them. Retail 1.00d instead failed its
`th08_0100d.ver` whitelist and crashed while listing the replay; its unterminated
last-line traversal reached address 1 in `strncmp`. A known accepted retail replay
loaded and played under the same Wine configuration. Native round trips alone had
therefore missed an incompatible file header.

Recording declares the pinned retail target explicitly: size `840704`, checksum
`2724749753`, version `0100d`. Reports keep that declaration separate from the native
producer's SHA256. It neither changes gameplay/RNG nor certifies retail execution.
`headless_replay_export_contract` first failed with serialized size zero, then passed
after the fix. It checks a complete fresh practice export, exact tape/replay outcome,
legacy repair row identity, idempotence, malformed input and refused output overwrite.

Preserve archived originals. To produce a separately named copy:

```sh
./build-headless/th08_replay_target reports/native/replays/story-v2--r64-b384-c16.rpy reports/local/story-retail-1.00d.rpy
```

This C++ tool accepts only a validated zero-identity or already-targeted 1.00d stream.
It changes the two decoded identity fields at `[0x124,0x12c)`, rebuilds compression
and the enclosing checksum, and refuses an existing output. All other decoded bytes,
stage offsets and trailing user data persist; already-targeted files copy byte for
byte. It reports hashes and never claims gameplay validation. The
[repair receipt](../reports/native/planner_story_retail_target.json) binds the original
to the separately saved
[retail-targeted copy](../reports/native/replays/story-v2--r64-b384-c16-retail-1.00d.rpy).
Both produce the same 232069 native calculation rows, SHA256
`ad7d90ea5d36904d75498c621f18d7be88ca4f42554eb9100ddd306a3eb1c3b5`.
Original archived replay hashes and the campaign ledger remain unchanged.

The separately recorded [retail audit](../reports/native/retail_story_validation.json)
plays this exact repaired file from Stage 1 through Final A in retail 1.00d under
Debian Wine 8.0. It observes the final stage-clear sequence, cumulative deaths/bombs
both zero, internal score `167865093` (display `1678650930`), and natural return to
the replay menu. The retail EXE SHA256 is
`330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`;
all 728064 loaded executable-code bytes match the original before and after playback.
No executable, gameplay memory or RNG patch is used.

This is a manual interoperability audit with read-only process sampling at a nominal
20 ms interval. The continuous sampler attached in Stage 2 at gameplay frame 14651;
the genuine Stage-1/frame-0 menu-start receipt is separate. Its original all-stage
coverage gate therefore reports `terminal_mismatch` (mask 118, missing Stage 1),
retained in the audit. Cumulative death/bomb counters persist through stage handoffs.
The separate terminal receipt observes the actual final clear and menu return;
replay's terminal branch does not set the normal game's `gameCleared` bit. It uses
`GUI_MSG_SHOW_STAGE_RESULTS` / `stageClearSequenceActive` instead. Retail also
continues fade calculations after the recorded input ends; these are not equated
with native capture rows. Recorded score and selected state restore at replay stage
handoffs, so matching terminal scores are not a complete physics differential.

To repeat the manual test, place an owned retail copy under ignored
`game_exe/retail-audit/game/`, put the repaired file in its `replay/` directory as
`th8_01.rpy`, and use a private 32-bit Wine prefix and display. Start windowed with
audio off, effects maximum, frameskip zero and slow mode off. Select Replay, the
first stage and the built-in Boss playback mode (2), which fast-forwards nonboss
sections without enlarging gameplay steps. The audit bounds its process group to
2 then 4 CPU cores, low CPU/I/O weight 10, nice 10, memory high/max 3/4 GiB and no
additional swap. Peak Wine-group memory is 429535232 bytes (about 410 MiB), with
zero memory-limit/OOM events. A prior black fullscreen startup was stopped before
the memory cap; that setup failure and the original header crash remain recorded.
The owned Wine/display processes are stopped after collecting terminal evidence.

Read-only probes are specific to the hashed x86 EXE: flags `0x0164d0b4`, current
stage `0x0164d2cc`, globals pointer `0x0160f510`, cumulative float deaths/bombs-used
at globals offsets `0x64/0x84`, and replay/title pointers `0x018b8a28/0x018bde08`.
No full per-frame retail projection or all-eleven-file retail validation is claimed.
Private raw game data, screenshots and diagnostic logs remain ignored.

The later online-story audit repeats the isolated retail procedure for the two
tracked shot-1 natural-4A replays. Start the read-only observer before confirming
Stage 1 so its first active sample covers frame 0. Derive the expected stage mask
from the frozen manifest: `[0,1,2,3,5,6]` yields `0x6f`; the earlier shot-0 4B route's
`0x77` is a different path. Preserve a failed observer report instead of rewriting
it. Adjudication must independently check the unchanged replay hash, manifest path,
logged cumulative death/bomb counters and reported sampled maxima, actual final clear sequence, expected
internal score, natural menu return and executable-code identity. Keep sampled
coverage distinct from a complete retail differential. A successful tape playback
does not establish an unpaused controller, deadline compliance or all-seed success.

## Local build and core checks

Requires CMake 3.16+, C++17 and OpenSSL development libraries. Linux x86_64 is tested.
Modern float32 with contraction disabled is the numerical profile; no fast-math.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
mkdir -p reports/local
```

The scenario CTest covers complete duration, seeded repeat/replay, phase/RNG continuity,
malformed checkpoints and the bounded-search counterexample/recovery. Planner tests
compare statuses, exact action/position bits, tie-breaking and attempted budgets with
the retained reference. Other tests guard their named component contracts.

## Reproduce the legacy subset evidence

Run serially for useful Release timings. Serialization occurs after timed execution.
Exit 0 means the report/check ran; inspect each outcome, since collision/search failure
is valid benchmark evidence and not a solved scene.

```sh
./build/th08_scenario_cases --scenario all --strategy all --seed 1 --duration 7200 --output reports/local/scenarios.json
./build/th08_scenario_cases --scenario all --strategy rolling-beam --seed 1 --duration 7200 --recover-goal 1 --output reports/local/recovery.json
./build/th08_scenario_cases --scenario relay --strategy rolling-beam --duration 72000 --recover-goal 1 --format tsv --output reports/local/long-relay.tsv
./build/th08_scenario_cases --scenario relay --strategy rolling-beam --visual-draws 0 --output reports/local/no-visual.json
./build/th08_spell_cases game_data_donottrack/th08.dat reports/local/id179 --verify-only
./build/th08_spell_cases game_data_donottrack/th08.dat reports/local/id179
./build/planner_bench > reports/local/planner_benchmark.json
```

ID179 verification checks exact events/terminal occupancy, RNG isolation, checkpoint
copy/resume and unknown-opcode rejection. The normal tool compares four strategies for
seeds 0, 1, 65535, storing twelve seed-qualified routes and a summary. It also checks illegal
and incomplete replay tapes. Raw assets are not extracted or committed.

The planner benchmark reference is the original sorting planner; its timing ratio
includes both heap selection and later early dedup improvements, not only the last
change. Report outcome/counters as well as time. Checked samples used Intel Xeon
Platinum 8573C, GCC 14.2, Release; timing is not portable or a guarantee.

## Exact input provenance

- DAT: 46,838,025 bytes, SHA256 `9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb`
- Component-oracle source: [N0zoM1z0/th08](https://github.com/N0zoM1z0/th08), pinned commit
  `a45e99fb1942714e6edded20847e32a654d56f97`
- Tracked native runtime: same upstream, branch `port/portable-64bit`, commit
  `02ff72f29e7b4928f21c5a2fb0f57ffa0cec5415`, adapted under `third_party/th08`.
  Earlier native reports identify the historical `861bec9` import and retain that profile
- Original preparation archive `TH08_AllCase_20260911.zip`: SHA256
  `8913fffc96824c27b681ff1b1133a4385f7c7ca3e8e377fede17e552e535b199`
- Older preparation reports mentioning a dirty worktree near `af72ca9` are historical
  evidence, not the pinned source or current runtime. `preparations/` remains
  unchanged locally and is excluded from published Git history

Legacy DAT analysis tools check the expected input hash. Native experiment tools
record the supplied DAT hash; compare evidence only for the stated input/profile.
Scenario reports retain relevant member/ANM/SHT hashes and instruction identities.
Source probe generators enforce per-file hashes before
extracting reviewed unchanged bodies; those maintained constants are the authoritative
file-hash list. Do not bypass a mismatch or use aggregate pass rates to hide it.

## Native headless reproduction and acceleration checks

The opt-in target requires SDL2, SDL2_image, SDL2_ttf and Fontconfig development
libraries (Debian/Ubuntu packages `libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
libfontconfig1-dev`). The CPU compatibility layer decodes resources; no video/audio
device or OpenGL renderer is initialized. No Python, Windows or Wine is used.

```sh
cmake -S . -B build-headless -DCMAKE_BUILD_TYPE=Release -DTH08_HEADLESS=ON -DTH08_HEADLESS_DAT="$PWD/game_data_donottrack/th08.dat"
cmake --build build-headless --parallel 2
ctest --test-dir build-headless --output-on-failure
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --spell-id 179 --difficulty 0 --seed 0 --strategy reactive --frames 2000 --actions reports/local/id179.actions --output reports/local/id179-native.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --spell-id 179 --difficulty 0 --seed 0 --frames 2000 --replay reports/local/id179.actions --output reports/local/id179-native-replay.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 195 --difficulty 4 --seed 0 --strategy spell-portfolio --frames 6000 --actions reports/local/id195.actions --output reports/local/id195-native.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 199 --difficulty 4 --seed 0 --strategy spell-portfolio --frames 5000 --actions reports/local/id199.actions --output reports/local/id199-native.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 201 --difficulty 4 --seed 0 --strategy spell-portfolio --frames 6000 --actions reports/local/id201.actions --output reports/local/id201-native.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 202 --difficulty 4 --seed 0 --strategy spell-portfolio --frames 6000 --actions reports/local/id202.actions --output reports/local/id202-native.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 204 --difficulty 4 --seed 0 --strategy spell-portfolio --frames 6000 --actions reports/local/id204.actions --output reports/local/id204-native.json
```

The real-data CTest runs Stage 1 to clear (22176 updates), ID179 through its original
wrapper/end (1292 updates, activation at 92) for seeds 0/1/65535, stationary collision
at 382, and retained/adapted boundaries for IDs 85, 89, 93, 198 and 201. IDs 85, 89,
93 and 198 complete for seeds 0/1/65535; ID201 preserves its generic WAIT collision at
362 and requires the seed-0 portfolio completion at 4292. Portfolio IDs 193/195/199
also run through their complete wrappers. ID202 retains its generic WAIT collision at
3265 and its WAIT-only portfolio collisions at 4204/4216/3613 for seeds 0/1/65535;
its original 12-update horizon and disabled future-ECL adapter are checked explicitly.
ID204 retains its generic relative-direction collision at 1205, requires complete
4832-update wrappers for seeds 0/65535, and preserves seed 1's collision at 3008.
ID32 preserves its generic 594 collision, checks the explicit 13-update/one-update-leg
profile and 192132 candidate evaluations for each complete 2372-update seed wrapper,
and runs Stage 3 through both its 7443 baseline collision and 39767 clear.
ID167 retains its generic 932 collision, requires all three 5492-update wrappers
with H12/four-update first leg and 444852 candidates. ID183 retains its generic 1041
collision and requires three complete 1292-update wrappers with H32/four-update leg
and 104652 candidates. It owns the single current continuous Stage6b gate: clear58853.
ID139 preserves its generic 5326 collision, requires all three 7292-update seed
wrappers, and runs Stage 6a through both its 48302 baseline collision and 61041 clear.
The native laser-item executable also checks 18 cancellation geometry cases, including
both source cancellation paths, angle quadrants, 32-unit spacing, exclusive end bounds,
item suppression and off-playfield X rejection.
A 10-update budget failure and the Stage 6b
reactive collision at update 854 remain covered.
Every execution tape replays in another process;
tests also reject unsupported input, excess tape and wrong-ID wrapper selection.
Generated tapes and `summary.json` are under `build-headless/headless-regression/`.
Without `TH08_HEADLESS_DAT`, these private-data tests are not registered.

The focused ID202 group can also run through the same generator, preserving its
baseline and all three failed seed wrappers rather than declaring the spell solved:

```sh
cmake -DEXECUTABLE="$PWD/build-headless/th08_headless" -DCOMPARE_EXECUTABLE="$PWD/build-headless-o0/th08_headless" -DDAT="$PWD/game_data_donottrack/th08.dat" -DWORK="$PWD/reports/local/id202-check" -DCASE_GROUP=id202 -P tests/headless_real_data.cmake
```

The 2026-10-02 Intel Xeon Platinum 8573C/GCC 14.2 run passed all 26 core tests and
the four focused ID202 scenes with fresh replay and native O0/O3 agreement. The normal
aggregate originally failed the unconditional historical Stage6b digest assertion:
this host produces 6279671846274327225, versus historical 4060221407534777929. Clean-main
and O0/O3 replay reproduce the current digest, while the 854/slot664 boundary, complete
collision object, RNG and feedback agree. The older profile did not identify solver
build/libm/CPU sufficiently to claim cross-environment float-bit equality. The precise
intermediate difference is not diagnosed.

The stage6b-semantic-v1 contract now guards DAT/profile/scene, terminal, RNG, feedback
and every collision field instead. It retains the old digest/report and emits
historical_digest_match plus numerical_profile_match=unverified; there is no compiler
whitelist or replacement digest. Strict current-run replay/O0/O3/diagnostic equality and
all 18 input-latch probes are unchanged. Negative DAT, RNG (including malformed fractional
integers), collision and replay mutations fail. This loses the unsupported historical
intermediate-trajectory oracle, so cross-host float drift outside the named boundary
is not proven absent. The historical aggregate remains an immutable provenance record.
`CASE_GROUP=id204` similarly runs the four ID204 fixtures and emits `id204-summary.json`.
Its baseline and seed-1 failure, as well as both complete seed wrappers, agree in fresh
processes and at native O0/O3 on this host. Both group selectors use the same scene/replay
checks as the full aggregate, whose historical digest remains provenance, with current-run equality still strict.

`CASE_GROUP=id32` runs the six ID32 wrapper/Stage3 cases and emits `id32-summary.json`.
All three seed wrappers and both baseline/complete stage tapes agree at O0/O3.
`CASE_GROUP=id167` now runs four wrapper fixtures and emits `id167-summary.json`.
The tracked five-case ID167 report is retained as preceding-checkpoint evidence,
including the then-genuine Stage6b ID183 collision. `CASE_GROUP=id183` runs five
current fixtures and emits `id183-summary.json`, including the new Stage6b clear.
All checked O3/O0 replays match; the full stage runs once in the aggregate.
For ID183 seed 0 on this GCC14 host, the complete 1292-update H32/81-path wrapper
uses 8894.1 ms of decision time; the failing 1041-update baseline uses 613.6 ms. These
different durations are not an equal-work performance comparison. Both exclude file
I/O and make the additional planning cost explicit.
`CASE_GROUP=id139` runs six baseline/complete scenes and emits `id139-summary.json`.
The checked report includes fresh O3 replay and separate O0 replay for every tape.
Pass `-DLASER_ITEMS_EXECUTABLE="$PWD/build-headless/th08_headless_laser_items"` to
include the focused native cancellation regression (the normal CTest already does).
An empty native `fsincos` stub previously caused undefined item geometry and an O0/O3
score difference at Stage 6a update 32432. Assigning both established sin/cos outputs
makes the long baseline and complete-stage tapes agree again: corrected digests are
`3641853114352264503` and `6502928606160698451`. The 18 geometry cases fail with both
pre-fix O0/O3 archives and pass after repair. This does not resolve or replace the
separate retained Stage 6b cross-host golden.

Reproduce the Stage 6b failure and compare the nine input directions locally:

```sh
mkdir -p reports/local/stage6b
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --difficulty 0 --seed 0 --strategy reactive --frames 50000 --actions reports/local/stage6b/baseline.actions --trace reports/local/stage6b/baseline.tsv --output reports/local/stage6b/baseline.json
./build-headless/th08_headless_probe --executable build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --difficulty 0 --seed 0 --actions reports/local/stage6b/baseline.actions --frame 854 --output-dir reports/local/stage6b/late
./build-headless/th08_headless_probe --executable build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --difficulty 0 --seed 0 --actions reports/local/stage6b/baseline.actions --frame 853 --through-frame 854 --output-dir reports/local/stage6b/early
```

The baseline intentionally exits 2. It hits fired, untransformed bullet slot 664:
player bounds `[373.174988,431.174988,374.825012,432.825012]` overlap hazard bounds
`[371.920898,428.774048,375.920898,432.774048]`. Movement consumes input 133 from
update 853, while update 854 samples 4165. Nine replacements at 854 all collide;
replacing 853 with left/up-left/down-left survives the observed update 854. The
real-data CTest guards these witnesses and verifies diagnostics leave the current
run’s trace unchanged (the historical 4060221407534777929 is separately labeled), and compares each branch tape at O0/O3 when
configured. It does not assert a whole-stage solution or an independent physics oracle.

The probe serializes its nine fresh processes and writes tapes, per-branch JSON/TSV
and `summary.json` into the output directory. It checks the projection at the common
prefix, rejects an out-of-tape frame and removes an old summary before a rerun. It
records actual total replayed updates, process wall time including initialization,
output and cleanup, and per-child maximum RSS in KiB. Local runs use approximately
73 MiB per child; use `ulimit -v 2097152` for a 2 GiB address-space cap if desired.
No remote host is needed. Repeated prefix work is explicit; no unsafe native snapshot
or shared action-dependent future is introduced to hide that cost.

The maintained seed-0 sweep enumerates 56 Easy standard/Extra practice checkpoints.
The Release `spell-portfolio` run completed 54 and retained 2 genuine collision
prefixes: bullet IDs 202/203. Bounded native-prefix repair additionally completes both,
so the combined documented recipe solves 56/56. All 56 rolling tapes
then agreed in a fresh process on the semantic fields used by `agree()` above. This is
broad algorithm evidence, while the smaller real-data CTest keeps the affected
transform/profile boundaries practical to rerun on every local change.
The ID204 change was compared with its preceding main checkpoint across all 56 cases:
the other 55 semantic reports and action tapes remained unchanged, and both versions'
tapes freshly replayed. Continuous Extra also remained at its earlier ID192 collision
on update 7424 at that historical checkpoint. The later ID192 change advances Extra below.

The ID139 comparison runs the previous portfolio and the new isolated profile on the
same corrected laser-item runtime: 49 versus 50 complete spells. All 55 unaffected
semantic records and literal action tapes agree; all 112 tapes freshly replay. Stage
6a changes from its preserved 48302 collision to a 61041 clear. Older aggregate
reports retain their original numerical/runtime profile rather than silently receiving
corrected item scores or digests.

The subsequent ID32 comparison changes only ID32 among 56 spells (50 to 51 clears):
all 55 unaffected semantic records and literal action tapes agree, and all 112 tapes
freshly replay. Continuous Stage3 changes from collision7443 to clear39767. The new
13-update horizon and 81 one-update-leg paths are isolated to ID32; generic projection
and tie-breaking are unchanged.

The subsequent ID167 comparison changes only ID167 among 56 spells (51 to 52 clears):
all 55 unaffected semantic records and literal action tapes agree, and all 112 tapes
freshly replay. Continuous Stage6b advances from ID167 at 42560 to the retained ID183
collision at 54401. This does not increase the stage-clear count.

The subsequent ID183 comparison changes only ID183 among 56 spells (52 to 53 clears):
all 55 unaffected semantic records and literal action tapes agree, and all 112 tapes
freshly replay. Continuous Stage6b advances from its preserved ID183 collision54401
to clear58853, adding the eighth complete stage. The prior ID167 report retains its
historical failure; the current single Stage6b fixture belongs to ID183.

The subsequent source/body comparison uses a complete separately compiled source snapshot
of preceding main 54f926d (original headers, Session, CLI and native units), rather than
an old-adapter/new-layout hybrid. It changes 53 to 54 clears, preserving all 53 successes.
Only ID192 and the still-failing ID203 prefix differ; 54 other semantic records and
literal tapes agree, including IDs201/202. Both versions of all 56 spells and Extra
freshly replay (114 tapes). Extra advances7424→67738, stopping in ID202.

ID192's focused five-record report preserves baseline 466, complete seeds 0/65535 at 3692,
seed 1 collision 1470 and actual Extra 67738. All agree with fresh O3 and O0 replays.
The H12/81-path profile costs 299052 candidates and 11074.8ms decision time for seed 0,
versus 4194 candidates/84.5ms for the 466-update baseline; these are unequal workloads.
At preceding checkpoint bd678, ID203's four-record report retained baseline 1711
and failures 1711/4823/2896 with bounded WAIT/vector projections; the current five-record
report below supersedes that partial evidence.
Native-linked regressions check WAIT/vector activation and final-clear boundaries,
DESPAWN's last lethal movement, 27 body source/lifecycle/clock guards, native movement
containment and the reciprocal-multiply size cancellation boundary. Both O0/O3 pass.
Independent native differential checks cover 23976 WAIT/vector updates and complete
ID192 tapes, including continuous Extra's delayed EndSpell-immunity transition.
That preceding bd678 normal CTest took 401.05s: 26 core tests passed; its native aggregate ended
at the unchanged historical Stage6b golden assertion described below, not a new gate.

The subsequent ID203 change keeps the rolling portfolio at 54/56 and adds one complete
seed 0 wrapper through bounded native-prefix repair. Compare against a complete separately
built bd678 source snapshot: the other 55 spell tapes/semantic results and continuous
Extra remain unchanged; the new rolling ID203 fails 787. Every comparison tape freshly
replays. The repaired 5492 tape is a separate verified outcome with full search costs,
not a replacement of the rolling failure record or a hardcoded route.
The final search used 91 candidates and 138198 native candidate updates, including 119908
replayed-prefix updates, in 384041ms. Initial execution used 787 updates/3919.03ms; the
built-in final replay used 5492 updates/875.446ms. Zero interrupted updates were charged.
The selected tape digest is 2771752472810487521 and independently agrees at native O0/O3.
The preceding f372 normal CTest took 799.97s and stopped at the old unconditional
Stage6b digest assertion. The latest expanded protocol suite
also passes separately. These host timings include process setup/artifacts and concurrent verification; they are
not the candidate policy's decision-only time or a performance guarantee.

The optional `headless_repair_protocol` test uses an explicitly synthetic child process
to reject malformed/duplicate/trailing JSON, wrong scene/prefix/hold/exit artifacts,
existing output directories and a TERM-ignoring timeout. It checks conservative charges
for semantically invalid children. Native policy tests check the pending-input step,
all nine first-action families, deterministic ties and 16533-expansion bound. The real-DAT
ID203 group checks baseline 1711, rolling 787 for three seeds, strict replay/prefix control,
and the complete bounded search with fresh O3/O0 replay. Its public JSON records include
child/repair executable SHA256 identities; `source_revision` remains the imported native
runtime provenance and does not identify a planner build. Historical numerical-profile limitations remain explicit under the v1 semantic contract.

`CASE_GROUP=id203` accepts `-DREPAIR_EXECUTABLE=$PWD/build-headless/th08_headless_repair`
to include the search (the normal CTest supplies it). The tracked five-case report
reuses that frozen successful search, then freshly replays its selected tape at O3/O0;
original search costs are retained. Optional `REPAIR_RESULT_DIR` requires both recorded
executable hashes to match and a complete result, and labels `search_reused`; fresh search
remains the default. Reproduce the complete solve with:

```sh
./build-headless/th08_headless_repair --executable build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 203 --difficulty 4 --seed 0 --frames 15000 --output-dir reports/local/id203-new-search
```

The output directory must not already exist. `summary.json` separates initial,
candidate/prefix and final-replay work; `processes.tsv` and `process-costs.json` retain
process outcomes and conservative unverified-update bounds. A budget-limited or malformed
run is not completion. All failed candidate reports/tapes remain available for diagnosis.

### Easy stage sweep

The 2026-10-02 local sweep used every supported stage entry, Easy, seed 0, the Release
O3 native profile and a 100000-update cap. Stage mode keeps
shooting enabled and advances original message scripts. Each execution wrote an action
tape and every tape agreed in a fresh process on the semantic fields and collision object
used by `agree()`:

```sh
mkdir -p reports/local/easy-stage-sweep
for stage in 1 2 3 4a 4b 5 6a 6b extra; do
  ./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage "$stage" --difficulty 0 --seed 0 --strategy spell-portfolio --frames 100000 --actions "reports/local/easy-stage-sweep/$stage.actions" --output "reports/local/easy-stage-sweep/$stage.json" || [ $? -eq 2 ]
  ./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage "$stage" --difficulty 0 --seed 0 --frames 100000 --replay "reports/local/easy-stage-sweep/$stage.actions" --output "reports/local/easy-stage-sweep/$stage-replay.json" || [ $? -eq 2 ]
done
```

| Stage | Outcome | Updates | Blocking spell | Decision ms | Execution ms | Peak bullets |
|---|---:|---:|---:|---:|---:|---:|
| 1 | complete | 24135 | - | 1828.4 | 4009.8 | 258 |
| 2 | complete | 32448 | - | 1581.4 | 3370.9 | 293 |
| 3 | complete | 39767 | - | 2195.0 | 3957.6 | 294 |
| 4a | complete | 22089 | - | 7354.6 | 11400.8 | 528 |
| 4b | complete | 43700 | - | 2675.4 | 5216.4 | 548 |
| 5 | complete | 43348 | - | 7025.9 | 10772.8 | 665 |
| 6a | complete | 61041 | - | 27644.8 | 33844.9 | 1201 |
| 6b | complete | 58853 | - | 22694.2 | 27796.8 | 1199 |
| extra | collision | 67738 | 202 | 32346.5 | 41409.4 | 1536 |

The corrected-runtime stage records, including the subsequent ID32 Stage3 and ID167/183
Stage6b and ID192 Extra reruns, cover 393119 updates and eight clears. All nine tapes freshly replayed.
These Intel Xeon Platinum 8573C/GCC 14.2 samples were run
alongside other verification; timings are workload records, not performance guarantees.
The earlier AMD EPYC/GCC12 five-clear sweep is superseded for current coverage. The
older 22176-update Stage 1 CTest uses the different `reactive` action tape and remains
a separate baseline.

Acceleration removes wall-clock waiting and presentation work while retaining every
original calc-chain update, timer increment and shared RNG consumer in that chain.
The virtual clock depends only on executed update count, never CPU throughput. Do not
accelerate by multiplying dt, subsampling actions/collisions, shortening spell timers,
or sharing action-dependent futures. These change the experiment's semantics.

Compare a second native library optimization level against the exact same tapes:

```sh
cmake -S . -B build-headless-o0 -DCMAKE_BUILD_TYPE=Release -DTH08_HEADLESS=ON -DTH08_HEADLESS_OPTIMIZATION=0
cmake --build build-headless-o0 --parallel 2 --target th08_headless
cmake -S . -B build-headless -DTH08_HEADLESS_COMPARE_EXECUTABLE="$PWD/build-headless-o0/th08_headless"
ctest --test-dir build-headless --output-on-failure -R '^headless_real_data$'
```

`TH08_HEADLESS_OPTIMIZATION` controls the imported game translation units (0..3),
not planner budgets or frame semantics. The checked GCC 12.2 O0/O3 tapes match all
reported semantic fields and every frame's diagnostic projection for these cases.
This checks acceleration within the native profile, not every spell or retail x87.
The original calc chain includes collision/feedback; the policy's extrapolation is
only a proposal, never the acceptance oracle.

Reports separate `simulation_ms` (calc-chain/queue updates minus timed native file
open/read/write/seek/stat/close wrappers), `file_io_ms`, `decision_ms` (observation and
policy/tape selection), `diagnostics_ms` (optional ring copies), and `execution_ms`
(whole loop including projection/trace work and I/O).
Initialization, DAT hashing, tape loading, output serialization and cleanup are outside
these loop measurements. CPU decoding during an update remains simulation work; compare
the same profile/tape. Run serially and record hardware/compiler/outcome/budget alongside
timing. The checked native host is Linux x86_64, AMD EPYC 7B12, GCC 12.2, Release.

Fresh-process replay checks execution agreement and RNG/feedback, not an independent
physics oracle. Original runtime globals are not yet branchable solver checkpoints.
Keep the scope in [Scenarios](SCENARIOS.md) when interpreting complete-scene reports.

## Optional pinned-source comparisons

Use a fresh ignored checkout directory; do not repurpose an existing working copy
that may contain local changes. Adjust the path below if `.cache/th08` already exists.

```sh
git clone https://github.com/N0zoM1z0/th08.git .cache/th08
git -C .cache/th08 checkout a45e99fb1942714e6edded20847e32a654d56f97
cmake -S . -B build-reference -DCMAKE_BUILD_TYPE=Release -DTH08_REFERENCE_SOURCE="$PWD/.cache/th08"
cmake --build build-reference --parallel 2
ctest --test-dir build-reference --output-on-failure
cmake --build build-reference --parallel 2 --target source_enemy_motion source_world_motion source_camera_particle source_spawn
./build-reference/source_spawn
```

The other three opt-in targets isolate comparisons already in `source_oracle`.
`source_spawn` separately compares 6000 allocation/post-store transactions with a
controlled immediate-ECL boundary. `source_effect_pool` checks effect allocation and
callbacks; passing DAT explicitly adds actual ANM/shared-pool cases. They do not execute
a complete original game world.

The source comparison chain covers collision predicates, all-seed RNG/selected ECL
random assignments, launch modes, fractional direction/acceleration clocks, transforms,
laser lifetime, slot selection, ANM scalar/control execution, enemy/world motion and
timeline behavior. Thin test adapters are scaffolding, not reconstructed complete actors.
Context layout/template ownership is also source-inspected and checked by native tests.

Native source equivalence is not retail executable, x87 or Direct3D equivalence. Fresh
replay shares the simulation implementation and therefore is not an independent physics
oracle. No original executable or running game was used for these results.

## Other retained tools and report maintenance

`th08_audit`, `th08_slices`, `th08_motion_cases`, `th08_animation_cases` and
`th08_timeline_cases` accept DAT and an output directory. They report structural or
restricted/component evidence, not whole spells. The two motion fixtures remain
useful regression baselines. `th08_first_spell` and entry/camera/effect tests are older
optional diagnostics, not mandatory solver checkpoints or the current roadmap.

Use `reports/local/` for experiments. Refresh only affected tracked outputs in
`reports/native/`, preserving their scope labels and input identities. Investigate
changed hashes, event counts, digests, routes or outcomes before publishing; timing-only
variation is expected. Preserve 64-bit digests as strings/integer-safe data.

The documentation CTest checks maintained local links/headings, selected report/status
counts and CMake's core-test count. It does not prove semantic correctness or inspect
historical preparations. Keep README, Status, Scenarios and generated evidence consistent.

Generated source-oracle translation units belong only in ignored build directories.
Their maintained generators share `source_probe_support.*`, not another tool's main().
Game DAT/EXE/assets are not distributed. Keep [third-party notices](THIRD_PARTY_NOTICES.md)
and the original preparation attribution; do not assign those artifacts a new license.

### Restored full verification (2026-10-03)

The H12/H32 repair adapter derives half-horizon holds from the selected failure's
homogeneous terminal unsafe run. It does not relabel ID202 as H32. The rebuilt
GCC14 O3/O0 run completes ID202 at 4712: 24 candidates, 102001 candidate updates,
100602 replayed-prefix updates, digest `7360258866840071988`. ID203 reproduces
5492 with 91 candidates and 138198 candidate updates. Both reports preserve their
rolling failures and independently replay at O0/O3. These two bounded repairs plus
the unchanged 54 rolling successes establish 56/56 seed-0 checkpoint recipes.
The fresh full CTest run passed 29/29 in 732.98 seconds, including native real-DAT,
O0 comparison and the historical Stage6b semantic-contract/18-probe checks.
Earlier timing paragraphs are historical measurements, not this run's timings.

The optional broad sweep now has a maintained producer. It records compact current
results, literal tape hashes and native executable identity while retaining the
full reports/tapes in the local output directory:

```sh
cmake -DEXECUTABLE="$PWD/build-headless/th08_headless" \
  -DDAT="$PWD/game_data_donottrack/th08.dat" \
  -DWORK="$PWD/reports/local/full-sweep" -P tests/headless_sweep.cmake
```

Its fixed 56-case rolling sweep and nine actual stage entries are independent of
the repair searches. A collision remains a collision in this report; adding two
isolated repair successes does not imply continuous Extra is complete.

A stage-sized Extra episode explicitly uses 144 candidates, 14400000 candidate
updates and 1800 search seconds, not the smaller isolated-spell budget. The first
fresh run used 9809223 candidate updates and 872386 ms of search, reaching ID203 at
69227 (`496550657910513129`, fresh O0 replay agrees). It is a failure, not stage clear.
To continue only an improved validated frontier without repeating earlier branches:

```sh
./build-headless/th08_headless_repair --executable "$PWD/build-headless/th08_headless" \
  --dat "$PWD/game_data_donottrack/th08.dat" --stage extra --difficulty 0 --seed 0 \
  --shoot 1 --frames 100000 --seconds 1800 --update-budget 14400000 \
  --resume-search "$PWD/reports/local/extra-episode1" \
  --resume-producer /path/to/exact/previous/th08_headless_repair \
  --output-dir "$PWD/reports/local/extra-episode2"
```

Keep the previous producer binary when rebuilding the search tool. Resume verifies
its hash, copies the selected artifacts, reconstructs the selected world afresh,
and reports prior/new/cumulative costs. An unchanged frontier is rejected rather
than silently starting the same finite proposal family again.

### Final seed-0 completion

The resumed Extra episode completes the actual carried-world stage at 92339,
ending after ID204, digest `15529738245795540729`. Its 87 candidates cost 6078636
native updates, with 6040785 repeated-prefix updates. Fresh source regeneration
used 69227 updates; final replay used 92339. Across both episodes the verified total
is 16117163 updates, with no interrupted uncertainty charge, and about 1591.15 seconds
of managed process wall time. Independent O0 replay agrees; those additional validation
costs and broad-sweep runs are separate from the managed episode totals.
The final integrated normal CTest passes 29/29 in 692.31 seconds.

After the rolling sweep and repaired searches, generate the current combined report:

```sh
cmake -DEXECUTABLE="$PWD/build-headless/th08_headless" \
  -DCOMPARE_EXECUTABLE="$PWD/build-headless-o0/th08_headless" \
  -DDAT="$PWD/game_data_donottrack/th08.dat" \
  -DSWEEP_DIR="$PWD/reports/local/full-sweep" \
  -DSPELL202_DIR="$PWD/build-headless/headless-regression/id202-repair" \
  -DSPELL203_DIR="$PWD/build-headless/headless-regression/id203-repair" \
  -DEXTRA_DIR="$PWD/reports/local/extra-episode2" \
  -DWORK="$PWD/reports/local/completion-verification" \
  -DOUTPUT="$PWD/reports/native/headless_current_summary.json" \
  -P tests/headless_completion.cmake
```

This verifies three repaired tapes afresh at O3/O0, checks binary/data identities,
and embeds the rolling failures, prior Extra search, hash-bound lineage, successful
searches and replay reports. The primary totals are 56 checkpoint recipes and nine
continuous stage recipes. The nested rolling-only totals remain 54/56 and 8/9.

### Multi-seed robustness baseline (2026-10-03)

The requested additional seeds were fixed before inspection: 1, 42 and 65535.
Each ran the same 56-checkpoint plus nine-continuous-stage rolling matrix and
frame caps, without automatic repair searches. All 195 fresh replays agree.
Use `-DSEED=<value>` with `tests/headless_sweep.cmake`; the default remains zero.

| Seed | Rolling spell clears | Rolling stage clears | Native updates including replay | Decision time, all 65 runs |
|---|---|---|---|---|
| 0 | 54/56 | 8/9 | 1160236 | 147.16 s |
| 1 | 51/56 | 7/9 | 1046086 | 113.64 s |
| 42 | 48/56 | 6/9 | 934060 | 100.44 s |
| 65535 | 50/56 | 5/9 | 916710 | 102.17 s |

These are rolling-only comparisons, not comparisons with seed zero's additional
repair wins. Earlier failed termination reduces work, so lower elapsed time is not
an efficiency improvement. Seed zero also ran alongside other verification;
these host timings are descriptive, not isolated performance benchmarks.

Retained spell failures (ID@update):
- Seed 1: 192@1470, 200@4161, 202@4216, 203@787, 204@3008
- Seed 42: 147@742, 192@727, 194@907, 200@1018, 201@2317, 202@4197, 203@787, 204@3008
- Seed 65535: 119@1511, 127@3964, 194@3236, 201@700, 202@3613, 203@787

Continuous-stage failures expose carried-world differences: ID127 in Stage6a
(seeds 1/65535), ID155 in Stage6b (42/65535), direct/imminent lasers in Stage4b
(42/65535), and different Extra failures. An isolated successful spell is not a
promise that the same spell clears inside a stage. ID201 seed 1 now completes on
this current native build; the earlier 3455 failure is historical evidence.

A diagnostic-only global H32 control, with the same nine focused constant paths,
avoids four local ID127/155 wall traps from identical replayed prefixes. It also
creates regressions: seed zero ID108/147 hit enemy bodies and ID175 hits an
unsupported polar transform. It is rejected as a general fix. No seed/frame or
new spell-ID branch is introduced to hide those failures. Seeds 7 and 2026 remain
reserved, uninspected holdouts until a justified candidate is frozen.


### Paused transform-contract experiment

The owner requested a tidy, published stopping point. The active source remains
at the reviewed body-observation/multi-seed checkpoint; the following experiment
was rejected rather than silently changing failure fixtures or tuning seeds.
The existing seed-zero completion report remains historical evidence for its
recorded native binary, not a newly regenerated search under this experiment.

Native-linked probes established three forecast defects:
- A VECTOR clearing at update 2 followed by a blocked POLAR produces x=2,4,8;
  the old unbounded vector projection gives 2,4,6. An allowed concurrent POLAR
  already changes the first update from predicted x=2 to actual x=4
- Native float recurrence gives x=8.480000496 at update 32 for vx=.1, ax=.01;
  the closed form gives 8.479999542. Exact float equivalence is not established
- WAIT timer 2 with the extra timer step and a subsequent POLAR gives x=1,2,6,
  whereas the nominal three-update linear certificate incorrectly gives x=1,2,3

The draft adds bounded VECTOR recurrence, explicit supported-clock checks and an
inactive-program linear certificate. It does not enable generic WAIT/POLAR or H32.
Focused core tests pass 26/26 and native bounds checks pass O3/O0 before the final
INT_MAX safe-prefix clamp; that final clamp has a focused O3 pass. The full normal
suite was **not** completed for this draft. Its old ID202 boundary assertion failed
because rolling collision moved from 4204 to 4212; it was not rebaselined.

The already-started four-seed, 65-route sweep completed with fresh replay agreement
for all 260 routes. Compared with the published rolling baseline:

| Seed | Draft spell clears | Draft stage clears | Published spell/stage clears |
|---|---|---|---|
| 0 | 54/56 | 8/9 | 54/56, 8/9 |
| 1 | 51/56 | 7/9 | 51/56, 7/9 |
| 42 | 47/56 | 6/9 | 48/56, 6/9 |
| 65535 | 49/56 | 5/9 | 50/56, 5/9 |

New failures are ID195 seed42 at 2752 and ID192 seed65535 at 2631. The unchanged
seed-zero bounded repair budgets still complete ID202 (4712 updates, six candidates,
25764 candidate updates) and ID203 (5492 updates, 77 candidates, 85144 candidate
updates), with built-in fresh replay. Independent O0 comparison of these draft
repair tapes was not run. No repaired Extra search or new completion claim follows.

[Compact results](../reports/experiments/transform-contract-rejected.json) record
binary/report hashes, failures, costs and verification limits. The accompanying
[archived patch](../reports/experiments/transform-contract-rejected.patch) applies
to fdc8f8fb0ccd4119e7c9cf0b17b5eb84961a9337, but is not compiled or applied.
The sweep binary precedes its final INT_MAX clamp; that edge change must not be
mistaken for a fully swept final binary. Do not apply it without integration review
and the same regression matrix. No DAT, extracted assets or action tapes are included.

Remaining work is bounded and separate: fix the demonstrated transform contracts
without losing successful scenes; cover lethal body motion only with valid
lifecycle/child/timeline certificates; improve the two late laser warnings without
hardcoded frames or unsupported geometry. Global H32 remains rejected. Holdout
seeds 7 and 2026 remain uninspected, and work is paused until requested again.
