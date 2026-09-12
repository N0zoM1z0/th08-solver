# ECL scalar ownership and compact call frames

## Evidence and scope

The preserved reconstruction checkout is commit
`a45e99fb1942714e6edded20847e32a654d56f97`. Relevant pinned files:

| File | SHA-256 |
|---|---|
| `src/EclManager.hpp` | `1df7f926d46d24ad9e303a1a5e9b82cf9674ebec63d9ac5ff770c9c3957812e4` |
| `src/EclDependencies.cpp` | `019f9cd6abdb73223d3d41cc8a6317641e6fe6bfbd7777d126a4bace3e14e2e4` |
| `src/EnemyManager.cpp` | `e8febe94a833472b33f732e83ee39ee48fdc5097c5d69ff094fd1f1bb8629a7d` |
| `src/EnemyTimeline.cpp` | `920ee34725aa6aad9f113d43454731acadab456abddac73256b2ba9a29e8e94b` |

`EnemyEclContext` declares a contiguous 0x78-byte scalar region. `SpawnEnemy2`
copies that region, not entity variables or global call parameters. Ordinary
`CallSubOnEnemy`/`PopEclContext` save and restore the context; entity and global
storage remains outside it. The source saves additional context members too:
timers, instruction pointer, callbacks, interpolation and child-context metadata.
The native restricted scheduler does **not** yet implement all those members.

## Scalar mapping

Slots below are native selector offsets (ECL selector minus 10000), in source
memory order rather than sorted selector order:

| Source field | Slots | Context snapshot | Template initialization |
|---|---|---|---|
| `intVariables[8]` | 0..7 | Yes | Zero |
| `floatVariables[8]` | 16..23 | Yes | Zero |
| `extraIntVariables[4]` | 36..39 | Yes | Zero |
| `extraFloatVariables[2]` | 94..95 | Yes | Zero |
| `callParameterInts[4]` | 53..56 | Yes | Zero |
| `callParameterFloats[4]` | 57..60 | Yes | Zero |
| Entity integer/float arrays | 8..15, 24..31 | No | Zero |
| Shared call integer/float arrays | 61..68 | No | Unchanged |
| Computed and external selectors | Other slots | No | Unchanged |

`ContextScalars` owns thirty native values and thirty validity flags. It is not
a binary ABI replica. Capture/restore performs no operand resolution or RNG draw;
it preserves unknown values as unknown and preserves signed zero. Restoring into
another actor's scalar projection cannot overwrite that actor's entity variables
or shared parameters. Shared storage still needs synchronization by the world
owner; this helper does not make per-workspace copies globally coherent.

`initialize_spawn_scalars` applies only the 46 scalar zeros proven by the spawn
template's `memset`. Call it after `begin`, which invalidates storage. It does not
initialize an entire actor, allocate a pool slot, execute ECL, or acknowledge any
timeline or world effect. Do not apply it to inherited contexts: capture and restore
the supplied context instead. `CallEclSub` is not a general scalar reset operation.

## Verification and memory

The dedicated `context_storage` CTest checks all 101 ownership slots, source field
order, unknown validity, signed zero, self-restore, template zeros, and two nested
calls spanning a synthetic external-effect boundary. Existing call/wait/fork tests
and the native DAT matrix exercise the changed call-frame implementation.

On the local GCC 12.2 Linux x86_64 profile, a call frame decreases from 944 to 304
bytes, and `Workspace` from 15120 to 5520 bytes. Fifteen frames save 9600 bytes per
workspace. If an eventual world provisions 480 such workspaces, the fixed-storage
saving is 4608000 bytes; this is a size calculation, not an implemented pool or a
world-throughput benchmark. Calls now copy thirty scalar slots and returns visit
only those thirty, without scanning all 101 slots.

The remaining spawn integration must preserve: template copy, immediate ECL and
its complete callback/movement/shot/ANM tail, then display color/drop/score/life
bookkeeping. `SpawnEnemy2` additionally copies context scalars before ECL and may
overwrite life again afterward. An ECL `frame_complete` result alone must not
complete that spawn transaction.
