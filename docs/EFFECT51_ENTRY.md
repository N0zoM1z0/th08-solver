# Effect51 entry integration

This is one narrow bridge for the actual ID2 Easy practice prelude, not a general
rendering engine or a complete world. `preparations/` remains unchanged.

## Responsibilities

- `effect_animation` validates the exact `enemy.anm` hash and script73 shape once.
  It retains time-zero control, opcode13 angular velocity and rotation, sprite121
  dimensions and callback-visible fields. It deliberately owns no pool or RNG.
- `effect_pool` owns 512 primary slots and the circular cursor. It consumes an
  explicit post-time-zero ANM projection, camera, frame multiplier and shared RNG.
  It knows nothing about ECL programs, enemy actors or timeline execution.
- `practice_entry` decodes opcode139, dispatches only effect51, then acknowledges
  the pending instruction after a successful pool transaction. It never supplies
  missing camera/RNG/pool state. Masked color lvalues remain unsupported; the
  real prelude's immediate ARGB word is preserved without scalar-selector lookup.

Immutable ANM preparation is reused across allocations. Each request stages only
new slot values in reusable scratch, then commits them and RNG together. Snapshot
copies own their slots and do not copy stale staging entries. This avoids an entire
pool copy on each request, without changing the source's allocation order. No
end-to-end speedup is claimed before a complete world benchmark exists.

## Source-sensitive rules

The pinned `EffectManager::SpawnEffect` selects the old cursor slot and advances
the cursor before testing occupancy. One call inspects at most512 slots. Only
actual allocations decrement count; nonpositive counts therefore visit every
free slot. Exhaustion returns sentinel653 even after partial allocation. A full
pool consumes no initializer RNG and requires no camera or ANM input.

Each new effect clears previous storage, executes ANM time zero, applies caller
color/position-offset and flags, then invokes the effect51 initializer. The source
callback always succeeds and consumes16 U16 draws per allocation. Unknown inputs
and numerical errors roll back the native transaction; this is an interface
contract, not a claim that the original engine rolls back failures.

Script73 contains angular velocity and sprite selection at time0, static completion
at time30000, and a sentinel. Angular velocity also affects the time-zero tail.
The projection retains that rotation; it is not a silent visual NOP. Renderer
matrices/textures, general ANM updates, other effect lifecycles and global ANM counters
are outside this component and must be added when their world consumers exist.

## Verification and next boundary

The existing core tests remain, with focused pool and entry regressions for
allocation, missing inputs, retry/fork and the real DAT. The optional
`source_effect_pool` compares6000 transactions against the unchanged hash-pinned
SpawnEffect body and camera callbacks. Its ANM boundary is supplied deliberately;
a separate extracted Initialize/angular-block comparison checks the new time-zero
projection when DAT is passed. This is component evidence, not full ANM/world or
retail x87 equivalence. Template51's script73 mapping and ECL color-lvalue semantics
were checked in the pinned source; they are not independently re-derived by that
allocation adapter.

```sh
./build/effect_entry_tests game_data_donottrack/th08.dat
./build/source_effect_pool game_data_donottrack/th08.dat
./build/th08_first_spell game_data_donottrack/th08.dat ../th08-reference reports/local/effect51
```

With no supplied world state, the original diagnostic still stops at sub0 PC1,
offset260, opcode139, now with `MISSING_ENTRY_STATE`. With an explicitly supplied
empty pool, camera `(position=0, lookAtOffset=(0,0,100), forward=(0,0,1))` and seed0,
it allocates16 effects, consumes256 draws, finishes immediate sub0 spawning and
applies its post-spawn stores. The timeline reaches PC1, offset39716, time2.

Stop there: EnemyManager, effect, background and player phases have not run.
Advancing the timeline alone to its next spawn would invent elapsed world state.
The next work is to own those phases and derive actual entry camera/shared-RNG
state. All three first-spell acceptance gates, complete worlds and solutions
remain zero. The separate supplied-state report records this distinction.

## Owned effect51 update phase

`PrimaryPool::advance_effect51` now owns one restricted primary-pool update:
ascending slots, explicit deathbomb-freeze observation, callback, certified
unit-rate script73 angular/static phase, then the effect timer. A callback cull
commits its earlier motion but skips ANM and timer; static completion retains
visibility and skips angular motion/timer. Rotation sets the source dirty flag.
The returned source active count is measured before retirement, while pool
occupancy reflects the surviving slots. Missing input at any slot rolls back the
whole native phase, permitting an unchanged retry.

The phase has no ECL, timeline or player dependencies. Camera, boss occupancy and
tint are explicit current observations. Unknown occupied slots and uncertified
ANM checkpoints block. Freeze avoids callback inputs and timer advancement.
This reuses the independently checked callback and angular kernels; only a few
ordering/rollback/termination regressions were added to the existing pool test.

This does not yet connect the global scheduler. Source calculation priorities are
Background8, Player9, EnemyManager11, BulletManager12, EffectManager13. The same
frame that spawns sub0 must still run its normal enemy update before the effect
phase. Draw-list construction, rendering callbacks, tamper checks, secondary/fixed
pools and other effect types remain excluded. In particular, draw callbacks can
write ANM fields; do not treat later rendering-dependent state as automatically
candidate-independent. The first-spell diagnostic still stops before the missing
world phases instead of stitching these components together in a guessed order.
