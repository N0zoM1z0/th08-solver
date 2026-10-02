# Current handoff status

Reviewed: 2026-10-02. Native headless adaptation and complete real-data solver experiments
are active again at the owner's request. Earlier subset/component evidence remains separate.

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

The table above records the legacy subset engine. Controlled survival is counted
separately from its source-faithful whole-world coverage. The tracked native runtime
has its own profile and evidence:

| Native headless outcome | Verified coverage |
|---|---|
| Complete stages | Easy Stages 1, 2, 4a, 4b and 5, Reimu/Yukari, seed 0, `spell-portfolio`; every tape freshly replayed |
| Complete spell survival | Raw ID179 Easy, stage 6b, seeds 0/1/65535; 1292 updates including wrapper |
| Spell portfolio sweep | 49/56 enumerated Easy standard and Extra checkpoints, seed 0; every tape freshly replayed |
| Transform/profile cross-check | IDs 85/198 pooled-laser motion, ID89 direct ECL, ID93 imminent pooled laser, IDs 193/195 source-vector and ID199 linear ranking complete for seeds 0/1/65535; ID201 bounded WAIT/ECL-shot profile complete for seed 0; ID204 relative-direction profile complete for seeds 0/65535 |
| Genuine failed baseline | Focused stationary ID179, seed 0; collision at update 382 |
| Stage 6b failure diagnosis | Easy seed 0 reactive; update 854, bullet slot 664; late intervention fails 9/9, one-update-earlier leftward intervention survives 3/9 |
| Deterministic feedback | Fresh-process tapes agree on per-frame projection, terminal, RNG and graze/score/gauge |

No universal solver or retail executable equivalence is claimed. Opcode counts and
passing component tests are not completion percentages.

## Results that matter

- The imported production calc chain runs without video/audio initialization or a
  wall-clock limiter. It retains gameplay and ANM updates, original shared RNG,
  dialogue, damage, cancellation, items and graze feedback. It stops at actual stage
  clear or spell end, without resetting the world between boss phases
- A Release Easy stage sweep ran 271449 native updates across all nine stage entries.
  `spell-portfolio` cleared Stages 1/2/4a/4b/5 in 24135/32448/22089/43700/43348
  updates. Stage 3 reached ID32, 6a reached ID139, 6b reached ID167 and Extra reached
  ID192 before their preserved collisions. Planning runs took 79.66 seconds total,
  peaked at 74980 KiB RSS and 1201 live bullets, and every tape freshly replayed
- Stage 6b's reactive failure is now reproducible with native collision bounds and
  independent nine-direction replays. Recording-mode movement uses the previously
  latched input; the heuristic assumes immediate movement and uses center distances
  rather than overlap feasibility. Replacing update 853 with left/up-left/down-left
  avoids update 854; replacing update 854 is too late. This is a local escape witness,
  not stage completion. No strategy parameters or original update order were changed
- The hazard policy accounts for the one-update input latch, forecasts existing native
  lasers, and mirrors active vector-acceleration order from source-owned state. A
  conservative laser broad phase reduced ID151 policy time from about 7.8 to 2.1 seconds
  without shortening its 120-update horizon. The native update remains the collision oracle
- On the fixed seed-0 Easy/Extra matrix, the simple reactive policy completed 19/56
  checkpoints. `spell-portfolio` completed 49/56 with no lost baseline completion;
  all 56 success/failure tapes replayed with matching terminal, RNG, feedback, collision
  and trace projection. The remaining failures are six bullets and one lethal region
- Vector-acceleration projection fixed ID193's transform-0x10 collision and completed
  IDs 193/195 for seeds 0, 1 and 65535. ID199 instead completed all three seeds with
  constant-velocity ranking; its isolated selector lives outside the generic kernel
- Preserving the native final active-laser collision before removal completed ID163.
  Retained failures are bullet IDs 32/139/167/183/202/203 and lethal-region ID192
- ID85's static-angle forecast stopped at the bottom center while ten live pooled lasers
  rotated around `(192,128)`; slot 7 hit on update 631. Replacing update 630 with any
  rightward direction survives that collision. Native before/after observations expose
  slot 7's simultaneous translation and roughly `-0.003802` rad/update rotation. Its
  isolated rigid-motion profile completes the 2792-update wrapper for seeds 0, 1 and
  65535 with fresh replay and O0/O3 agreement
- ID89's unadapted source collision is a direct ECL `CalcLaserHitbox`, not a pooled laser.
  Its 590x160 hitbox first appears on update 393, and single-action replacements at
  updates 391/392/393 fail 9/9. The active ECL cursor exposes constant opcode 137 / EX 9
  from at least update 362. Its isolated adapter constrains candidates across the
  repeating callback interval, then lets the generic scorer handle bullets. It completes
  the 3162-update wrapper for seeds 0, 1 and 65535 with fresh replay agreement
- ID93's slot-0 beam is created during update 1537 and becomes fully lethal on update
  1539. Once the pooled laser is observable at update 1538, all nine input replacements
  still collide; at update 1537, only right survives the fixed suffix. The isolated
  adapter decodes the currently due non-aimed opcode 114 without consuming RNG and
  constrains proposals before spawn. All 82 decoded spawns are supported in each run;
  seeds 0, 1 and 65535 complete the 3392-update wrapper with fresh replay agreement
- ID198's static forecast first meets rotating pooled-laser slot 0 on update 446. Rigid
  motion removes that hit, but nine constant paths later oscillate into a bullet/beam
  trap on update 1062. Its isolated two-leg profile ranks every initial/continuation
  pair from shared immutable forecasts and completes the 4292-update wrapper for seeds
  0, 1 and 65535. On local x86-64 GCC 12.2 Release, difficulty 4, seed 0 and a
  6000-update budget, forecast caching retained the exact action tape while reducing
  decision time from roughly 15.3 to 10.3 seconds; file I/O is outside that metric
