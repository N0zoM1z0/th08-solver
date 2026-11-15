# Roadmap

Written November 15, 2026, after the repository cleanup. Nothing here has been
started. It is the plan for the next scoring work, aimed at the complete Lunatic
NMNB story (record 6,653,194,550; gate: above 7,053,332,460). Read
[Status](STATUS.md), [Scoring](SCORING.md), [Lessons](LESSONS.md),
[Planner](PLANNER.md) and [AGENTS.md](../AGENTS.md) first.

## Diagnosis

The first campaign improved the route scene by scene, ranking alternatives by local
evidence: displayed score at the end of a 128-update horizon (the planner's
economic re-ranking), and scene cash, orb counts or PIV when choosing which
component world to keep. The source mechanics say this objective is biased.

**Score is mostly a cross term between point items and time orbs**
([Scoring](SCORING.md#whole-run-model)). An orb pays 5·P (P = cumulative points)
and every two orbs raise the point item value by 10; points double at extreme human
gauge. At the end of the run:

- one point item is worth about `600k + 5·T_final + 5·T(t) + 75k` (1.9–3.0 M for the record);
- one time orb is worth about `10·P_final − 5·P(t) + 1.5k` (10–18 k), so an early orb
  is worth about 1.7× a late one.

**The record loses early orbs, not points.** Against the best same-shot human
(Levea, 6,755,587,090 with 1 miss / 9 bombs) the record has the same points
(1,628 versus about 1,661) but fewer orbs in Stages 1–3: 12,755/33,391/36,052
versus 21,873/37,662/46,219. Valued with the model, those 23,556 orbs are worth
about 395 M: the measured deficit at the Stage 5 exit is −392 M, and the gap to the
gate is 400 M. Final B is already 290 M better than the human.

**Four blind spots follow.**

1. *Objective.* Horizon cash credits an early orb with only its pickup value
   (about 250 at P ≈ 50, against about 17,500 of final value) and an early point with
   about a third of its value. Selecting component worlds by cash, orbs or PIV has the
   same problem; [Lessons](LESSONS.md#5-methodological-lessons) lists more than a
   dozen local wins that lost in complete futures.
2. *Gauge.* Orb production depends on being at an extreme gauge when it matters
   (youkai while grazing a boss, human while points are collected). The gauge
   carries across segments; the 128-update MPC cannot plan it. On card 6 the human
   collects 151 focused orbs in 40 updates and swings −8000 → +10000, while Native
   is still at 679 after 120 updates. A 935-orb capture reward saturates the gauge
   toward whatever focus state the player holds while collecting it.
3. *Where effort went.* Late-scene tuning produced 14 records worth +43 M in total;
   early-card plans that raised orb production (cards 17/25/29, 77, 171) produced
   +382 M. Early changes are expensive because the whole downstream route must be
   recomputed (about 2,600 s per complete story on two local cores).
4. *Shot.* Both all-shot references are solo Youmu. Her gauge reaches both extremes
   at ±3000 (teams need ±8000) and she keeps team-rate graze and chain orbs. The best
   human Scarlet story is 6.76 B even with bombs.

**Evidence quality.** Over 175 unique complete NMNB seed-32796 stories in
[results.tsv](experiments/results.tsv), adding the model's future value of carried
points and orbs to the boundary score improves the rank correlation with the final
score at the Stage 1–3 boundaries (Spearman 0.49→0.71, 0.80→0.83, 0.83→0.85 over 6,
15 and 43 distinct boundary worlds) and changes nothing from 4A on. A constant
`cash + K·PIV` ranking was tried once per K on Extra (tasks 835–838: K = 256/1024/4096
gave 2.911/3.048/3.033 B against 3.088 B at K = 0; the story runs 839–842 are
unresolved or failed NMNB). That test used one sample per arm, a constant price and
no point term, so it does not refute the items below, but it is a warning.

## Evaluation rules for every item

- Same fixed seed episode, NMNB, captures optional; follow [AGENTS.md](../AGENTS.md)
  resource limits. Record every owner in the ledger and [results.tsv](experiments/results.tsv).
- Routes are chaotic: any change, including an identical-parameter restart, moves
  the downstream world by tens of millions. Compare **matched arms** with several
  variants each (for example two checkpoints or two neighbouring budgets), not single runs.
- Only complete routes decide. Use boundary values to *select* what to complete, never
  to claim a score.
- Check the realized inputs (fire/focus bits), not the controller label.

## R0. Restore the working environment

The local workspace lost the ledger, launchers and memory guard
([Status](STATUS.md#local-workspace)).

- Recreate `reports/local/` ledger from [campaign-ledger.tsv](experiments/campaign-ledger.tsv);
  the next ID is 2210.
- Write one small tracked runner (Node, not Python) that admits an owner only at
  ≥ 12 GiB host `MemAvailable`, runs it under `prlimit --as=4294967296` and `nice`
  (or a user systemd scope with MemoryHigh 6 GiB / MemoryMax 10 GiB), aborts below
  8 GiB, appends the ledger line and writes `manifest.json`/`exit.json` next to the
  output. One tool replaces the ten near-identical `verify-no-bomb-*.cjs` copies.
- Build `build-native` in Release with the DAT and keep `build-extra-live` only until
  R1 passes.

## R1. Rebuild the record tapes

Both producer tapes (`plan.actions`) are lost; only the replays remain. Continuations
need the exact owner input tape.

- Natural replay rows (`th08_replay_check … ROWS`, 25 little-endian 32-bit fields per
  row; schema in `tests/replay_suite_schema.json`) contain the input of every
  calculation row. Owner inputs and calculation rows differ at stage-control
  boundaries: natural rows trail the producer by 19 inputs per stage handoff
  (0/19/38/57/76/95 in the story).
- Write a C++ tool that converts rows into an owner tape, reconstructing the
  handoff inputs; check it with `th08_shadow_follow` (its `first_mismatch` field
  locates the first wrong input).
- **Acceptance:** fresh follow completes NMNB with 159,445 (story) / 76,725 (Extra)
  owner inputs and the record score; re-exporting a replay from the rebuilt tape
  (`th08_shadow_style` with the whole tape as `--prefix-actions`) produces natural
  rows byte-identical to the record's rows.
- Then map every card's actual owner-input entry (the old maps 2100/2117 are lost);
  phase checkpoints must use these owner counts, not audit clocks.

## R2. Rank by whole-run value

Add an opt-in value criterion; keep the default unchanged.

- Code: `tools/shadow_style.cpp`, `search()` → `finish`; the two comparisons
  `a[s::Field::Score] > b[s::Field::Score]` (fire/form and movement alternatives).
  The projection already carries `PointItems` (cumulative), `Orbs` (current stage),
  `PointValue` (PIV) and `Gauge` (`src/shadow_native.cpp`).
- Value of a certified horizon end relative to its start, using PIV for orbs because
  it is cumulative across stages (ΔT ≈ ΔPIV / 5):
  `V = ΔScore + ΔT·(10·(P_f − P) + 1,500) + ΔP·(5·(T_f − T) + 75,000)`, with P and T at
  the horizon start and plug-in finals `P_f`, `T_f` from the incumbent route
  (1,628 and 234,573 for the story). Use the Extra clear constants (2,000 per orb,
  100,000 per point) on Extra.
- Proposed flags: `--value-ranking 0|1`, `--value-final-points N`, `--value-final-orbs N`;
  report adoptions under both criteria.
- Use the same `V` (cumulative from the stage results) to rank stage-boundary worlds
  in R4 and in `th08_phase_frontier` input rows.
- **Acceptance:** first replay the existing complete runs offline and confirm `V` still
  ranks better than cash on fresh data. Then run matched complete stories (control
  versus value ranking, at least two variants each) from the same prefixes; promote
  only if value ranking produces a complete NMNB route above the record, or clearly
  better Stage 3 boundary values without a worse downstream.

## R3. Plan the gauge in Stages 1–3

Measure before building.

- Diagnostic (read-only, cheap): play the record and the human references through
  `th08_replay_check` and compare `gauge`, `timeOrbs`, `pointItems` and position per
  calculation row, stage by stage and card by card. Locally retained human replays:
  `reports/local/source-human-20261028/downloads/` (Sakuya Extra/Lunatic, Youmu
  Extra). Re-download and hash-bind Levea's Scarlet Lunatic and the IwanaM solo
  Youmu Lunatic replays from the [score table](https://maribelhearn.com/wr#IN).
  Human replays may contain bombs; they suggest timing, not NMNB worlds.
- Mechanics to plan with ([Scoring](SCORING.md#gauge)): shooting moves the gauge up
  to 21 per update toward the focus side after a 30-update ramp; each orb ±111 by
  focus; kills ±200; youkai grazing +100; the capture reward (up to 935 orbs) can
  swing the gauge across its whole range during pickup.
- Build a segment-level gauge target: for each road section and card, the desired
  gauge at entry (youkai extreme before graze-heavy boss phases, human extreme
  before point-heavy waves) and the focus state used while collecting the previous
  capture reward. A natural carrier is a new phase-plan field next to the existing
  capture form (`src/phase_strategy.cpp`, `include/th08/phase_strategy.hpp`).
- First target: card 6 and the Stage 1 road (12,755 versus 21,873 orbs), then cards
  10, 13, 47 and 116 (see [Lessons](LESSONS.md#4-economics-facts) for per-card numbers).
- **Acceptance:** higher Stage 1–3 boundary value `V` in a carried world, then a
  complete route above the record.

## R4. Stage-boundary beam

Replace "one complete future per idea" with selection at stage ends.

- For a stage, produce several end-of-stage worlds from the same incoming tape
  (different phase plans, gauge targets or budgets), stopping at the actual stage
  clear. Rank them by cumulative `V`, keep the best few (with diversity, as
  `th08_phase_frontier` does) and continue only those. Complete futures are run only
  for the final few candidates.
- `--stop-after-card` stops at a card; a stage-clear stop may need a small new
  control. Checkpoints remain literal owner-input prefixes.
- **Acceptance:** at equal compute, more complete candidates above the incumbent's
  Stage 5 value than the old one-at-a-time loop.

## R5. Evaluate solo Youmu

This is a decision for the owner, because it starts a new episode.

- Capture a new Wine seed receipt (`TH08_SHADOW_CHECKPOINT=0,3,10,-1,1`,
  `TH08_SHADOW_PRIOR_FINAL_A_CLEAR=0`; see [Validation](VALIDATION.md#seed-acquisition-wine)).
  The old capture driver in `reports/local/retail-seed-episodes-20261027/*/capture-driver.cjs`
  points at a deleted `game_exe/shadow-runtime-v1`; `game_exe/retail-audit/` has a
  retail copy. Never pick or reroll the seed after seeing Native scores.
- Before tuning, read the solo Youmu code paths in `Player.cpp` (gauge limits ±5000,
  extremes ±3000, `IsSoloHuman`/`IsSoloYoukai` both false) and check whether the
  policy's form handling (`can_switch_forms`, phase capture form) behaves sensibly
  for a solo shot. Then run a plain baseline story with the public profile.
- **Acceptance for continuing:** a baseline within reach of the Scarlet record, or a
  clear source-backed advantage in Stage 1–3 orb production.

## R6. Throughput

- Guidance dominates cost (task 1903: 106 s Native versus 627 s Policy guidance).
  Profile the policy in a Release build before changing search budgets.
- A persistent per-entry worker that re-executes a long prefix once and forks
  candidates copy-on-write would save the 18–24 s prefix replay per owner; measure first.
- The 32-core remote host stays paused until the owner verifies its new host key.

## Not planned

Extra 4 B: points are close to their ceiling (36 M repricing envelope) and the gap is
691 M; revisit only if R2/R3 show large gains on the story. Bombs stay paused.
