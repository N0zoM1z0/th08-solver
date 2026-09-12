# Complete-coverage roadmap

Reviewed: 2026-09-12. Everything below is an acceptance contract or unfinished
integration task, not an assertion that the corresponding world already runs.
Current verified complete worlds and spell solutions: **0**.

## What complete coverage means

The structural audit indexes 222 original spell IDs and 431 spell-start occurrences.
An ID is not one interchangeable execution case. Retain at least:

```text
DAT/member hash + entry mode + timeline/sub/PC/offset + difficulty/override
  + selected spell ID + character/shot/power/form + entry-state provenance
  + shared RNG state + numerical/timing profile + player actions
```

The existing [spell-site ledger](../reports/native/spell_sites.tsv) and
[execution matrix](../reports/native/slice_matrix.tsv) preserve structural identities
and restricted blockers. They are not yet a terminal-world coverage registry.
Do not assign a percentage by dividing supported opcodes or successful slices by
these counts. Continuous player actions and entry states also require explicit
domains and justified equivalence classes, not an invented finite case count.

Each claimed complete case must establish:

1. A source-derived entry, or an explicitly labeled supplied checkpoint with its
   dependencies. No guessed globals, seed, damage state or empty pool.
2. Every reachable resource/effect and frame phase within the declared case domain,
   including callbacks, child contexts, allocation failure and all RNG consumers.
3. Candidate-sensitive updates when player actions change aim, alignment, damage,
   random branches, resource lifetimes or future allocations.
4. The real terminal transition, not an arbitrary horizon or the return of a callee.
   Keep capture, timeout survival, player death and unsupported execution distinct.
5. Independent replay of the complete action route and ordered events, with input
   hashes, numerical profile and first-divergence diagnostics.
6. Measurements on the verified workload before further optimization. No claim of
   globally fastest or mathematically optimal solving without a corresponding proof.

Proposed terminal-world result labels are `CAPTURED`, `SURVIVED_TIMEOUT`,
`PLAYER_DEAD`, `UNSUPPORTED_WORLD_EFFECT`, `MISSING_ENTRY_STATE`, and `SEARCH_LIMIT`.
They are not a currently implemented complete-world API. Search exhaustion does
not establish impossibility; timeout survival does not establish normal capture.

## Dependency-ordered unfinished work

| Order | Deliverable | Existing building blocks | Acceptance before marking complete |
|---|---|---|---|
| 1 | Actor spawn ownership and entry | Owned 480-slot restricted spawn transactions, timeline connection, resumable ECL and context snapshots | Extend the checked spawn prefix through effect51, ANM, complete frame tails and parent-link timing; derive all surrounding entry state |
| 2 | Complete enemy/context lifecycle | Scalar calls/waits, motion handlers and separate integration phases | Main/child context order, same-frame installation/re-entry, callback and interpolation lifetime, form masks, pause/death/offscreen gates, destruction and inherited state |
| 3 | ANM/effect/background world consumers | ANM scalar/control projection, parsed STD, effect-51 callbacks | Required visual/resource fields retained, camera evolution, effect allocation/ANM/freeze/retirement, correct shared RNG order under pool contention |
| 4 | Shot dispatch and bullet/laser ownership | Launch, transforms, slot index, motion and geometry kernels | Distance/alignment/rank/deferred gates before the correct operand reads, successful allocations, child patterns, sprite changes, collision windows, cancellation and retirement |
| 5 | Player, damage and spell endings | SHT parser, movement parameters in fixtures, source-derived Wriggle contract | Actual movement/form/power/shot lifecycle, familiar-parent damage, death/bomb/capture validity, life/timer callbacks and real ending transitions |
| 6 | Candidate-dependent solving | Bounded planner, owned snapshots and unindexed replay | Fork all relevant world state and shared RNG; merge/cache only with dependency/equivalence evidence; independent complete-route replay |
| 7 | Expand every indexed family and entry variant | Complete structural indices and the first-world acceptance pattern | Case registry records source identity, supported domain, earliest blocker or terminal evidence for every indexed occurrence and relevant variant |