- ID201's generic policy hits bullet slot 24 on update 362 while its WAIT transform is
  active. The isolated profile exports a source-bounded count of unchanged-velocity
  updates through WAIT/child-transform boundaries and previews deterministic non-aimed
  ECL shots over a 32-update horizon. Its seed-0 wrapper completes at update 4292;
  15140 source-decoded spawn observations produced 184616 warnings, 638 decisions were
  constrained and no unsupported future was substituted. Fresh replay matches digest
  `954752188841509475`. Seed 1 still collides at 3455 and seed 65535 at 700; seed 1's
  confirmed gap is an RNG-dependent child pattern, so this is not multi-seed coverage
- ID202's generic policy collides at update 3265 with slot 1369, a random child already
  born on update 3258. Its copied WAIT program proves unchanged velocity through that
  hit; enabling only the existing bounded WAIT projection avoids it without changing
  the 12-update horizon. Up/up-left inputs at 3264 avoid the hit; all nine replacements
  at 3265 are too late. The wrapper remains unsolved: seeds 0/1/65535 collide at
  4204/4216/3613, with fresh replay agreement. Seed 0's new blocker is an ECL shot born
  at 4203. A deterministic-preview extension stops earlier because opcode 99 uses
  random-angle selector 10082; no RNG future was guessed or unknown shot omitted
- ID204's generic policy collides at 1205 with slot 669 while relative-direction
  transform `0x40` decelerates it. The isolated source-bounded recurrence includes the
  final turn's movement and stops before a subsequent enabled transform. With the
  same 12-update horizon, seeds 0/65535 complete at 4832 and freshly replay at O0/O3;
  seed 1 still collides at 3008. This adds seed-0 coverage, not all-seed robustness
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
in the [report index](../reports/native/README.md). Older subset samples used Linux
x86_64, Intel Xeon Platinum 8573C, GCC 14.2. Earlier native samples use AMD EPYC 7B12,
GCC 12.2; the ID202 checkpoint uses Intel Xeon Platinum 8573C, GCC 14.2. Both native
profiles are Release with contraction disabled; timings depend on environment.

## Existing reusable components

The repository includes DAT/ECL/SHT/ANM/STD parsing, scalar ECL calls/waits/RNG,
resumable world-effect handoffs, context ownership, timeline/motion kernels, certified
ANM timing, bullet transforms/slot selection, laser collision/lifetime, spatial indexing
and fixed-model planning. [Architecture](ARCHITECTURE.md) maps their owners.

Earlier practice-entry, effect51/62 and camera components remain checked code. They
are optional component evidence, not prerequisites or the next integration roadmap.
The old Wriggle sub40/41 fixtures are fixed-emitter slices, not whole spells.

There are 26 core CTests without private data, plus two optional pinned-source tests
(28 total), and one optional native real-data CTest. The native test covers complete
duration, original boss transitions, the adapted IDs 85/89/93/198/201/204, ID202's partial
WAIT improvement and retained failures, fresh replay, and the Stage 6b
input-latch counterfactuals.
Public CI excludes DAT and reconstruction; it cannot certify those profiles. See
[Validation](VALIDATION.md) for commands and evidence limits.
The current GCC14 host passes core tests and the focused ID202/204 O0/O3/replay fixtures,
but its full native CTest hits the retained GCC12-host Stage 6b golden-digest mismatch.
Clean-main execution reproduces it; outcomes and collision details are unchanged.
The aggregate is not reported as passing on this host.

## Next useful work

1. ID202's remaining seed-0 failure needs advance emission awareness at update 4203.
   Its other ECL shots and child patterns consume random angles. Evaluate a conservative
   emission envelope before considering branch-owned full-world RNG; do not enable the
   deterministic ID201 preview while silently excluding those random shots
2. Preserve ID204's seed-0/65535 completion and diagnose its remaining seed-1 update-3008
   untransformed-bullet collision after the seed-0 portfolio gaps. Do not broaden its
   active relative-direction forecast beyond the source-owned transform bound
3. Diagnose ID139's update-5326 bottom-excluding bounce (`0x800`). Mirror the source
   boundary-test/update order in a case adapter, then use Stage 6a as the continuous gate
4. Probe ID32's untransformed update-594 hit and the matching Stage 3 failure. Compare
   constant, longer-horizon and two-leg proposals on the same prefix; retain the simplest
   route that completes the whole wrapper and then the stage
5. Apply the same earliest-avoidable-decision procedure separately to untransformed
   ID167 (update 932), ID183 (1041) and ID203 (1711). Do not share a tuned profile until
   their source traces establish the same mechanism; Stage 6b is their continuous gate
6. Expose the source owner, lifetime and future geometry for ID192's lethal region at
   update 466. It needs a typed warning distinct from bullet and laser projection. The
   Extra run must pass it without resetting carried state; final clear is the aggregate gate
7. Return to ID201 robustness after the seven seed-0 failures. Seed 1's random child
   pattern needs owned RNG/order evidence or a justified conservative envelope; never
   reuse one sampled future across action-dependent branches. Recheck seeds 0/1/65535
8. After each isolated fix, rerun its baseline, full wrapper, fresh replay, the 56-case
   sweep and the affected continuous stage. Record failed seeds as failures rather than
   changing budgets, tie-breaking or RNG consumption to improve the count

Do not return to full camera/menu reconstruction merely to unblock a controlled
benchmark. Do not concatenate isolated spell fixtures and label the result an actual stage.
