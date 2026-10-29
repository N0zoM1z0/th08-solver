# Current handoff status

Updated October29,2026. Current work is offline Native score attack for **Extra**
and **complete Lunatic1→2→3→4A→5→FinalB**, using stage/spell strategy composition
and bounded local searches. All shots are permitted; Scarlet Team has priority.
The active targets are **4B Extra and7B complete Lunatic**, then the story
all-shot human reference. The6.5B milestone is achieved. Scarlet remains the
priority; solo Sakuya now has separately retail-seeded complete NMNB baselines.
The latest local batch raises complete Lunatic through per-spell beam
composition:187 uses64, then191 uses384. Experiments1930–1953 have finished;
publication audit1954 checks the two verified improvements and complete negatives.
The owner now requests **local-only work** after a VPS SSH identity change.
Nineteen earlier remote terminal receipts remain unavailable; their state is
explicitly unresolved and they certify no new score. Remote closure/retention
awaits verified access; no remote key is changed or connection retried.

## Verified results

All scores are displayed points. Every Native entry below is complete No Miss /
No Bomb, with original-entry fresh41-word terminal agreement and natural exported
replay score/capture/name agreement. Captures are optional for current scoring;
strict NMNB/all-capture defaults remain available in the CLI.

| Route | Shot | Best Native score | Captures | Producer / fresh / natural |
| --- | --- | ---: | ---: | --- |
| Extra | Scarlet, Sakuya/Remilia | **3,296,376,500** |14/14 |1917 /1919 /1920 |
| Complete Lunatic | Scarlet, Sakuya/Remilia | **6,626,897,920** |36/37,10 timeout |1938 /1939 /1940 |
| Extra | Magic, Marisa/Alice |3,013,913,970 |14/14 |597; identities in Validation |
| Complete Lunatic | Magic, Marisa/Alice |5,550,599,270 |36/37,47 timeout |620; identities in Validation |
| Complete Lunatic, all captures | Magic, Marisa/Alice |5,107,553,670 |37/37 |575; identities in Validation |

