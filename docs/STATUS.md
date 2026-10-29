# Current handoff status

Updated October 29, 2026. Scoring is paused after the owner's batch handoff.
The current task is documentation/code maintenance, with no new scoring episode
or automatic continuation. Project targets remain **4 billion Extra** and
**7 billion complete Lunatic**; both are unmet. The 6.5 billion milestone is achieved.

All shots are permitted, with Scarlet Team preferred. The complete team story
route is **1 → 2 → 3 → 4A → 5 → Final B**. Current scoring constraints are No Miss /
No Bomb (NMNB), with captures optional. Strict all-capture CLI defaults remain
available. Wine online solving and remote search remain paused.

## Verified results

| Route | Shot | Best Native score | Captures | Producer / fresh / natural tasks |
| --- | --- | ---: | --- | --- |
| Extra | Scarlet — Sakuya/Remilia | **3,308,826,050** | 14/14 | 2047 / 2096 / 2097 |
| Complete Lunatic | Scarlet — Sakuya/Remilia | **6,653,194,550** | 36/37; card 10 times out | 2113 / 2115 / 2116 |
| Extra | Magic — Marisa/Alice | 3,013,913,970 | 14/14 | 597; full lineage in Validation |
| Complete Lunatic | Magic — Marisa/Alice | 5,550,599,270 | 36/37; card 47 times out | 620; full lineage in Validation |
| Complete Lunatic, all captures | Magic — Marisa/Alice | 5,107,553,670 | 37/37 | 575; full lineage in Validation |

All are complete NMNB runs with original-entry fresh execution and natural replay
agreement. The [51 selected progress replays](../reports/native/README.md) retain
exact bytes, original seeds and input lineage. Their
[progress index](../reports/native/replays/progress.tsv) owns replay/action hashes,
constraints and producer identities. [Validation](VALIDATION.md) owns reproduction
and verification scope. These checks do not independently prove retail equivalence.

The verified Scarlet Extra uses the original retail seed 9098, preserves task
2035's first 70,754 inputs and changes the card 205 search to beam 64. The verified
story uses the original retail seed 32796, preserves task 2095's first 156,833
inputs and changes only card 191 to beam 64. Maps 2100 and 2117 verify their actual
owner-input boundaries and complete audits. Some successful scene profiles remain
frozen local experiments; public-main search is not claimed to regenerate them.

