# Current handoff status

Reviewed: 2026-10-02. The owner is taking over locally. TH08 feature work is paused;
this page records verified behavior and suggested next work, not an active task queue.

## Verified coverage

| Outcome | Verified count |
|---|---|
| Indexed original spell IDs | 222 |
| Indexed spell-start occurrences | 431 |
| Complete offline spell worlds | 0 |
| Complete offline spell solutions | 0 |
| DAT-derived controlled complete survival profiles | 1: ID179 Easy |
| Synthetic 7200-frame profiles solved by baseline rolling beam | 2 |
| Synthetic 7200-frame profiles solved with optional goal recovery | 3 |
| Synthetic 72000-frame continuous profiles with replayed routes | 1 |
| Older source-driven 600-frame particle fixtures | 2 |

Controlled survival is counted separately from source-faithful whole-world coverage.
There is no verified complete original stage, retail practice capture or universal
solver. Opcode counts and passing component tests are not completion percentages.

## Results that matter

- Synthetic relay/lane-switch retain live bullets and both RNG streams across phases.
  Both baseline rolling planners survive 7200 frames and regenerate/replay their tapes
- A closing-gate case exposed center-seeking beam pruning. Optional geometry-derived
  goal recovery completes it without a hardcoded scene/escape coordinate. It shares
  the original 200000 expansion budget: measured peak 198117, with target probes reported
  separately. Default behavior and the failing baseline remain available
- A 72000-frame relay (20 simulated minutes) survived and freshly replayed: 38320 births,
  2400 decisions, 120-frame peak forecast and 27471 peak model bullet references. Local
  Release runtime was about 21.1 seconds including replay, not a timing guarantee
- Actual DAT ID179 executes main sub72, child73, fast-spawn/polar bullets, 1200 lethal
  phases and the timeout/end callback. Full-horizon and rolling planning survive seeds
  0, 1, 65535; stationary fails all three and greedy succeeds only 0. Every result,
  including collision/search-failure prefixes, is independently replayed without the index
- For that short player-independent spell, full-horizon solve/replay took 92–94 ms,
  rolling 387–401 ms. Repeated overlapping forecasts/searches add work; choose strategies
  using evidence rather than assuming rolling is always better
- Early exact-successor dedup avoids 32.6% of collision queries in the moving benchmark
  and 86.5% at a clamped corner, preserving routes, attempted budgets and tie-breaking.
  Both existing DAT sub40/41 route files remain byte-identical

Exact profiles and exclusions are in [Scenarios](SCENARIOS.md); measured records are
in the [report index](../reports/native/README.md). Samples were collected on Linux
x86_64, Intel Xeon Platinum 8573C, GCC 14.2, Release, contraction disabled.

## Existing reusable components

The repository includes DAT/ECL/SHT/ANM/STD parsing, scalar ECL calls/waits/RNG,
resumable world-effect handoffs, context ownership, timeline/motion kernels, certified
ANM timing, bullet transforms/slot selection, laser collision/lifetime, spatial indexing
and fixed-model planning. [Architecture](ARCHITECTURE.md) maps their owners.

Earlier practice-entry, effect51/62 and camera components remain checked code. They
are optional component evidence, not prerequisites or the next integration roadmap.
The old Wriggle sub40/41 fixtures are fixed-emitter slices, not whole spells.

There are 25 core CTests without private data, plus two optional pinned-source tests
(27 total). The local Release suite and actual-DAT checks passed at handoff. Public
CI excludes DAT and reconstruction; it cannot certify those profiles. See
[Validation](VALIDATION.md) for commands and evidence limits.

## Recommended next steps for local work

1. Reproduce the checked profiles on the local machine before changing algorithms
2. Broaden real-DAT complete checkpoints and seeds, keeping failures and simple
   baselines. Prioritize gameplay opcodes/lifecycles that unlock meaningful whole scenes
3. For a genuine stage segment, define entrance and terminal state, carry actors,
   bullets and RNG continuously, and implement actual transition/cancellation lifetimes.
   ID179's despawning slots cannot currently be treated as an empty next-stage pool
4. Add candidate-owned state before caching futures affected by aiming, damage,
   form/graze feedback, RNG or pool contention. Position alone is not a world-state key
5. Optimize measured end-to-end bottlenecks; retain outcome, action and replay checks.
   Recovery remains a heuristic, not complete or optimal search

Do not return to full camera/menu reconstruction merely to unblock a controlled
benchmark. Do not concatenate isolated spell fixtures and label the result an actual stage.
