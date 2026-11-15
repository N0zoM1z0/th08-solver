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

The November 15 cleanup centred the repository on the offline Native pipeline
([Architecture](ARCHITECTURE.md)). The online MPC code (live Wine client,
current-memory and 60 Hz controllers, value models and the `th08_plan` verifier)
is **kept and tested** but paused ([Planner](PLANNER.md#online-mpc-retained-paused)).
Removed subsystems (the pre-import reimplementation, synthetic and controlled-DAT
worlds, source probes, repair/probe/hazard diagnostics, the legacy replay-target
tool and the legacy report archive) remain in Git at tag `pre-cleanup-20261115`.
The dated journals are condensed into [Lessons](LESSONS.md).

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

## Diagnosis and next work

[Roadmap](ROADMAP.md) owns the plan. In short: the story record matches the best
same-shot human on point items but collects about 23,556 fewer time orbs in
Stages 1–3, worth about 395 M under the source score model, which is roughly the
whole gap to the gate. The previous campaign ranked alternatives by local horizon
cash, which undervalues early orbs by about 70×, and never planned the gauge
across segments. Roadmap items, in order: restore the run environment (R0),
rebuild the record tapes from the replays (R1), rank by whole-run value (R2),
plan the gauge in Stages 1–3 (R3), select at stage boundaries (R4), evaluate solo
Youmu (R5) and improve throughput (R6). None has started.

## Operating limits

- Local: at most two owned drivers, two cores total, low CPU/I/O priority; 4 GiB
  address space per process and 4 GiB RSS per owner tree; admit at 12 GiB host
  `MemAvailable`, abort below 8 GiB. Builds use at most `--parallel 2`.
- Remote Native search (`yann@185.189.44.160`) stays paused after its host-key
  change; do not reconnect until the owner verifies the host identity.
- Reuse the fixed seed episodes in [Validation](VALIDATION.md#seed-acquisition-wine).
  A new shot or entry needs a new Wine receipt before any Native scoring.
