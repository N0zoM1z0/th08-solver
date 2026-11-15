# Status

Updated November 15, 2026. **Scoring is authorized again** (owner instruction of
November 15, 2026). Play is offline Native only; Wine is used for seed acquisition
and final retail replay playback.

## Targets and records

| Route | Shot | Record | Captures | Target | Dated all-shot human reference |
| --- | --- | ---: | --- | ---: | ---: |
| Complete Lunatic 1→2→3→4A→5→Final B | Scarlet Team | **6,653,194,550** | 36/37 (card 10 timeout) | 7,000,000,000 | 7,053,332,460 (IwanaM, solo Youmu, July 14, 2026) |
| Extra | Scarlet Team | **3,308,826,050** | 14/14 | 4,000,000,000 | 3,178,460,830 (Sakurei, solo Youmu, June 21, 2022) |

Constraints are No Miss / No Bomb; captures are optional. The story record is
400,137,910 below its retail gate (a complete NMNB score must exceed the
reference before Wine replay validation). Extra already exceeds its reference.
Other shots: Magic Team Extra 3,013,913,970 and story 5,550,599,270 (all-capture
5,107,553,670); solo Sakuya Extra 2,182,761,340 and story 4,417,570,020. Same-shot
human references: Scarlet story 6,755,587,090 (Levea, 1 miss / 9 bombs), Scarlet
Extra 3,000,284,220 (ASL), Magic story 6,168,365,790 (Alan), Magic Extra
3,021,639,920 (ASL, one bomb).

## Repository state

The November 15 cleanup reduced the repository to the offline Native pipeline
([Architecture](ARCHITECTURE.md)). Removed subsystems (the pre-import
reimplementation, synthetic and controlled-DAT worlds, source probes, the old fork
planner `th08_plan`, live Wine/online controllers, value models, repair/probe
diagnostics and the legacy report archive) remain in Git at tag
`pre-cleanup-20261115`. The dated journals are condensed into [Lessons](LESSONS.md).

## Local workspace

Most of the ignored `reports/local/` was deleted before November 15, 2026,
including the working ledger, run launchers, memory guard, journal exporter,
private policy overlays and **every producer tape (`plan.actions`) of the
records**. Retained: the Wine seed episodes under
`reports/local/retail-seed-episodes-20261026/`, `…-20261027/` and `…-20261028/`.
The published [ledger](experiments/campaign-ledger.tsv) ends at task 2209; the
next task ID is 2210.

## Resuming from the records

A continuation needs the record's owner input tape. Natural replay rows contain
the input of each calculation row, but owner inputs and calculation rows differ at
stage-control boundaries, so a tape rebuilt from the `.rpy` must be proven by a
fresh `th08_shadow_follow` run that reproduces the record's terminal projection
and score before it is used as a prefix. Until then, new work starts from the
original episode entry.

## Diagnosis: where the story score is lost

[Scoring](SCORING.md) gives the source mechanics. The record collects almost the
same point items as the best same-shot human (1,628 versus about 1,661) and more
time orbs overall (234,573), but **fewer orbs early**: Stages 1–3 have
12,755/33,391/36,052 versus the human's 21,873/37,662/46,219. Because each orb
raises the value of every later point item (doubled at extreme human), an early orb
is worth about 17 k at the end of the run. Under that model, the human's extra
23,556 Stage 1–3 orbs are worth about 395 M, matching the measured −392 M deficit
at the Stage 5 exit (the record then gains 290 M back in Final B).

The previous campaign optimized each scene by local evidence: horizon cash in the
planner's economic re-ranking, and scene cash, orbs or PIV when selecting
components. Horizon cash credits an early orb with only its pickup value (about
250 at P ≈ 50), roughly 70× too little, and nothing in the planner values the
gauge state that produces orbs later. This matches what the lessons record: local
improvements repeatedly reversed in complete futures, while the large gains came
from early-card plans that raised orb production (17/25/29, 77, 171: +382 M).

Checked against the existing data: across 175 unique complete NMNB story runs of
seed 32796, a value that adds the model's future worth of carried points and orbs
ranks final scores better than raw cash at the Stage 1–3 boundaries (Spearman
0.49→0.71, 0.80→0.83, 0.83→0.85; 6, 15 and 43 distinct boundary worlds) and equally
from 4A on. The signal is real but modest, because downstream strategy still differs.

## Next work

In order of expected value per unit of compute:

1. **Rebuild the record tapes** from the two `.rpy` files and prove them with a fresh
   follow, so continuations can resume from the records.
2. **Rank with whole-run value instead of horizon cash.** Replace the economic
   re-ranking criterion with `Δscore + α(t)·ΔT + β(t)·ΔP` (α ≈ 10·(P_final − P(t)),
   β ≈ 5·(T_final − T(t)), plug-in finals from the incumbent), and use the same
   value to rank stage-boundary worlds. Earlier constant `cash + K·PIV` ranking
   (tasks 835–842) lost; the declining α(t) and stage-boundary use are the changes.
3. **Plan the gauge as a resource in Stages 1–3.** The human's early advantage comes
   from gauge bootstraps, for example on card 6, where 151 focused orb pickups in 40
   updates swing the gauge from −8000 to +10000 and graze orbs follow. Capture-reward
   orbs (935 per fast capture) saturate the gauge toward the focus side, so focus
   during reward pickup selects the next segment's gauge. This needs a segment-level
   plan, not a longer MPC horizon.
4. **Stage-boundary beam.** Keep several stage-end worlds, rank them by the value in
   item 2, and run complete futures only for the best few. A complete story future
   costs about 2,600 s on two local cores, so throughput decides how many ideas can
   be tested.
5. **Reconsider the shot.** Both all-shot references are solo Youmu, whose gauge
   reaches both extremes at ±3000 instead of ±8000, with team-rate graze and chain
   orbs. The best human Scarlet story is 6,755,587,090 even with bombs. A Youmu
   episode needs a new Wine seed receipt and a solo-Youmu gauge controller.

Extra 4 B is not near: points are close to their ceiling (36 M repricing envelope)
and the gap is 691 M.

## Operating limits

- Local: at most two owned drivers, two cores total, low CPU/I/O priority; 4 GiB
  address space per process and 4 GiB RSS per owner tree; admit at 12 GiB host
  `MemAvailable`, abort below 8 GiB. Builds use at most `--parallel 2`.
- Remote Native search (`yann@185.189.44.160`) stays paused after its host-key
  change; do not reconnect until the owner verifies the host identity.
- Reuse the fixed seed episodes in [Validation](VALIDATION.md#seed-acquisition-wine).
  A new shot or entry needs a new Wine receipt before any Native scoring.
