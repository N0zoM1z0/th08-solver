# Provenance and verification

## Pinned inputs

- DAT: 46,838,025 bytes; SHA-256 `9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb`.
- Reconstruction: [N0zoM1z0/th08](https://github.com/N0zoM1z0/th08), commit `a45e99fb1942714e6edded20847e32a654d56f97`.
- `src/Player.cpp`: `80c6829a41a30fcce47837edaa8da90bb11779130b5c443db842c7623745242c`.
- `src/Global.cpp`: `8df17616c935d684b6636619d4726889e68f7d2d7000e27c25aebc4bc460b74b`.
- `TH08_AllCase_20260911.zip`: `8913fffc96824c27b681ff1b1133a4385f7c7ca3e8e377fede17e552e535b199`.

Older reports also refer to a dirty worktree near `af72ca9...`; that evidence must
not be conflated with the pinned source above. Original preparation files remain
unchanged. Their code is not automatically promoted into a verified component.

## Comparison chain

| Behavior | Reference | Current check | Excluded |
|---|---|---|---|
| DAT decoding | PbgArchive, Lzss, FileSystem::Decrypt | All 317 decoded member hashes match the historical reproduction | Gameplay semantics |
| ECL structure | EclManager.hpp and historical audit | 24 files, 1449 subs, 36661 instructions, 2182 jumps, 431 spell starts | Complete VM execution |
| Boxes and lasers | Pinned Player.cpp function bodies | 300000 scenes, 600000 predicate comparisons, alive/death side-effect checks | Game loop and x87 equivalence |
| Spatial index | Unindexed hazard scan | Random scenes, cell boundaries, exact contact, snapshot ownership, invalid arguments | Formal proof for all float inputs |
| Wriggle scheduling | Actual DAT and historical event digests | sub40/41 ordered digests, 360 ticks, 160 commands, 840 requests; sub42 alignment variants | Successful allocation, bullet motion, complete spells |
| Planning | Explicit collision-restoration fixture | Legal actions, terminal region, unindexed replay, budget failure without a route | Reisen gameplay or complete search |

Reference functions are extracted only into the optional test build directory.
The reference checkout is not modified. Competing laser interpretations differ on
whether player extents also rotate; production function output supports center-only
rotation. Snapshot ownership tests distinguish a stable owned index from a borrowed
container that changes after construction, including copy/move cases.

Any decoded member hash mismatch, source-predicate mismatch, or replay collision
invalidates the corresponding model. An aggregate pass rate cannot hide such a failure.

## Attribution and distribution

The reconstruction uses MIT; see [Third-party notices](THIRD_PARTY_NOTICES.md).
Game DAT/EXE files, audiovisual assets, and extracted game content are not distributed.
Preparation artifacts are tracked at the user's request and retain their original
language and attribution. This project does not assign them a new license.

The Nitori workflow informed input pinning, competing mechanism checks, and independent
comparisons. Its commit trailer records workflow assistance, not human authorship.
