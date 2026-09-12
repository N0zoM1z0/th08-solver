# Architecture and contracts

## Data flow

```text
User DAT -> Archive decode -> Ecl instruction arena -> predecoded Program
                                      |                      |
                              complete source index    emission/transform events

Validated hazard phases -> owned Snapshot -> bounded beam search -> reference replay
```

`th08_motion_cases` connects these pipelines for two explicitly configured,
source-driven particle fixtures. They are not yet a complete world executor.
An emission request cannot be treated directly as an active hazard.

## Resources and programs

`Archive` owns the DAT bytes read once. LZSS uses a fixed 8 KiB dictionary, a bit
reservoir, and a single output allocation. Block decryption reuses a fixed scratch
array. End padding is allowed only after the complete declared output has been
produced, to decode the terminator. Names, section ranges, output sizes,
uninitialized dictionary reads, ECL instruction sizes, and jump boundaries are checked.

`Ecl` stores one contiguous instruction arena; subprograms store ranges into it.
Operands remain in the caller-owned decoded resource and use explicit little-endian
reads rather than host structure layouts. Every observed opcode has a checked payload
schema. Timeline instructions, SHT headers and descriptors, ANM v3 entries/sprites/scripts,
and STD objects/quads/instances/instructions have native bounded parsers. Parsing does
not execute those resource programs. Shared SHT descriptors retain their occurrences;
ANM raw IDs and directory indices remain distinct.

`Program` predecodes one subprogram into fixed-size records and resolves supported
jumps to array indices. `Module` owns all predecoded subprograms in a resource, reused
across matrix entries. `Workspace` reuses register validity flags, emission records,
and transform-write records across executions. Each emission references the number
of preceding transform writes, avoiding a full transform-table copy per emission.

Restricted execution supports scalar arithmetic, trigonometry, point geometry, all
twelve conditional branches, normal calls/returns, secondary-clock waits, unconditional
and decrement jumps, transform descriptors, and shot requests. Uninitialized registers
cannot be read. Typed local, entity, extra, and call-parameter storage can be explicitly
initialized. Computed engine fields are not fabricated. Float selectors truncate before
dispatch; float-to-int reads truncate toward zero; unmapped rvalue selectors remain raw.
Unmapped lvalues would modify bytecode and remain unsupported. Signed integer overflow,
non-finite results, and division errors stop explicitly.

Normal calls save the caller clock and context registers, inherit local storage, and
load call parameters from shared slots. Return restores context storage but preserves
entity/shared storage. Calls exceeding fifteen saved frames, disabled-stack behavior,
negative/truncated subprogram IDs, child-context lifetimes, and callbacks remain outside
the verified call subset. A standalone `Program` cannot resolve a call without a `Module`.

Shot records do not execute random spread, aimed direction, distance suppression,
rank adjustment, deferred dispatch, or pool allocation. Reusing a resulting model
still requires dependency evidence from the world layer.

An optional caller-owned RNG enables isolated scalar execution, including random
integer/unit/signed-unit/angle selectors and random-sign assignments. Calls use the
same stream; context return never restores it. The current verified instruction
domain permits at most one RNG-consuming expression: two random operands, or a
random sign combined with a random operand, stop as unsupported because expression
evaluation order is not yet established. On a blocked instruction the RNG returns
to that instruction's entry state; completed instructions retain their draws.
Workspace output is diagnostic and is not a resumable VM snapshot.

RNG-enabled execution stops before every shot request, including deterministic aim
modes, because the missing allocation/callback world could consume additional draws.
This mode assumes no external actor advances the shared stream during the isolated
slice. The default no-RNG mode and its all-entry matrix remain unchanged. The source
oracle preserves the original random selector blocks and assignment bodies, with
narrow local-storage adapters, for 720896 additional bitwise value/seed comparisons.

## Launch kinematics and numerical profile

`random::Rng` owns an explicit 16-bit seed, unsigned draw counter, and optional
saved seed. Snapshot copies preserve all state; restoring the saved seed does not
roll back the counter. Integer zero ranges consume no draws, while floating zero
ranges consume two. A 32-bit draw takes the first 16-bit result as its high word.
The oracle compares 524288 operations over all 65536 initial seeds against the
pinned reconstructed bodies compiled locally. This establishes the native profile,
not original executable evaluation order. No component invents an entry seed or
assumes that independent entities own independent RNG streams.

`kinematics::launch` implements all nine aim modes with caller-supplied random samples.
It preserves fan order, the count2 (not count2-1) speed denominator, capped angle
normalization, and the distinction between raw velocity angle and stored normalized
angle. It allocates nothing and does not consume an implicit RNG. Its current velocity
profile follows `TH08_MODERN_PORT` float32 sinf/cosf, not the original x87 fsincos path.
The source oracle extracts the pinned launch switch and normalization function, then
compares all five output float fields bitwise and checks random draw counts.

