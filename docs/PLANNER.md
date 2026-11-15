# Native planner

`th08_shadow_style` is a synchronous model-predictive controller over the imported
game. The game itself is the forecaster: every committed input is backed by a
complete Native future executed with the true RNG in a forked child. Proposal search
may approximate; Native rollouts decide safety.

## Control loop

```text
for each owner update:
    if it is time to replan (see below):
        candidates = Policy proposals from the current observation
        for each candidate: fork, execute lead + candidate + guided tail to horizon H
        adopt the first certified future (optionally re-ranked, below)
        publish its first P inputs into the ActionBuffer
    input = prefix input | next published input | fallback (hold, dialogue skip)
    owner Session executes input; Policy observes the new state
```

| Control | Default | Meaning |
| --- | ---: | --- |
| `--verify-horizon H` | 64 | Updates every adopted future must survive (powers of two 64..2048), or reach the real terminal |
| `--publish-horizon P` | 64 | Inputs of a certificate released to the owner, `P ≤ H` |
| `--replan-updates R` | 8 | Minimum spacing between searches; a search starts when at most `P − R` inputs remain queued |
| `--lead-updates L` | 8 | Already-queued inputs a new plan must keep unchanged (the latch plus the queue in flight) |
| `--policy-horizon` / `--policy-beam` | 24 / 192 | Depth and width of the Policy proposal beam |
| `--search-budget-ms` | 110 | Wall budget per search, including all forks |
| `--spell-horizon S` | 0 | When nonzero, active spells use H=S, P=S/2, R=S/4 |
| `--nominal-warm-updates` | 0 | Reuse length of the previous certificate's unconsumed inputs |

Without a certificate the owner executes an explicit fallback input (held movement
and focus while shooting, Z cadence during dialogue). Fallback is never a safety
claim. The last scoring campaign used H128, P64, R32, lead 1, policy beam 64–384
and search budgets up to 30 s.

When a fresh search fails but the previous certificate verified beyond the
publication window, the remaining verified inputs may extend the queue (counted as
`retained_windows`). A failed extension contributes nothing.

## Proposals

The Policy (`online.cpp`) is a current-memory heuristic. It sees copied bullets,
lasers, lethal regions, ECL cursors, upcoming spawns, items and the player, and runs
a beam search over its own float kernels. It never steps the native world. Styles
(`baseline`, `zen`, `expert`, `sniper`, `score`, `score-survival`, `score-harvest`,
`score-precollect`, `score-flow`, `graze`) set preference weights; scoring
switches add goals:

- **Collection and aim** (`--collect-items`, `--aim-boss`, `--boss-collect`, `--precollect`):
  climb above the SHT collection line when enough items are visible; follow the boss.
- **Attack pacing** (`--attack-pacing`, `--farm-to-timeout`, `--farm-charge-fire`):
  withhold fire while realized income is positive, then restore fire before the
  capture reserve.
- **Positioning** (`--upper-flow`, `--upper-stage`, `--priced-collection`,
  `--drop-target`, `--drop-follow`, `--parent-chain-bias`, `--hit-income`): stream and
  item-price targets.
- **Form** (`--human-bias`, `--survival-focus`, `--attack-focus-bias`): soft focus
  preferences.

A **guided tail** (`predict_with_policy`) executes the fixed candidate, then lets a
branch-owned Policy copy replan inside the child in batches of at most eight
inputs until the full horizon. Guidance never runs in the owner.

**Recovery** (`--baseline-recovery 1`, `--recovery-style`, `--recovery-fire`,
`--held-recovery`) adds a separate safety Policy and held-direction candidates
when ordinary proposals fail. With `--allow-bombs 1`, one-update bomb prefixes are
tried after ordinary recovery fails.

**Economic re-ranking** (`--economic-proposals N`, `--economic-movement N`): after a
certificate is found, up to N fire/focus variants (and up to N 16-update cardinal
pulses during spells) of its first 32 inputs are verified. A variant replaces the
incumbent only if its certified horizon ends with strictly higher displayed score.
This is the only place the planner compares scores, and it compares **horizon cash**,
which [Scoring](SCORING.md#why-local-cash-misleads) shows is a biased proxy.

## Phase plans

`--phase-plan` accepts up to 32 comma-separated
`stage:card:top:rows:reserve:feedback|shoot:human|youkai` entries (requires
`--attack-pacing 1`). While the actual Native stage and one-based card match, the
Policy targets `rows` rows starting at Y=`top` with 32 px spacing, fires per the
fire mode, and switches to the capture form `reserve` updates before the timer
ends. Example: `8:203:368:3:0:feedback:youkai`. A plan is a proposal preference;
Native recovery may leave the rows. `--phase-recovery-prior 1` lets recovery
share only the capture-form preference. `--stop-after-card C` stops after the
actual close of card C with exit 2 and no replay.

## Goals and constraints

| Goal | Planner flags | Refiner flags |
| --- | --- | --- |
| NMNB, all captures (CLI default) | `--allow-bombs 0 --require-all-captures 1 --verify-captures 1` | `--allow-bombs 0 --require-all-captures 1` |
| NMNB, captures optional (current scoring) | `--allow-bombs 0 --require-all-captures 0 --verify-captures 0` | `--allow-bombs 0 --require-all-captures 0` |
| Bombs allowed (paused) | `--allow-bombs 1 --require-all-captures 0 --verify-captures 0` | `--allow-bombs 1 --require-all-captures 0` |

`--verify-captures 1` rejects futures whose horizon loses a source capture.
No Miss is always mandatory. `--target-score D` makes a complete run below D exit 5
(still exported). `--replay-name` defaults to `ysopwnab` and must be eight bytes.

## Checkpoints

`--prefix-actions FILE --prefix-updates N` executes the first N inputs of FILE
literally from the original entry, then plans from the resulting world. Every
manager, the RNG, items and capture state are the real carried state; nothing is
injected. N counts **owner inputs**, not calculation updates or replay rows.
`--checkpoint-updates K` writes the executed tape and a metadata file every K
inputs so an interrupted owner can resume by literal re-execution; the saved tape
is not a certificate.

## Other tools

- `th08_score_refine` edits windows of a complete incumbent tape (fire, form or bomb
  families), re-executes every complete remaining future and keeps only strictly
  higher terminal scores under the same capture and route goals.
- `th08_phase_frontier INPUT_TSV WIDTH GENERATION OUTPUT_TSV` selects up to 64
  completed segment worlds by score with family diversity. Its keys are
  owner-supplied hashes, not world equivalence.