All44 selected progress replay files, including superseded records, are published
under [shot/route directories](../reports/native/README.md). The
[progress index](../reports/native/replays/progress.tsv) binds scores, constraints,
producers, original receipts and exact replay/action hashes.
[Validation](VALIDATION.md#scarlet-team-verified-native-baselines) owns full lineage.
Fresh execution/index agreement is not an independent retail physics oracle.

Original Wine evidence has a separate scope: Scarlet Extra **3,179,193,860**,
NMNB14/14, passes original-game playback1343 and corrected typed analysis1367.
The original wrapper's offset-decoding failure remains recorded. Newer Native
scores above do not inherit that retail validation. Earlier Magic online clears
are Extra1,166,969,620/14of14 and complete Lunatic2,189,965,070/33of37, both NMNB.
Their selected recordings and game-audio videos remain indexed by
[README](../README.md#original-wine-results) and
[Validation](VALIDATION.md#selected-online-recordings-and-replay-provenance).
Online MPC remains paused.

### Score targets and historical references

The dated **October27 all-shot references** are Extra3,178,460,830
(Sakurei, solo Youmu, June21,2022) and Lunatic7,053,332,460
(IwanaM, solo Youmu, July14,2026). Their dated source is retained in
[Validation](VALIDATION.md#october27-verified-native-progress).
Native Extra is117,915,670 above its reference and703,623,500 below4B.
Complete Lunatic remains373,102,080 below7B and426,434,540 below its retail replay gate.

Earlier same-shot comparisons remain distinct: Magic ASL Extra3,021,639,920
(one bomb) and Alan Lunatic6,168,365,790; Scarlet ASL Extra3,000,284,220 and
Levea Lunatic6,755,587,090. These are historical references with their own
constraints, not NMNB upper bounds. The
[human/source investigation](experiments/2026-10-27-phase-composition.md)
retains the actual traces, constraints and accounting limits.

## What has been explored

The [experiment index](experiments/README.md) owns the full record: hypotheses,
source/human evidence, parameters, costs, failures, retries and decisions.
The [campaign ledger](experiments/campaign-ledger.tsv) has every allocation;
[per-task results](experiments/results.tsv) preserve measured outcomes and missing
historical measurements. A missing value means unavailable, never zero.
The177 tasks1466–1642 retain their terminal outcomes. The subsequent source/
human investigation, solo episodes and complete compositions are recorded in
the same journal and cumulative ledger.
The index separately lists231 older ledger-only records and29 missing owner
closures; those historical uncertainties are not a live queue or new certificates.

| Exploration | Evidence and outcome | Consequence |
| --- | --- | --- |
| Human6 gauge bootstrap and early road worlds |1406–1465:151 focused pickups explain the human bootstrap; legal prelude and Stage1/2 variants do not yield a complete record | Preserve the actual incoming world; a pose or local PIV gain is insufficient |
| Phase-controller portability | V47 initially matches25 but not17/29. V49 and public1530 reproduce legacy17/25/29 inputs,41 words and audits after restoring independent capture-form recovery and the missing Stage2 road component | Default-off `--phase-recovery-prior` is public; private road guides remain private |
| Extra192/197/202 composition | V48 cash0 tails are configuration-confounded controls. Correct cash4 produces1517 at3,194,638,140; actual205 retuning produces1539 at3,203,494,130 | Explicitly bind the whole suffix profile and retime every changed boundary |
| Stage2 cash/PIV frontier and77 | V49/V51/V52 carry distinct real worlds;77 reaches8945 orbs/capture or9128/timeout, then recomputes Stage5 and FinalB | Partial gains are not added to the old route |
|116 capture time | V53 fixed600 reserves time out. Read-only HP/pose probes explain insufficient damage time; V55 focused1200/1600 reserves capture with3184/3423 orbs | Retest the reserve after an earlier world changes; it is not a universal safety bound |
| Extra point production and composition | V57/V59/V60 verify1621 at3.213B. New1654 road has646 early points;1665 loses45 at a full202 pool. V88 retimes its actual55417 entry;1740 restores1051 final points and verifies3,255,579,400 | Preserve distinct incoming worlds, retime actual boundaries and measure production as well as PIV |
| Shot capabilities and human inputs | Source separates all12 families; Sakuya8 has no extreme-youkai graze-orb income. Newly seeded solo Extra/story verify2.183B/4.418B NMNB14/14 and37/37, below overall records | Keep human bomb/miss constraints separate; do not copy Scarlet farming unchanged |
| Trajectory and guidance negatives | Larger story proposal horizons/beams, hard/soft charge phases and six complete203 upper geometries yield no further record | Retain costs and actual action traces; unproductive controller prototypes remain private |
| Actual171 and final survivals | Earlier V56/V64 remain negative. V97/V98/V100 carry a new77 world through Stage5 and FinalB; initial full1813 loses. Retarget actual owner146244 at171; reserve0 world1838 yields17504 orbs and complete1847 verifies6,618,962,600, while18522-orb1842 gives a lower full score | Recompute the complete suffix and retain original input/owner-coordinate lineage |
| Faster streams and priming | Current199 sampling excludes80.44% of eligible slot-observations at the3px/update lookup cutoff. Private8/8 gains51.110M over a matched full control, still below Extra record. Above-boss priming is mixed/negative phase evidence | No private experimental controller is promoted; source simulation remains unchanged |
| Actual phase control and reward lifecycle | V116–V135: cash4 overrides new fire intent; cash0 realizes25040-orb199 timeout but omits45 point requests and its capture award. Eight complete199/205 futures stay below the record; best3,239,225,090 | Inspect actual inputs, carry130-update pending rewards, and distinguish absent drops from pool failures |
| Captured195 and205 composition |1901’s actual no-fire/capture world produces complete1903 at3.282B. Retarget its actual205 entry with cash0/beam128;1917 verifies3,296,376,500. The matched beam384 case is lower and slower in this scene | Preserve distinct orb/bonus worlds; use the cheaper beam only where the complete result supports it |
| Selective longer175 view | Uniform H256/512 are negative. Retain only H256’s175, then restore H128:1913 reaches6,615,055,070, still3,907,530 below the record | A better local component can recover most of a bad whole-tail profile without setting a route record |
| New202 point-pool diagnosis |1916’s higher-PIV world loses45 points. Stock/observer rows and full audits agree; all45 requests encounter2096/2096 occupancy. Human retiming restores1051 final points but ends3,246,938,610 | Correct the measured mechanism, then compare full income; recovering points alone does not guarantee a record |
| Per-spell smaller beams and broader worlds |1931 preserves the first155473 inputs, improves187 with beam64, then1938 restores beam384 at actual191 entry156833 and verifies6,626,897,920. Earlier-FinalB beam128/64 compositions finish6,555,553,350/6,491,775,810 | Transfer scene-specific policies while recomputing each actual world; a cheap whole-stage profile is not automatically a better route |
| Extra198 frontier |1935/1936 gain895/376198 orbs plus higher awards; complete compositions1946/1947 finish3,280,306,170/3,254,041,190 with1051 points | Local orb/SCB gains do not bound the changed later trajectory |
| Checkpoint units and interruption | Audit calculation counters differ from owner input counts. V104 corrects3–5-update labels without changing raw trials. SSH identity changes leave missing remote closures unverified | Use measured owner phase maps; preserve failed setup/unknown receipts and local-only steering |

The [October28 journal](experiments/2026-10-28-stage-spell-search.md) records each
V47–V168 case and source-backed conclusion, including setup failures, resource
aborts and all unsuccessful complete combinations. The
[October27 phase journal](experiments/2026-10-27-phase-composition.md) owns earlier
phase maps and the6.5B composition; the [VPS journal](experiments/2026-10-27-vps.md)
owns the initial remote protocol and older campaigns. Earlier chronological
status text is preserved in the fixed
[pre-maintenance snapshot](https://github.com/N0zoM1z0/th08-solver/blob/5929ced/docs/STATUS.md).
It is historical evidence, not another current launch plan.

## Operational resume contract

| Requirement | Active contract |
| --- | --- |
| Publication | Direct `main`; fetch first, preserve others' work, use English `gpt-6.1-sol-max: ...` progress commits and push tested scope |
| Local owners | Current local profile: at most2 solver, probe, replay-verification or test owners, one user service, aggregate CPU quota2 cores and low CPU/I/O priority. Standing ceiling remains4 drivers. Fork children belong to one owner. Compile with `--parallel 2` |
| Local memory |Service MemoryHigh6GiB/MemoryMax10GiB/no swap;4GiB address space per process and4GiB summed RSS per owned tree; admit with at least12GiB host MemAvailable, abort below8GiB. Resource-aborted work remains charged and certifies nothing |
| Remote owners | Currently paused by owner local-only steering; do not reconnect or replace host keys without verified identity. Standing contract: Native only; at most32 owners and32 total CPU cores under the owned user systemd manager, low CPU/I/O priority; preserve the old checkout and unrelated workloads |
| Remote memory |4GiB AS/tree per owner,96GiB aggregate owned RSS guard,128GiB hard service limit, no service swap; host128GiB admission/96GiB abort reserves |
| Campaign | One cumulative ledger at `reports/local/shadow-runtime-v1/tasks.tsv`; cap1984. Extension1920→1984 is recorded at1918 before1921;1856→1920 at1851 before use. Extensions1664→1728,1728→1792 and1792→1856 were recorded at1657,1713 and1783 before use. Record further64-task extensions before use; never reset, reuse IDs or omit aborted/retried work |
| Scope | No Miss, `--allow-bombs 0 --require-all-captures 0 --verify-captures 0`; record actual bombs and captures. Restore both capture flags to1 for strict all-capture work |
| Episodes | Fixed original retail seeds from the table below; new shot/entry episodes require a new original Wine pre-RegisterChain receipt before Native exploration. No Native-selected or hindsight-selected seed |
| Checkpoints | Reexecute literal firstN owned inputs from that same original entry. N is owner updates, not spell age or replay row count. Carry RNG, bullets, actors, items, latch and observer histories |
| Acceptance | Inspect owner `exit.json`, `run/result.json` and `run/score-audit.json`. A launcher exit0 does not certify a child. Exit5 can be a valid complete below-target result; incomplete/failed/resource-aborted futures cannot certify a prefix |
| Publication gate | Retain a future `.rpy` only for a strict overall Extra or complete-Lunatic improvement, with fresh exact41-word terminal and natural score/zero-miss/bomb/capture/name checks. Preserve earlier selected replay bytes |
| Wine gate | Local seed acquisition and qualified final playback only. A complete NMNB result must exceed the corresponding all-shot reference under its declared capture goal before retail replay validation.7B alone is below the Lunatic gate |
| Code verification | Read affected interfaces, freeze tested binaries, run relevant real-DAT cases and normal CTest before code publication. Documentation-only maintenance runs documentation CTest; do not relink during tests |
| Evidence | Hash-bind source, ELF, loader/libraries, DAT, receipt, actual args and input prefix. Charge retries, target selection and cross-host canaries. Fresh replay is not independent retail equivalence |

The current local launcher is
`reports/local/global-score-20261027/run-local-two-v153.cjs`, which calls
`run-scarlet-guarded-1984.cjs`; its memory guard is
`reports/local/retail-score-offline-20261026/memory-guard.cjs` and its relaxed
fresh/natural checker is `reports/local/score-continue-20261027/verify-no-bomb-1984.cjs`.
These are ignored workspace tools, not files supplied by a fresh clone.
Use fresh names and the maximum first ledger column, not line count: the ledger
preserves repeated historical lifecycle notes. Older320/1280-task launchers and
interrupted queues are historical, not current entry points.

Remote packets live in `reports/local/vps-native-20261027/deploy/`; received
owner receipts are under `received/`. Every new remote binary/library closure
must pass complete local/remote execution and row-byte canaries before scoring.
Return selected outputs for fresh local verification. The frozen initial bundle
uses base `bc75a3e`; later private planner overlays have separate source/binary
manifests. Do not mistake that base for the current public source revision or
relink old `.o` files across header/ABI changes.

## Validated retail episodes

| Shot / route | Episode under `reports/local/` | Fixed seed | Raw stage / difficulty / shot / prior FinalA |
| --- | --- | ---: | --- |
| Scarlet Extra | `retail-seed-episodes-20261027/scarlet-extra-entry-v6/` |9098 |8 /4 /2 /false |
| Scarlet complete Lunatic | `retail-seed-episodes-20261027/scarlet-story-4a-finalb-entry-v1/` |32796 |0 /3 /2 /true |
| Solo Sakuya Extra | `retail-seed-episodes-20261028/sakuya-extra-entry-v1/` |53346 |8 /4 /8 /false |
| Solo Sakuya complete Lunatic | `retail-seed-episodes-20261028/sakuya-story-4a-finalb-entry-v3/` |24238 |0 /3 /8 /false |
| Magic Extra | `retail-seed-episodes-20261026/extra-entry-v4/` |46997 |8 /4 /1 /false |
| Magic complete Lunatic | `retail-seed-episodes-20261026/story-4a-finalb-entry-v1/` |1805 |0 /3 /1 /true |

All use story=true, spell=-1 and the declared dialogue behavior.
`native-entry.json` binds the original receipt; retain `start.txt`, acquisition
manifest, observer/launcher identities and failed startup receipts.
[Validation](VALIDATION.md#scarlet-team-retail-episodes) owns exact hashes.
Historical Extra10025 and story40755 retain their earlier identities; they are
not replacement seed receipts under the current acquisition policy.

## Current evidence and tooling

Paths below are local workspace evidence unless linked to published files.

| Item | Location |
| --- | --- |
| Latest Extra producer | `reports/local/retail-score-offline-20261026/local-extra3282-last205-v149-cash0-move4-beam128/`; complete76677 inputs, fresh1919/natural1920 and actual owner map1921 |
| Latest story producer | `reports/local/retail-score-offline-20261026/local-luna191-v161-beam384-cash4-move3/`; complete159445 inputs, fresh1939/natural1940, actual map1941 |
| Selected exact bytes | `reports/native/replays/{scarlet,magic}/{extra,lunatic}/` and `reports/native/replays/progress.tsv` |
| Current private state | `reports/local/vps-native-20261027/continuation-state.json`; resumption metadata, not a gameplay snapshot |
| Remote controls and outcomes | `reports/local/vps-native-20261027/deploy/`, `received/`, hypothesis packets and retention receipts |
| Normal test coverage | Final minimal solo-history build1752 and uninterrupted62/62 normal1753 pass,933.58s; peak tree1104392192B, minimum host MemAvailable9839382528B, no resource abort. Earlier1697/1713 retain their fixture failure/retry |
| Affected CLI/runtime parity | First-update solo story regression passes inside1753; fresh solo complete-route1715/1716 and1754/1755 pass. Earlier1530 still binds17/25/29 and omitted-flag parity |

The public phase plan is an immutable proposal keyed by real stage/card.
`--phase-recovery-prior` defaults off; when enabled it preserves only capture-form
preference in independent recovery, without adding scoring memory, farm geometry,
fire/reserve control or generic pacing. The full-plan constructor remains strict.
Native rollout still decides actual safety, HP, timeout and reward.
Private source-clock road guides and historical learned snapshots retain their
own manifests and limitations; public-main search is not claimed to regenerate
every old private parent. Published replays retain exact original bytes.

## Measured point economy and next scoring work

The source-faithful6.512B point audit measures an optimistic191,843,350 of
same-pickup height/human-doubling repricing, less than the7B deficit. That is an
accounting envelope, not guaranteed extra score. Earlier orb/PIV income, actual
point production, prices, bonuses and clear rewards all need measurement.

The earlier V64 frontier's closed171 cash is558,200 below its gold, but closed191 cash
is31,513,550 below it. Its last-five incremental deficit is30,955,350; final
settlement adds4,214,350 of deficit, totaling35,727,900. Both FinalB runs collect
438 points; the new route has higher final PIV1455300 versus1446720.
These observations locate the loss without proving a causal reward breakdown.
[The full journal conclusion](experiments/2026-10-28-stage-spell-search.md#v64-full-route-conclusion)
records the actual audits.

Earlier1830 gains97,383,060 over1381. It collects434 FinalB points
versus438 before, yet ends with74431 FinalB orbs/PIV1467680 versus70872/1446720.
The richer171 and recomputed final survivals produce the verified total; these
scalars are observations, not interchangeable worlds or an additive score model.
Earlier1847 gains105,506,420 over1381, with434 FinalB points,73650 orbs and
PIV1463770; that improvement does not require more final orbs.
Current1938 gains113,441,740 over1381, ending434 FinalB points/74073 orbs/
PIV1465890. It preserves the original entry and the actual187/191 worlds.

## Unresolved problems and proposed next work

- 7B complete Lunatic and the7,053,332,460 all-shot gate remain unachieved.
- Current171 retargeting converts the new77 world into a complete record.
  Continue studying earlier point/orb production and source-specific trajectories,
  using the current1941 story owner map and1921 Extra map for changed boundaries.
- Preserve failures and explicit counterexamples: local score, PIV or orb
  dominance does not prove the best complete continuation or world equivalence.
- The latest Extra remains Native-only. Schedule final original Wine playback
  for the chosen qualified result; retain1314's existing retail proof separately.
- Extra4B remains unachieved. Same-pickup repricing of the earlier3.213B record
  offers only an optimistic36.233M; earlier production and source-specific
  trajectories are larger questions. New203 upper bands and charging prototypes
  are measured negatives, not reasons to change physics or gauge rules.

## Documentation and local retention

[The storage journal](experiments/2026-10-28-storage-cleanup.md) owns current
retention policy, completed maintenance and restore limits. The owner's latest
instruction keeps compressed video versions and permits removal of other video
artifacts; earlier exact-MKV/audio recovery guarantees are historical once those
archives are retired. Selected replay/input/receipt/source evidence remains
separate from disposable media and Wine scratch environments.

The complete pre-maintenance status is preserved at the fixed5929ced link above;
no old commit, report or selected replay is rewritten. Current facts belong here,
commands and provenance in Validation, mechanisms in Planner/Architecture, and
per-case history in the existing experiment journals.

## Legacy component evidence

These are the separate legacy subset catalog counts, not Native scoring coverage.

| Outcome | Verified count |
| --- | --- |
| Indexed original spell IDs | 222 |
| Indexed spell-start occurrences | 431 |
| Complete offline spell solutions | 0 |

There are 38 core CTests without private data, plus optional pinned-source checks.
The current local DAT/retained configuration has62 cases. Old57/59/60-case suites
remain historical measurements in Validation and the experiment journal.
