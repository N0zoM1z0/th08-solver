# Current handoff status

Reviewed: 2026-10-03. Native headless adaptation and complete real-data solver experiments
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
| Complete stages | Easy Stages 1, 2, 3, 4a, 4b, 5, 6a and 6b, Reimu/Yukari, seed 0, `spell-portfolio`; every tape freshly replayed |
| Complete spell survival | Raw ID179 Easy, stage 6b, seeds 0/1/65535; 1292 updates including wrapper |
| Spell solver sweep | 56/56 enumerated Easy standard and Extra checkpoints, seed 0: 54 rolling-portfolio clears plus bounded native-prefix repair for IDs202/203; every selected tape freshly replayed |
| Transform/profile cross-check | IDs 85/198 pooled-laser motion, ID89 direct ECL, ID93 imminent pooled laser, IDs 193/195 source-vector and ID199 linear ranking complete for seeds 0/1/65535; ID201 bounded WAIT/ECL-shot profile complete for seed 0; ID204 relative-direction profile complete for seeds 0/65535; IDs32/139/167/183 two-leg profiles complete for seeds 0/1/65535 |
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
- The current stage records cover 393119 native updates across nine entries,
  including the later Stage3/6b replacement runs. `spell-portfolio` clears Stages
  1/2/3/4a/4b/5/6a/6b in 24135/32448/39767/22089/43700/43348/61041/58853 updates.
  Extra now passes ID192 and collides in ID202 at 67738. Every tape freshly replays; native carried
  bullets, items and RNG persist between phases
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
  checkpoints. `spell-portfolio` completed 54/56 with no lost baseline completion;
  all 56 success/failure tapes replayed with matching terminal, RNG, feedback, collision
  and trace projection. IDs202/203 additionally complete through the bounded repair recipe
- Vector-acceleration projection fixed ID193's transform-0x10 collision and completed
  IDs 193/195 for seeds 0, 1 and 65535. ID199 instead completed all three seeds with
  constant-velocity ranking; its isolated selector lives outside the generic kernel
- Preserving the native final active-laser collision before removal completed ID163.
  The rolling profile retains bullet failures IDs202/203; bounded repair completes ID203
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
- ID32's corner collision at update 594 is avoidable by departing one update earlier.
  A 13-update horizon removes it but still collides at 1394; the next counterfactual
  requires a one-update diagonal leg before rightward continuation. Its isolated
  81-path profile clears all three 2372-update wrappers and continuous Stage 3 at
  39767. Fresh/O0/O3 replay agrees; both the original failure and explicit extra
  horizon/candidate costs remain visible
- ID167's unchanged-horizon two-leg profile avoids entering the bottom-left trap that
  defeats the constant-path baseline at 932. It clears all three 5492-update wrappers
  with 444852 candidate evaluations each and fresh/O0/O3 agreement. Continuous Stage6b
  at that checkpoint advanced from its ID167 collision at 42560 to ID183 at 54401;
  the subsequent ID183 profile provides the current stage clear
- ID183's baseline collides at 1041. The H13-only full-wrapper experiment still
  fails at 820; H12/two-leg4 also fails at 819.
  The fatal large bullet is already observed 31 updates earlier. Explicit H32/81-path
  planning clears all three 1292-update wrappers and continuous Stage6b at 58853,
  with fresh/O0/O3 agreement and unchanged source projection
- ID139's baseline hits a bottom-excluding bounce at update 5326. Source projection
  uses sprite bounds and the pre-movement boundary test through the final bounce.
  Bounce-only horizon 12 still fails at 5327; 81 two-leg paths at horizon 12 fail at
  4355. The observed 31-update approach motivates an explicit 32-update horizon.
  The resulting isolated profile clears seeds 0/1/65535 at 7292 and continuous Stage 6a
  at 61041, with fresh-process and O0/O3 replay. It retains the baseline failure and
  reports the larger candidate/projection cost rather than hiding it
