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
jumps to array indices. Eight common operand words stay inline; a contiguous owned
payload arena retains every byte of longer world instructions. Payload access is
bounds-checked, survives decoded-resource destruction and program copy/move, and
performs no runtime allocation. The DAT check compares all 38110 compiled payloads,
including 1449 terminal records, with the decoded input. `Module` owns all predecoded subprograms in a resource, reused
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
same stream; context return never restores it. This isolated scalar domain
permits at most one RNG-consuming expression: two random operands, or a
random sign combined with a random operand, stop as unsupported because expression
evaluation order is not yet established. On a blocked instruction the RNG returns
to that instruction's entry state; completed instructions retain their draws.
Hard-error output remains diagnostic; execution cannot resume a partially failed
instruction. Restore an earlier caller-owned checkpoint to retry with new context.

`Execution` preserves the active program, PC, call depth, integer local/secondary
clocks and cumulative instruction budget. `advance` runs to one unit-rate frame
boundary or an explicit world instruction. Copy it together with `Workspace` and
the caller-owned RNG to fork; immutable programs must outlive those copies.
`run` remains a fresh, bounded convenience wrapper with identical legacy results.

In `yield_to_world` mode, shot/transform requests and unimplemented world opcodes
stop before reading operands or drawing RNG. Repeated advances preserve the pending
instruction. A world handler must implement it, publish affected computed fields,
and acknowledge its execution-count token exactly once; only then can scalar
execution continue in the same frame. Unknown effects must never be acknowledged
as NOPs. This boundary is not itself a world handler. Opcode 1 termination is
distinguished from normal root return. Tests cover waits inside calls, independent
forks, same-frame resumption and world-side RNG draws between scalar instructions.
Fractional ECL clocks, dynamic difficulty masks, child contexts, callbacks and the
enemy frame tail are still outside this scheduler's unit-rate contract.

RNG-enabled bounded request-recording execution stops before every shot request, including deterministic aim
modes, because the missing allocation/callback world could consume additional draws.
This mode assumes no external actor advances the shared stream during the isolated
slice. The default no-RNG mode and its all-entry matrix remain unchanged. The source
oracle preserves the original random selector blocks and assignment bodies, with
narrow local-storage adapters, for 720896 additional bitwise value/seed comparisons.

`decode_operands` exposes the same typed selectors to world handlers. Each field
declares its byte offset, signed16/signed32/float32 type and independent flag index;
raw fields ignore flags. Up to sixteen fields use fixed scratch storage, including
checked access to complete payloads beyond the eight inline words. Output and RNG
remain unchanged on failure; decoding alone neither mutates nor acknowledges a world
effect. The default rejects multiple random expressions. `source_ordered_fields`
is valid only when separate source statements establish that order, not when C++
leaves operand order unspecified.

`world::apply_motion_effect` handles pending ECL instructions 63..76 and 178.
Motion, scalar storage, RNG and the execution acknowledgement commit together only
after every operation succeeds. Missing player/RNG/register input, invalid state,
and unsupported evaluation order leave the instruction pending and all inputs
unchanged. Other unhandled effects remain explicit `not_handled`.
This rollback is the native interface contract, not a claim that the source engine
rolls back failed instructions.

The handler reads operands at their source statement boundaries. Later operands
can observe an angle or interpolation field written earlier in the same instruction.
Finite polar setup rereads speed for x and y, and rereads duration at its original
sites; caching those values would change RNG consumption and state-dependent reads.
Two random factors in the same product remain unsupported. Aimed opcodes use local
position, while player-angle selectors use world position; coincident x/y returns
pi/2, including signed zero. `publish_motion` uses the caller's phase-correct world
position, preserves unrelated registers and invalidates player-derived slots when
no player is supplied. Applying an effect is not velocity update or displacement.
The owning world still schedules actors, child contexts, callbacks, shots and ANM.

Random movement (67/178) shares the caller's RNG and depends on the current player,
not a cached emitter trajectory. Opcode 67 applies independent left/right/top/bottom
corrections using strict source margins, even when clamping is disabled; its positive
right-wall branch uses the previous movement angle and does not normalize afterward.
Opcode 178 chooses the source's wrapped horizontal bias, preserving the original
float operations and distance tie-breaking. Its zero-roll branch never reads the
player. Both timed random helpers omit the ordinary polar helper's mirror operation
and repeat speed/duration reads; a missing later input rolls back all provisional draws.

### Timeline control and world handoffs

