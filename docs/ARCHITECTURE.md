# Architecture and contracts

## Data flow

```text
User DAT -> Archive decode -> Ecl instruction arena -> predecoded Program
                                      |                      |
                              complete source index    emission/transform events

Validated hazard phases -> owned Snapshot -> bounded beam search -> reference replay
```

These pipelines are not yet connected by a complete world executor.
An emission request cannot be treated directly as an active hazard.

## Resources and programs

`Archive` owns the DAT bytes read once. LZSS uses a fixed 8 KiB dictionary, a bit
reservoir, and a single output allocation. Block decryption reuses a fixed scratch
array. End padding is allowed only after the complete declared output has been
produced, to decode the terminator. Names, section ranges, output sizes,
uninitialized dictionary reads, ECL instruction sizes, and jump boundaries are checked.

`Ecl` stores one contiguous instruction arena; subprograms store ranges into it.
Operands remain in the caller-owned decoded resource and use explicit little-endian
reads rather than host structure layouts. The current parser validates timeline
directory entries but does not implement timeline execution, complete opcode payload
schemas, or ANM/SHT/STD semantics. These capabilities remain in the historical
experiments and must be migrated with comparisons.

`Program` predecodes one subprogram into fixed-size records and resolves supported
jumps to array indices. `Workspace` reuses register validity flags, emission records,
and transform-write records across executions. Each emission references the number
of preceding transform writes, avoiding a full transform-table copy per emission.

Restricted execution supports NOP, return, secondary-clock waits, unconditional and
decrement jumps, integer/float assignment, float add/subtract/multiply, transform
descriptors, and shot requests. Uninitialized registers cannot be read. Only explicitly
initialized scalar locals and EXTRA_I0-3 can be written in the isolated context.
Other behavior returns a status and the first stopping instruction.

Shot records do not execute random spread, aimed direction, distance suppression,
rank adjustment, deferred dispatch, or pool allocation. Reusing a resulting model
still requires dependency evidence from the world layer.

## Geometry and planning

Coordinates are local playfield coordinates. Box sizes store full dimensions;
player dimensions are half sizes. Contact is lethal. As in the reference source,
laser collision rotates only the player center, preserving the axis-aligned player
half sizes. Upstream code must resolve gates, invulnerability, cancellation, and lifecycle.

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
using an unindexed scan. The current proposal implementation still sorts candidates
and uses a position set; it is neither complete nor optimal. Position merging applies
only to this candidate-independent, fixed-movement model, not to worlds carrying
different RNG, damage, alignment, or lifecycle state.

## Performance sequence

Establish correctness comparisons, then reduce work through batching, shared structure,
and data layout. Apply SIMD or further specialization only to measured bottlenecks.
A fixed emitter slice can generate its schedule once and share it; repeated interpreter
costs shown in microbenchmarks need not be paid per candidate. The next substantial
task is connecting emission, bullet motion, and candidate dependencies correctly.
Microbenchmark time is not complete solving time.