- A pre-existing empty native `fsincos` helper left laser-cancellation item positions
  uninitialized. Long Stage 6a O0/O3 score divergence first exposed it at update 32432.
  The helper now writes both outputs using the established native sin/cos path. Eighteen
  real-runtime item geometry cases fail before the fix and pass at O0/O3 afterward;
  complete Stage 6a now also agrees. This fixes native-profile undefined behavior,
  without claiming retail x87 equivalence
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
GCC 12.2; the ID32/139/167/183/192/202/203/204 checkpoints use Intel Xeon Platinum 8573C, GCC 14.2. Both native
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
(28 total), optional headless process-protocol/semantic-contract tests and one optional native real-data CTest. The native test covers complete
duration, original boss transitions, the adapted IDs 32/85/89/93/139/167/183/192/198/201/204, ID202's partial WAIT improvement, ID203's bounded repair and retained rolling failures, fresh replay, and the Stage 6b
input-latch counterfactuals.
Public CI excludes DAT and reconstruction; it cannot certify those profiles. See
[Validation](VALIDATION.md) for commands and evidence limits.
The historical GCC12 Stage6b digest differs on this GCC14 host. Its old numerical
profile did not pin the complete solver build, libm or CPU, so it cannot promise
cross-environment trajectory-bit equality. The named stage6b-semantic-v1 contract
retains the historical literal/report, asserts the full scene/terminal/RNG/feedback
and collision boundary, and labels numerical-profile identity unverified. Same-build
fresh replay, O0/O3 comparison, diagnostics and all 18 latch probes remain strict.
The exact historical trajectory difference remains unexplained; this is not a
cross-host or retail bit-equivalence claim.

- ID192 now uses typed source-owned body rectangles and outward-rounded all-angle
  random-move envelopes, with H12/81 paths/one-update first leg. Seeds 0/65535 complete
  3692 updates; seed 1 retains a bullet collision at 1470. Actual Extra advances from
  ID192 at 7424 to ID202 at 67738. Body overlap joins the existing earliest-overlap
  ranking on each actual path; it is not a body-only admissibility filter
- The WAIT certificate includes DESPAWN's final lethal fired movement. ID203's earlier
  WAIT/vector-only profile failed at 1711/4823/2896. Its new observed H32 beam retains
  seven player-path prefixes per first direction (63 total), but still fails at 787
  for all three seeds. A bounded native repair automatically derives rollback proposals
  from the failed trace and completes seed 0 at 5492 after 91 candidates. Each candidate
  reconstructs the actual world/RNG in a fresh process; only the proposal beam shares
  immutable observed hazards. Seeds 1/65535 have no repaired completion claim
- Repair uses at most 2 rounds, 8 rollback segments per round and 9 direction holds of
  16 updates, with 144 candidate/2160000 native-update/600-second search limits. Initial
  execution and final fresh replay have separate costs. All failed candidates remain
  recorded; interrupted/unverified children carry a conservative full-frame-cap charge.
  The observed beam expands at most 16533 prefixes per decision, with stable original
  score ordering and deterministic first-action-family retention

## Next useful work

1. ID202's remaining seed-0 failure needs advance emission awareness at update 4203.
   Its other ECL shots and child patterns consume random angles. Evaluate a conservative
   emission envelope before considering branch-owned full-world RNG; do not enable the
   deterministic ID201 preview while silently excluding those random shots
2. Preserve ID204's seed-0/65535 completion and diagnose its remaining seed-1 update-3008
   untransformed-bullet collision after the seed-0 portfolio gaps. Do not broaden its
   active relative-direction forecast beyond the source-owned transform bound
3. Apply the bounded native repair to ID202 with an explicitly evidenced proposal
   profile. The current repair trigger supports H32; do not silently treat ID202's
   original H12 trace as that profile or skip unsupported hazards
4. Finish actual continuous Extra beyond ID202, preserving carried world/RNG state.
   Keep the historical cross-host numerical-profile limitation explicit; its named
   semantic contract does not detect every possible intermediate float drift
5. Return to ID201 robustness after the remaining seed-0 failure. Seed 1's random child
   pattern needs owned RNG/order evidence or a justified conservative envelope; never
   reuse one sampled future across action-dependent branches. Recheck seeds 0/1/65535
6. After each isolated fix, rerun its baseline, full wrapper, fresh replay, the 56-case
   sweep and the affected continuous stage. Record failed seeds as failures rather than
   changing budgets, tie-breaking or RNG consumption to improve the count

Do not return to full camera/menu reconstruction merely to unblock a controlled
benchmark. Do not concatenate isolated spell fixtures and label the result an actual stage.