`timeline::Program` owns predecoded instructions and complete payload bytes. All
32 DAT timelines and 2003 nonterminal payloads are compared against the input;
program copies remain valid after decoded resource destruction. `State` owns the
exact fractional clock, PC and a monotonically increasing pending-effect token.
Copy it with `Context` to fork the four shared event slots and gate observations.

Difficulty uses any-bit intersection, not enemy ECL's containment test. Only exact
integer-time matches execute; stale instructions skip without reading context.
Boss/message/event waits decrement the source clock before its frame-tail tick.
The extra-step flag affects that decrement, not the tick. A negative-time sentinel
still ticks on repeated calls; `at_end` is an observation, not a frozen stop latch.

GUI boss presence, spawn suppression, message waits, boss activity and event slots
must be known when the selected source branch reads them. Short-circuiting preserves
unused unknowns. Event publication fills every negative slot; event waits consume
every match. Calls failing on context, state or budget roll back both state and
events. Selected unknown opcodes remain unsupported rather than becoming NOPs.

Spawning, messages, pending boss subroutines, power changes and retry menus yield
before operands or RNG are consumed. The world must perform the complete effect
before acknowledging its token; only then can the same source frame resume.
The 200000-frame source comparison records external calls but does not implement
their world effects. The real practice timeline fixture observes sub0/sub42/retry
boundaries and five synthetic boss-wait frames; its supplied boss observations do
not represent an actual defeated boss. The all-mask baseline supplies no world
observations, so all 160 attempts correctly stop with `REQUIRES_CONTEXT`.

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

### ANM execution

`animation::certify_timing` proves completion time and one immutable sprite for a
restricted straight-line ANM script. It explicitly rejects unsupported instructions,
variable masks, and sprite replacement. Accepted visual-only writes cannot influence
these two observables. The template has already executed time zero before it is copied
into a new bullet. The certificate requires a unit-rate clock and no external interrupts.
The full resource audit currently certifies 42 scripts and rejects 1109 as unsupported.

`animation::control` adds an allocation-free runtime projection for clocks, PC,
sprite identity, visibility, stop/interrupt state, typed variables and player-shot
hit-animation metadata. Immutable contiguous programs predecode jumps and
stable-sort interrupt labels: the first exact match wins, while
an unmatched interrupt uses the last default label. Runtime state owns all clocks
and the single interrupt-return slot. An initial call executes template time zero.
The zeroed wait timer and initialized main timer
retain their distinct source initialization states.

ANM executes instructions whose time is less than or equal to the current integer
clock, unlike ECL equality scheduling. Static completion keeps visibility; delete
and sentinel completion hide. Stop does not advance the PC. Wait compares integer
timer fields, while decrement/tick preserve fractions; the explicit extra-step flag
affects decrement but not the frame-tail tick. Missing interrupts clear the stop
flag and hold the clock for that call. Interrupt return restores the saved full
clock and PC without clearing the return slot, matching the source.

Four integer variables, four float variables and two integer counters are owned and
zero-initialized as in the source initialization. Masked reads support cross-type
conversion; float selectors truncate before dispatch. Writes stay strictly typed:
literal or unmapped destinations would modify bytecode and remain unsupported.
Arithmetic, trigonometry, decrement jumps and twelve comparisons preserve native
float32 behavior within the finite, defined-arithmetic domain. Overflow, invalid
conversions and division/domain errors stop explicitly. Opcode 83 stores its raw
hit-animation selector even when masked; it does not execute a player-hit effect.

The final optional argument to `control::advance` is `random::Rng*`. No stream is
invented when it is null: random instructions return `requires_context`. Failed calls
roll back the entire State and RNG, including earlier instructions in that call;
`Result.pc` still identifies the failing instruction. Successful prior calls retain
their draws. The caller may advance the same stream
for other world actors between ANM calls. Integer zero ranges consume no draws,
while float zero ranges still consume two.

Enumerated visual writes are projected out after typed operand checks; those fields
cannot feed the exposed scalar/control observables. Their rendering, interpolation
and effects on other consumers remain unimplemented. Unknown opcodes, sprite resource
loading, dimensions and external lifecycle gates remain explicit boundaries. The
source oracle retains unchanged control/scalar blocks, all four typed accessors,
actual RNG and ZunTimer: 48224 control frames and 228669 scalar calls match.

The 1151-script audit runs 600 unit-rate calls without interrupts: 340 complete this
projection, 800 remain bounded and 11 require RNG context. Separately, explicit
per-script seeds 0 and 65535 each yield 350 completions and 801 bounded prefixes;
these independent streams do not establish world draw order. The original 13 columns
of all 1065 previously supported rows are unchanged. The 42 timing certificates,
including frame-30000 endings, are checked separately rather than relabeled as
600-call completions.