Original Wine playback has a separate scope: Scarlet Extra **3,179,193,860**,
NMNB 14/14, passes original-game observation 1343 and corrected typed analysis
1367. The earlier decoder failure remains recorded. Newer Native results do not
inherit this validation. Earlier Magic online clears are Extra 1,166,969,620
(14/14) and complete Lunatic 2,189,965,070 (33/37), both NMNB. See the
[recording provenance](VALIDATION.md#selected-online-recordings-and-replay-provenance).

### Score targets and historical references

The **October 27, 2026 snapshot** records all-shot human references of
Extra **3,178,460,830** (Sakurei, solo Youmu, June 21, 2022) and Lunatic
**7,053,332,460** (IwanaM, solo Youmu, July 14, 2026). These are dated references,
not a claim about today's live leaderboard. [Validation](VALIDATION.md#october-27-verified-native-progress)
retains the source and declared comparison rules.

Native Extra is 130,365,220 above that reference and 691,173,950 below its
4 billion target. Complete Lunatic is 346,805,450 below 7 billion and 400,137,910
below the retail replay gate. A score of 7 billion alone does not pass that gate.

Earlier same-shot references remain distinct: Magic ASL Extra 3,021,639,920
(one bomb), Alan Lunatic 6,168,365,790, Scarlet ASL Extra 3,000,284,220 and Levea
Lunatic 6,755,587,090. They have their own constraints and are not NMNB upper bounds.

## What has been explored

The [experiment index](experiments/README.md) links hypotheses, source/human evidence,
parameters, failures, costs and decisions. The [lifecycle ledger](experiments/campaign-ledger.tsv)
preserves every allocation and correction; [per-task results](experiments/results.tsv)
preserve available measurements and provenance. Empty fields mean unrecorded or
unavailable, never zero. Historical missing closures are not a live queue.

The latest closed scoring batch adds six NMNB card 74 scenes after two charged
pre-Native CLI setup failures. Task 2201 captures with 489 updates remaining and
3,255 collected orbs, carrying a pending 21,381,650 award and 216 time orbs. Its
local income is lower than the ordinary control. Its complete continuation has
not run, so it establishes no new route record.

Earlier card 47 composition yields 9,656 orbs with capture, versus 8,149 in the
previous captured profile; full movement-profile retention yields 11,319 with a
timeout. Complete futures end at 6,534,027,080 and 6,554,033,050, below the record.
Whole replay rows/audits, 16 prefix edges and two Native maps agree. The successful
local strategy remains archived alongside these negative complete outcomes.

Historical stage/spell composition, early-road point production, solo Sakuya,
option anchors, reward settlement, pool limits and private proposal experiments
remain in the existing [October 27](experiments/2026-10-27-phase-composition.md)
and [October 28–29](experiments/2026-10-28-stage-spell-search.md) journals.
Those dated records describe their original run states, not current launch plans.

## Operational resume contract

These rules constrain authorized work; they do not authorize resuming scoring.

| Requirement | Current contract |
| --- | --- |
| Publication | Direct `main`; fetch first, preserve others' work, use `gpt-6.1-sol-max: <English summary>` and push tested changes |
| Local owners | Last profile: at most two drivers in one user service, aggregate two-core quota, low CPU/I/O priority. Standing ceiling: four drivers including solvers, probes, replay verifiers and test owners. Fork children belong to one owner |
| Local memory | 4 GiB address space per process and 4 GiB summed RSS per owned tree; service MemoryHigh 6 GiB / MemoryMax 10 GiB, no swap. Admit at 12 GiB host MemAvailable; abort below 8 GiB. Aborted work is charged and certifies nothing |
| Campaign | One ledger at `reports/local/shadow-runtime-v1/tasks.tsv`; current cap 2240. Extension 2176 → 2240 was recorded at 2168 before 2169. Read the maximum first-column ID, not line count. Record further authorized 64-task extensions before use; never reset IDs or omit failures |
| Constraints | `--allow-bombs 0 --require-all-captures 0 --verify-captures 0`; record actual outcomes. Use both capture flags at 1 for strict all-capture work |
| Episodes | Reuse original episode seeds below. A new shot/entry episode needs a new original Wine pre-RegisterChain receipt before Native scoring; no Native-selected or hindsight-selected seed |
| Checkpoints | Reexecute the literal first N owned inputs from the same entry; N is not spell age, replay row count or an audit calculation counter. Carry the actual world/RNG and lifecycle state |
| Acceptance | Inspect owner exit, result and audit. Wrapper exit 0 is insufficient. Exit 5 can mean a valid complete below-target run; partial/failed/resource-aborted futures cannot establish a route record |
| Replay retention | A new `.rpy` needs a strict overall Extra or complete-Lunatic improvement plus all fresh/natural gates. Preserve earlier selected bytes |
| Wine gate | Seed acquisition and qualified replay playback only. A complete NMNB score must strictly exceed its dated all-shot reference under the declared constraints |
| Resource/test ownership | No test driver beside a full solver allocation. Compile with at most two jobs; run relevant checks and normal CTest before publication |
| Remote work | Paused after a host-key identity change. No reconnect or key replacement without verified identity. Nineteen original terminal receipts remain unresolved; no remote closure or success is inferred |

The last local launcher is `reports/local/global-score-20261027/run-local-two-v232.cjs`,
using `run-scarlet-guarded-2240.cjs`. Its memory guard is
`reports/local/retail-score-offline-20261026/memory-guard.cjs`; the relaxed verifier
is `reports/local/score-continue-20261027/verify-no-bomb-2240.cjs`. These ignored
workspace tools are not supplied by a fresh clone. Old launchers and stopped
queues are historical evidence, not supported resume commands.

If remote work is explicitly resumed, its standing limits are 32 Native owners /
32 cores, 4 GiB address space/tree RSS per owner, 96 GiB aggregate owned RSS and
128 GiB hard service memory with no swap. Host admission/abort reserves are
128/96 GiB. Use the same ledger and isolated owned user service; preserve unrelated
workloads. Bind every source/binary/library/DAT/input/receipt identity and pass
complete cross-host execution/row-byte canaries before scoring.

### Fixed original episodes

| Shot | Extra seed / acquisition task | Story seed / acquisition task | Story entry |
| --- | --- | --- | --- |
| Scarlet Team (2) | 9098 / 665 | 32796 / 666 | Stage 1, Lunatic, prior Final A clear |
| Magic Team (1) | 46997 / 265 | 1805 / 266 | Stage 1, Lunatic, prior Final A clear |
| Solo Sakuya (8) | 53346 / 1659 | 24238 / 1676 | Stage 1, Lunatic, no team-history flag |

[Validation](VALIDATION.md#native-score-first-workflow) owns receipt hashes,
failed acquisitions and configuration details. Historical fixed-seed benchmarks
are separate episodes and retain their original identities.

## Verification and current public code

Normal CTest results depend on configured assets and retained fixtures. The current
maintenance configuration passes 62/62 in task 2208 (911.20 s), including the
real-DAT and replay-export regressions. [Validation](VALIDATION.md#local-build-and-core-checks)
owns build commands and the data-free test count. Private scoring prototypes have
separate frozen contracts and real-data gates; they are not public-main features.

The public phase plan is immutable and keyed by actual stage/card. The optional
capture-form recovery prior is disabled by default, needs a nonempty plan and
enabled baseline recovery, and supplies no farm geometry or future world state.
Native rollouts determine safety and actual damage/capture/reward behavior.

## Unresolved problems and proposed next work

Scoring remains paused. These proposals require renewed owner instruction:

- Continue toward 4 billion Extra and 7 billion Lunatic, then the story all-shot gate.
  Current maps 2100/2117 locate verified record boundaries; changed worlds need new maps.
- If resumed, task 2201's captured card 74 scene closes at owned input 82780. Recompute
  card 77, Stage 5 and Final B before claiming any gain. The same policy times out
  on parent 2189, so the reserve is not portable across incoming worlds.
- Card 13 enters at input 13817 in the verified story. Existing HP/anchor traces
  support testing a damaged prefix followed by a bounded fire pause; no trial has run.
- Study earlier point/orb production and actual capture timing. Local PIV, score,
  or orb dominance does not establish world equivalence or the best complete future.
- Keep newer Native Extra evidence separate from the original Wine validation of
  task 1314's replay. Retail validation remains a distinct qualified step.

## Documentation and local retention

The [storage journal](experiments/2026-10-28-storage-cleanup.md) owns retained
recordings, Wine/object archives and restoration limits. Deleted MKV/PCM originals
are not recoverable promises. Current code contracts belong in Architecture,
mechanisms in Planner, commands/provenance in Validation, and per-case history
in the existing journals. The [pre-maintenance snapshot](https://github.com/N0zoM1z0/th08-solver/blob/5929ced/docs/STATUS.md)
is historical evidence rather than another current launch plan.

## Legacy component evidence

The separate [legacy subset catalog](../reports/native/archive/README.md#legacy-subset-catalog)
has no verified source-faithful full-spell solution. This count does not describe
Native game scoring, controlled DAT scenarios or synthetic worlds.
