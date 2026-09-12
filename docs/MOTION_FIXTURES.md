# Source-driven particle integration fixtures

`th08_motion_cases` connects native ECL scheduling, launch kinematics, certified ANM
timing, particle motion, indexed geometry, finite-horizon search, and unindexed replay.
It deliberately reports `PASSED_COMPONENT_FIXTURES`, not a complete spell solution.

## Exact scope

- Resource: pinned DAT, `ecldata1.ecl`, sub40 or sub41, inclusion mask 8.
- Entry: emitter fixed at (192,96); empty transform table and bullet pool; cursor zero;
  no rank adjustment, distance/alignment gate, suppression, deferred firing or cancellation.
- Each run requests and instantiates 840 type-2 bullets. Total births are below the
  1536-slot capacity even without recycling, so allocation contention cannot occur here.
- The main sprite is certified from `etama.anm` script index 2; color-specific sprite
  dimensions come from the resource. Type-2 collision full size is (4,4), from the
  pinned `BulletManager::AddedCallback` template rule.
- Fast-spawn script index 21 is certified to complete at time 10. Its template has
  already executed time zero; a copied bullet activates on its tenth update. That update
  performs both spawning displacement and fired motion.
- The only installed motion transform is relative direction change, optionally preceded
  by cull-delay installation. Births use the source-verified eighteen-record executor,
  including its gates and speed sentinel. The particle adapter requires the remaining
  cursor to be at a zero record or end of table, and rejects other active effects.
  This establishes that no later installation needs to be simulated in these fixtures.
- Numeric profile: modern-port float32 velocity, unit frame rate. Particle motion is
  pre-collision and unaffected by the candidate player in these explicitly bounded fixtures.
- Player starts at (192,400), stays focused, and uses axis/diagonal speed and half of
  the hurtbox size from `ply00a.sht`. Horizon is 600 collision phases after movement.

## Evidence and interpretation

Each case produces 600 per-frame records, a cumulative particle-state digest, and a
600-action route. sub40 covers 207277 live-update attempts with a peak of 568 lethal
bullets; sub41 covers 216393 with a peak of 609. Both routes are replayed using an
unindexed collision scan. Digests are reproducibility records, not independent physics
oracles. The independent source oracle separately tests launch, direction transforms,
laser lifetime and collision predicates; lifecycle boundary tests cover spawn activation
and culling.

The optimized proposal preserves all actions, positions and expansion counts of the
retained unoptimized reference on 80 deterministic comparison scenes. Both integrated
routes also remained byte-for-byte identical after the search change. An alternating
14-batch benchmark measures the old and new searches in the same process; it does not
claim complete-world throughput or optimality.

## Missing context

These subprograms are called by a larger enemy/familiar world. The parent call graph,
moving emitters, damage, power and focus transitions, alignment/shot gates, RNG consumers,
pool interference, cancellation, and stage/spell lifecycle must be assembled before
these routes can be called Wriggle spell solutions. No game execution or input control
is performed, and no complete spell has yet been verified.