### Enemy movement phases

`enemy::State` owns local position, offset, published world position, velocity,
interpolation/orbit fields, fractional clock, bounds and motion flags. None, polar,
interpolated and orbital modes preserve their different expiry and z behavior.
Resolved configuration helpers do not evaluate ECL operands or consume RNG.
Zero-duration interpolation is invalid; source-representable negative relative
durations expire on the first update. Easing is a three-bit source field, including
the unnamed value 7 that follows the linear default.

The source phase order is part of the API contract:

```text
all ECL contexts -> update_velocity -> shot/ANM phase -> integrate_position
```

Velocity update must not be fused with displacement: shots and ANM observe the
intervening state. Integration preserves pre/post movement clamping, mirrored x,
the prior-displacement sample and skip-movement behavior. Bounds use the source's
ordered branches even when inverted. Parent inheritance takes the parent's local
position; an unresolved existing parent blocks instead of becoming a zero offset.
ECL world-position refresh retains z, while the manager integration phase zeros
published world z. Both phases are allocation-free and failure-atomic.

The oracle compares 580000 configuration/update/integration phases with pinned
methods and manager code. The effect bridge separately compares 420868 instruction
effects, with twelve native rollback checks. These checks do not supply enemy creation,
pause/death/alignment gates, callbacks, shot/ANM scheduling or a complete world loop.

### Bullet and laser motion

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
This kernel remains allocation-free and failure-atomic.

`transform::Program` contains eighteen immutable records that particles may share;
`transform::State` owns the program cursor, enabled/active flags, scalar motion,
three acceleration slots, one shared direction slot, bounce state and wait/wrap clocks. `advance_program`
performs birth-time installation without ticking. `step` projects the fired phase
through transform updates, before displacement, culling, ANM or collision.

The active-effect gate precedes the enabled-kind mask test. Cull-delay and sound
records continue immediately, while at most one timed effect is installed per call.
A zero-kind record halts the table. Relative, absolute and aimed flags run in that
order against the same direction state, potentially advancing/resetting it several
times per frame. WAIT and WRAP use the original decrement semantics and an explicit extra-timer
step flag; the acceleration/direction clocks ignore that flag, as the source does.

The bounded result records up to 22 ordered sound requests: eighteen table records,
three overlapping direction firings and one bounce. Failed projection leaves the state unchanged
and exposes no sound events. Despawn is a phase-transition request, not immediate
deallocation; a later call requires the still-unimplemented despawn ANM/world layer.
Sprite replacement, child patterns, external EX mutation and other
unverified record kinds remain unsupported. Unknown active flags also stop execution.
The source oracle retains `AdvanceTransformProgram`, its payload layouts and the
fired transform-dispatch block, comparing 120748 steps including simultaneous effects.

Bounce waits until the complete resource-derived sprite box leaves the playfield;
unresolved dimensions stop the projection. It preserves source reflection order,
does not reposition the particle, and even an excluded bottom exit resets speed and
consumes a bounce count. Horizontal and vertical wrapping each translate at most once,
keep exact far-edge coordinates unchanged, and share a countdown. Both axes may
therefore decrement that clock in one frame; wrapping precedes its expiry check.
The source comparison supplies displacement between isolated transform phases to
exercise repeated crossings, without treating that fixture as a complete lifecycle.

`laser::advance` emits up to three ordered collision calls while updating offsets,
phase and lifetime. It preserves switch fallthrough, the source's ramp axis, graze
flags, and collision calls emitted before retirement. `timing::tick` retains the
original fractional-clock threshold and carry. The oracle uses the actual `ZunTimer`
definition and `Supervisor::TickTimer`, not an integer-clock substitution.

## Geometry and planning

`bullet::Slots` is an occupancy/cursor index, not bullet storage. Twenty-four 64-bit
free masks replace the linear 1536-slot scan while preserving the first free slot
in circular cursor order. A free count makes full-pool failure constant-time; other
queries inspect at most 25 word fragments, allocate nothing, and leave state unchanged.
Snapshot copies own their masks and cursor. All non-UNUSED phases stay occupied until
the world actually deactivates them; animation or cancellation requests do not free them.

Reservation and cursor completion are separate. The source selects before launch RNG,
marks the bullet active before transform callbacks, then advances its cursor only after
those callbacks return. Nested children may therefore move the cursor before their
parent overwrites it with the position after its own slot. The caller must pair
reservations/completions in that source order and synchronize actual deactivations.
This index alone neither rolls back a failed initialization nor implements child
lifecycles, full-pool RNG effects, sprite storage, cancellation or generation handles.

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