These are dependency seams, not seven isolated implementations. For example,
effect-51 camera-dependent lifetime affects successful later allocations and RNG,
so step 3 is required to validate step 1's real practice prelude. Source-oracle
expansion and numerical checks accompany each step rather than waiting until the end.

## First complete world: ID2 Easy, then Wriggle IDs 3..5

The [first-spell execution plan](FIRST_SPELL.md) selects one case before expanding
the family. Implement its reachable dependencies end to end; the layer table above
is not an instruction to finish a universal engine before attempting a route.

Use the exact [Wriggle world contract](WRIGGLE_WORLD_CONTRACT.md), starting with
`ecldata1sp.ecl` timeline 0. Current building blocks do **not** cross these milestones:

| Milestone | Required observations | Current missing integration |
|---|---|---|
| Practice entry reaches the selected spell | Timeline spawns sub0 then sub42; EX19 publishes the explicit selected ID; spell main sub24 starts at life1500 with effective callback41 | Owned spawn transaction, prelude effect pool/camera, wrapper effects and player/global entry state |
| Both familiar waves run correctly | Parent/main/child identities, immediate spawn and same-frame child execution, orbit motion, human/youkai masks and source-ordered shooting | Entity links, child contexts, alignment effects and shot dispatch |
| Player choices produce the right world | Paired routes change aimed shots, random movement, damage and allocation/RNG traces as predicted | Player/damage execution and coherent complete-world checkpointing |
| Genuine terminal transition and replay | Capture and timeout produce their distinct callbacks, cancellation, boss removal and remaining collision phases | Spell lifecycle, terminal ownership and independent full-world replay |
| All four difficulties and stage entry | Each difficulty and relevant form/character state; separately derived ordinary-stage inheritance | The earlier nonspell, dialogue and stage state cannot be replaced by practice defaults |

The timeline's sub0 spawn is now selected and retained by the owned prefix, but not
completed. With explicitly supplied GUI gates, it executes interaction opcode80 and
stops at effect51 creation (sub0 PC1 offset260); no world acceptance gate is passed.
The practice wrapper separately begins with EX136/19. Reporting only the wrapper's
restricted first blocker must not imply that the earlier prelude was executed.

An ECL `frame_complete` signal is not successful spawn completion. Source spawn
executes callbacks/child contexts, movement and shot/ANM work before final metadata.
`SpawnEnemy2` also copies the thirty scalar words and may overwrite life afterward.
These ordering requirements are documented in [Context storage](ECL_CONTEXT_STORAGE.md).

## Remaining families and geometry domains

IDs 2..5 are an integration choice, not a reduction of scope. Other Wriggle families,
ordinary stages, practice, Extra, Last Word and all other indexed spell sites remain
open until their own entries and reachable effects satisfy the contract above.
Reisen collision-restoration windows, Double Spark and the three-barrier cases need
explicit visibility-versus-collision and phase ownership. Rising and Hourai Jewel
need their own RNG/lifetime/re-aiming dependencies. These names come from the task's
preparation queue; no complete native solution is currently claimed for them.

Every geometry consumer must first resolve whether the source invokes collision,
which phase supplies position/dimensions, and which gates apply. A correct narrow
predicate alone does not implement a spell's collision window. Preserve signed laser
dimensions and source numerical semantics; use broad-phase acceleration only when
independent scans retain every lethal contact in the declared domain.

## Completion and non-goals

The all-case objective is complete only when the case registry, world implementation,
terminal traces and independent route replays agree for every claimed domain, with
remaining unsupported inputs stated explicitly. Parser coverage is already broad;
world integration and complete-route evidence are the principal remaining work.

Wine/game launches, a Linux game port, live input control and latency compensation
remain outside this offline phase. Native float32 agreement is not retail x87/D3DX
agreement. Neither boundary may be silently expanded to make a completion claim.