This kernel runs before transform installation, pool allocation effects, spawn animation,
rank adjustment, suppression, and collision. It is not yet a complete bullet simulation.

## Lifecycle projections

`animation::certify_timing` proves completion time and one immutable sprite for a
restricted straight-line ANM script. It explicitly rejects unsupported instructions,
variable masks, and sprite replacement. Accepted visual-only writes cannot influence
these two observables. The template has already executed time zero before it is copied
into a new bullet. The certificate requires a unit-rate clock and no external interrupts.
The full resource audit currently certifies 42 scripts and rejects 1109 as unsupported.

`bullet::advance_direction` models relative, absolute and aimed changes. Missing target
angles block only a firing frame and leave state unchanged. Integer firing thresholds,
fractional deceleration age and reset/increment order follow the original `ZunTimer`;
the source oracle retains that actual timer rather than substituting an integer.
`bullet::advance` composes
one installed direction transform with spawn displacement, same-frame activation,
scripted freeze, cull delay, and offscreen lifetime. Its contract excludes cancellation,
later transform records, pool contention and player interaction. Source-driven sub40/41
fixtures explicitly establish those preconditions; see `MOTION_FIXTURES.md`.

`bullet::advance_acceleration` projects one installed deceleration, vector or polar
effect. It uses fractional timer age, preserves velocity on the expiry frame, and
ticks even when that frame clears the active flag. Vector acceleration updates angle
only outside the source's strict 0.0001 per-axis dead zone and leaves scalar speed
unchanged. Its installed vector includes the installation frame multiplier; each
update applies that update's multiplier again. Polar speed is signed, not clamped.
Concurrent effects must run in source order: deceleration, vector, polar, then turns.
Transform-program installation/scheduling and concurrent-state ownership remain
separate from this allocation-free, failure-atomic kernel.

`laser::advance` emits up to three ordered collision calls while updating offsets,
phase and lifetime. It preserves switch fallthrough, the source's ramp axis, graze
flags, and collision calls emitted before retirement. `timing::tick` retains the
original fractional-clock threshold and carry. The oracle uses the actual `ZunTimer`
definition and `Supervisor::TickTimer`, not an integer-clock substitution.

## Geometry and planning

Coordinates are local playfield coordinates. Box sizes store full dimensions;
player dimensions are half sizes. Contact is lethal. As in the reference source,
laser collision rotates only the player center, preserving the axis-aligned player
half sizes. Upstream code must resolve gates, invulnerability, cancellation, and lifecycle.
Laser dimensions retain their sign: terminal ramp roundoff can produce a negative
dimension that still participates in the source predicate. Broad-phase radii are
conservatively bounded without clamping narrow-phase dimensions. See `REGRESSIONS.md`.
Bullet dimensions and player half sizes must remain nonnegative.

`Snapshot` takes and owns hazard data. Caller mutation, temporary input destruction,
and snapshot copy/move cannot invalidate its index. A 24-by-28 grid of 16-pixel cells
uses CSR storage: prefix offsets and one contiguous reference array. Large hazards
have a separate list. Dense cells and out-of-playfield queries fall back to scanning.
Queries allocate no heap memory. Roundoff padding affects only the broad phase;
the final predicate is unchanged. The API accepts coordinates within plus/minus 1e6
and laser angles with absolute value at most 16.

`Model::frames[t]` is the lethal collision phase after movement action t, not an
arbitrary captured screen frame. A model must be a fixture or a caller-justified
candidate-independent future, with a matching epoch. Movement parameters are fixed
and actions use nine directions. Terminal bounds constrain the player center.
Actual character speeds must come from the selected SHT; current defaults serve fixtures.

Search is bounded by beam width and expansion count. Every successful path is replayed
using an unindexed scan. One contiguous node arena, reused candidate storage, a
deterministic min-heap, and a bounded open-addressing position table replace per-frame
tree allocation and full sorting. Explicit tie-breaks preserve the original reference
routes. The search is neither complete nor optimal. Position merging applies
only to this candidate-independent, fixed-movement model, not to worlds carrying
different RNG, damage, alignment, or lifecycle state.

## Performance sequence

Establish correctness comparisons, then reduce work through batching, shared structure,
and data layout. Apply SIMD or further specialization only to measured bottlenecks.
A fixed emitter slice can generate its schedule once and share it; repeated interpreter
costs shown in microbenchmarks need not be paid per candidate. The next substantial
task is connecting emission, bullet motion, and candidate dependencies correctly.
Microbenchmark time is not complete solving time.
