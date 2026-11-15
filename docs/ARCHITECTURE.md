# Architecture

[Status](STATUS.md) owns current results; [Planner](PLANNER.md) owns the search
algorithm; [Validation](VALIDATION.md) owns commands and provenance;
[Scoring](SCORING.md) owns the game's score mechanics.

## Pipeline

```text
Wine retail game ──(pre-RegisterChain seed receipt)──► fixed episode (stage, difficulty, shot, seed, history)
                                                            │
                         th08_shadow_style ◄────────────────┘
                         │  owner Session executes one input per update
                         │  Policy proposes candidates from copied observations
                         │  forked children verify complete true-RNG futures
                         ▼
               plan.actions + native.rpy + score-audit.json
                         │
   th08_shadow_follow (fresh re-execution) + th08_replay_check (natural playback)
                         │
                 selected .rpy ──► optional retail Wine playback
```

## Code map

| Responsibility | Files |
| --- | --- |
| Imported game and headless session | `third_party/th08/src/` (upstream translation units), `modern/headless/session.*`, `runtime.*`, `d3d_null.cpp` |
| Source capture audit | `modern/headless/score_audit.*`; one catalogue shared by reports and the capture goal |
| Natural replay playback and rows | `modern/headless/replay_check.*`, `tools/replay_check.cpp` |
| Wire projection, fork verification, action queue | `shadow_native.*`, `shadow_process.*`, `shadow_rollout.*`, `shadow_plan.hpp`, `shadow_protocol.hpp` |
| Policy-guided futures | `shadow_proposal.*`; the child owns the guided Policy copy |
| Offline scoring CLI | `tools/shadow_style.cpp` |
| Fresh tape re-execution | `tools/shadow_follow.cpp` |
| Complete-tape refinement | `score_refine.*`, `tools/score_refine.cpp` |
| Proposal policy | `online.hpp`, `online.cpp`, `online_*.hpp`, `online_model.cpp`, `online_value.*`; owns no native world |
| Native observation adapter | `online_native.*`, `anm_terminal_clock.hpp`, `direct_laser_policy.hpp`, `ecl_turn_hint.hpp` |
| Stage/card phase plans | `phase_strategy.*` |
| Segment frontier selection | `phase_archive.*`, `tools/phase_frontier.cpp` |
| Tape parsing, route input, rollout contracts | `plan_prefix.hpp`, `plan_route.hpp`, `plan_contracts.hpp`, `plan_beam.hpp` |
| Shared float kernels used by the policy | `kinematics.hpp`, `bullet_motion.hpp`, `timing.hpp`, `geometry.hpp` |
| Replay corpus gate | `tools/replay_suite.cpp`, `tools/headless_report.hpp` |
| Wine observer, owned launcher and live input hook | `tools/wine-shadow/`, `shadow_bridge.hpp`, `shadow_start.hpp`, `render_timing.hpp` |
| Online MPC (retained, paused) | `tools/shadow_online.cpp`, `shadow_recorded.*`, `shadow_compare.hpp`, `tools/shadow_export.cpp`, `tools/online.cpp`, `online_live.*`, `tools/online_live.cpp`, `tools/value_collect.cpp`, `tools/value_train.cpp`, `models/`; see [Planner](PLANNER.md#online-mpc-retained-paused) |
| Earlier fork planner (online verifier) | `tools/plan.cpp`, `plan_observation.hpp`, `plan_matrix.hpp` |

Paths are under `include/th08/` or `src/` unless qualified. The imported game is
built only with `TH08_HEADLESS=ON` ([Headless.cmake](../cmake/Headless.cmake)); the
default build covers the asset-free libraries and contract tests.

## Ownership and lifetimes

- `headless::Session` wraps process-global managers. It is noncopyable and allows one
  instance per process. Never memcpy its state or share one future between actions.
- A Native future is a `fork()` of a single-threaded owner. The child owns the complete
  world, input latch, true RNG and a private scratch directory (score/config files).
  It reports through a bounded shared arena that the parent reads only after the
  child exits. Death, native fault, timeout and incomplete horizons never certify
  any prefix; interrupted work is charged its full requested update count.
- `ActionBuffer` (`shadow_plan.hpp`) is the committed frontier. A new certificate
  may replace only unexecuted inputs, after every executed receipt since its origin
  matches the predicted prefix. Execution never moves backwards.
- The policy owns observation history, forecast buffers and tie RNG per instance.
  The adapter copies native views; no native pointer crosses into the policy.
  A guided future uses a branch-owned copy inside the child, so the owner's policy
  history and tie RNG stay untouched.
- Phase plans and archives are immutable after parsing and shared read-only.

## Update order and RNG

The session calls the original ordered calculation chain once per supplied input,
with synchronous resource initialization and
`timeGetTime = floor(frame * 1000 / 60)`. It omits drawing and text rasterization,
not GUI/message logic. Gameplay RNG is the original shared RNG; there is no visual
RNG split.

In recording mode the supervisor samples input at priority 0, the player moves at 9,
bullets update/collide at 14 and the replay manager latches movement at 17, so the
player reads the previous latch. Proposals account for that pending input instead of
reordering the game.

`State::calculation_updates` counts actual `gameplayFrameCounter` increments.
Stage-control calls can return before that increment, so **owner input counts and
calculation counts differ**. Checkpoints (`--prefix-updates`) are owner input counts.

`SampledGameplay` (used for all scoring episodes) applies the supplied sample at
the GameManager input boundary after synchronous initialization; the default
`Controller` profile keeps the original first-new-stage zero sample. Retail replay
playback cannot select the sampled profile; both preserve the movement latch.

## Numerical profile

Native float32 with contraction disabled and no fast-math. Nothing here claims
retail x87/Direct3D equivalence. Replay agreement between the producer, a fresh
follower and natural playback proves determinism of this implementation, not
retail physics; retail playback in Wine is the separate external check.

## Failure meaning

Unknown gameplay opcodes or unsupported state stop the affected run. Search
exhaustion, runtime failure, budget exhaustion and player death are distinct
outcomes. A failed or incomplete rollout cannot certify a prefix, and only a
complete route establishes a route score.
