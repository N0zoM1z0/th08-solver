# Continuous offline scenarios

The solver-first target is a complete scenario from an explicit reproducible
checkpoint to a declared terminal condition, without launching the game. Menus,
practice preludes, camera and rendering are not prerequisites. This document
separates the implemented synthetic runner from the pending DAT-driven spell adapter.

## Reproducibility and RNG

A scenario has immutable phases, movement/hurtbox values, duration and a visual-hook
policy. Its checkpoint owns frame, live bullets, RNG state and the player position.
The synthetic gameplay stream starts at the selected 16-bit seed; its visual stream
starts at `seed xor 0xa5a5`. The configured number of visual draws per frame may change
without perturbing gameplay randomness. This is a **controlled scenario**, not an
original-game shared-RNG trace or an attempt to reconstruct visual consumers.

Changing a visual policy can change the world when the original game shares its RNG.
We deliberately name that difference rather than claiming retail equivalence.
Gameplay RNG, player-dependent aiming, damage, cancellation and branch-dependent
state may not be silently replaced by a fixed tape. The current synthetic patterns
never read the player's position; only that restricted future can be shared across
candidates or merged by exact position.

## Streaming execution and strategy comparison

`scenario.hpp` owns definitions and checkpoints; `scenario.cpp` owns generation,
forecasting, execution and fresh replay; `scenario_cases.cpp` only handles CLI/reporting.
The runner retains live bullets and RNG across phase boundaries. It does not reset
the scene when replanning. Geometry storage covers the lookahead window, not the
whole stage; the saved action tape grows by one byte per executed frame.

Each collision snapshot is the lethal geometry after that frame's world update,
tested against the player's position after its action. Reported death frames are
one-based. A successful short forecast is committed only for a prefix, then the
runner replans. Default lookahead is 120 frames, committed prefix 30, beam 128 and
200,000 attempted action expansions per plan.

Compare three strategies from the same checkpoint:

- Stationary: stay still, exposing whether the scene is trivial at the start position
- Greedy: choose the next safe position closest to the goal, without future knowledge
- Rolling beam: search a bounded future and commit a short verified prefix

A search limit/exhaustion is reported separately from collision and survival. No
fallback action is invented after search failure, and failure is not an impossibility
proof. Every executed tape, including failed prefixes, is replayed from a fresh copy
of the checkpoint with regenerated geometry and unindexed collision checks. World,
event, route and both RNG digests must agree. This detects index/planner/replay
inconsistency; it is not an independent oracle for the scenario generator itself.

## Profiles and useful failure cases

These are explicitly synthetic stress scenes, not original Touhou stages:

- `relay`: curtain, rings and moving-lane phases
- `lane-switch`: moving-lane, curtain and ring phases in another order
- `late-gate`: an initially quiet scene followed by a wide closing obstacle

The late gate deliberately exposes a limitation of center-seeking, finite-beam
search: early sideways alternatives may be discarded even though an escape route
exists. Setting an early leftward goal is a reproducible witness, not a general
algorithm fix. Preserve the failing baseline when evaluating better heuristics;
raising beam width or changing a goal alone must not be advertised as completeness.

## Reproduce

Build and run the normal CTests first, then collect Release samples without other
benchmarks running:

```sh
./build/th08_scenario_cases --scenario all --strategy all --seed 1 --duration 7200 --output reports/local/scenarios.json
./build/th08_scenario_cases --scenario late-gate --strategy rolling-beam --seed 1 --duration 7200 --goal-x 24 --output reports/local/late-gate-escape.json
./build/th08_scenario_cases --scenario relay --strategy rolling-beam --seed 1 --duration 7200 --visual-draws 0 --output reports/local/no-visual-draws.json
```

The CLI supports JSON/TSV, horizon, committed prefix, beam and per-plan budget.
Exit zero permits a correctly reported collision/search failure; invalid input or
replay disagreement exits nonzero. Compare outcomes and digests before timings.
Reported generation, search, execution and replay timings exclude report file I/O.

## Real-resource progression

The next profile uses actual `ecldata7sp.ecl` ID179 Easy, supplied checkpoint sub72
PC10, and child sub73 through the 1200-frame timeout and explicit end callback.
It needs real ECL scheduling, fast-spawn lifecycle and polar acceleration, not a
repeated leaf-emitter fixture. The profile must state omitted graze/score/visual
feedback and terminal semantics; a controlled survival result is not automatically
a retail practice capture or complete stage. Later stages must preserve live hazards,
RNG and relevant actor state across transitions unless a real transition clears them.

## Checked Release sample (2026-10-02)

On Linux x86_64, Intel Xeon Platinum 8573C, GCC 14.2, Release with contraction disabled,
seed 1 and 7200 frames, both `relay` and `lane-switch` rolling beams survived all frames
and freshly replayed in about 1.84–1.85 seconds each. This includes repeated forecasts,
search and replay, and is a sample rather than a timing guarantee. Stationary/greedy
baselines died at 385/435 and 309/359 respectively. The baseline late gate stopped with
`search_exhausted` at 2310; its stationary/greedy baselines died at 2401. The explicit
left-goal witness survived all 7200 frames.

The two successful rolling runs used 32.22 million attempted action expansions each,
with about 10.2 million duplicate successor queries avoided. Peak geometry horizon
remained 120 frames. Repeating relay with zero rather than two visual draws per frame
preserved its world, events, gameplay RNG, complete action tape and route digest;
only the visual RNG trace changed. See the generated
[full comparison](../reports/native/scenario_summary.json) and
[escape witness](../reports/native/scenario_escape.json).
