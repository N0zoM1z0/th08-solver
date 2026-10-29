# October 28 local storage maintenance

## Current retention and authority

The owner's later instruction retains only compressed versions of earlier videos
and permits deleting other video artifacts. This explicitly supersedes the
first pass's exact-MKV/PCM recovery requirement. Current local recordings are
**29 complete H.264 MP4 files**, totaling 16382582293 bytes. 28 include AAC;
wine 29 is the pre-existing seven-second silent recording and remains silent.
No new video transcoding or audio removal from retained files occurred.

The maintenance operator is the repository assistant, using
`gensokyo-skills:sakuya-checkpointed-execution`. The explicit deletion authority
is the owner's local cleanup and subsequent compressed-video-only instruction.
Private plans, inventories, source identities, operation logs, guards and proofs
are under `reports/local/storage-cleanup-20261028-v2/`. No new scoring task,
retail gameplay, seed acquisition or remote-host cleanup starts in this pass.

| Scope | Completed action | Recovery / unchanged evidence |
| --- | --- | --- |
| Video/media derivatives | Remove 111 files: redundant previews/encodes, one zero-byte MP4, raw audio, earlier lossless media archives/descriptors and disposable pacing fixtures | Keep 29 full compressed recordings; original raw MKV/PCM bytes are no longer exactly recoverable |
| Inactive owned Wine scratch prefixes | Archive 45 directories,27666527886 bytes, into a standalone 221725288-byte tar.zst | Full materialized restore verifies 44915 entries before unlink; preserve 10 existing prefix environments, including valid seeds, current replay environment and two in-use historical prefixes |
| Private compile intermediates | Archive 656 reports/local .o files,98780186 bytes, into 10265362 bytes | Full materialized restore verifies all object hashes before removal; source files, producer ELFs, libraries and current build directories are unchanged |
| Replay/report provenance | Recheck all 37 selected replay hashes and 272 historical payload hashes | No replay, action tape, seed receipt, result, failure log or immutable source/binary manifest is removed |

The removed media include 12 redundant nonzero MP4 variants and one zero-byte
stub. For wine 31 and 34, the existing full-duration compact encodes replace the
larger local versions. wine 30 retains its full audio/video recording; wine 33
retains the full practice recording rather than its 20-second preview. The
private `compressed-media-index.json` records every retained filename, exact
hash, audio scope and removed variant. A preview/re-encode is not claimed to be
byte- or frame-identical to the selected full recording. Retired adjacent restore
descriptors are preserved centrally as audit metadata, not as working restore
instructions. The README animation originals and unrelated user files remain local.

## Ordered execution and gates

1. Capture published-evidence hashes and explicit candidate inventories. Confirm
   no solver/test/recording job is using candidates; exclude every prefix present
   in a process's WINEPREFIX. The owner-authorized scope never includes unrelated
   processes, the old remote checkout, preparations or original game assets.
2. Probe all 42 original MP4 paths. One is a pre-existing zero-byte stub; retain
   its valid repaired counterpart. Select complete existing compressed encodes
   with audio where the episode had audio; record full hashes before deletion.
3. Before irreversible media removal, persist the exact deletion plan and the
   owner's updated retention authority. Copy old restore descriptors centrally,
   recheck retained/candidate identities, then unlink only listed media. Verify
   retained hashes and published evidence after removal. No exact-media rollback
   is claimed: the owner's accepted replacement is the compressed recording.
4. Inventory inactive Wine trees without following symlinks. Interleave matching
   relative paths in tar so duplicate DLL bytes compress together. The archive
   uses explicit owned paths, one Zstandard thread and a 128 MiB long window; it is
   self-contained and does not depend on a live prefix or a retained MP4.
5. Materialize the entire Wine archive and compare file bytes, modes, owners,
   mtimes and symlink targets with the inventory. Persist the matching tree and
   archive hashes. Recheck original identities and live-prefix exclusion before
   each directory unlink; stop on any mismatch. Remove restored verification
   copies only after the retained archive/proof and unlink receipts exist.
6. Independently archive and fully restore the private object intermediates,
   compare every object hash, then remove only those listed .o files. These are
   maintenance archives, not permission to relink incompatible historical ABIs.
7. Recheck the selected replays, historical payloads, kept media/archives and
   active seed environments. Record exact file-content accounting, owner exits
   and documentation checks. No transition remains half-open on completion.

An abort leaves untouched originals or already-verified archives with per-step
receipts. Source changes, a new live consumer, failed restoration/hash, unknown
state, resource guard or ineffective compression stop the affected operation.
Neither a checksum nor an archive is described as protection against disk loss.
Inode/ctime/hardlink identity is not restored; no new retail replay is certified.

## Restore commands for retained non-media archives

Run from the repository root. The old prefix archive expands only the archived
relative scratch paths; current seed/replay environments were excluded. Restore
only in a clean, inactive destination and verify the recorded hashes before use.

```sh
tar -I 'zstd -d --long=27' -xpf \
  reports/local/storage-cleanup-20261028-v2/old-wine-prefixes.tar.zst

tar -I zstd -xpf \
  reports/local/storage-cleanup-20261028-v2/private-objects.tar.zst
```

Prefix archive SHA-256:
`324b8480c05b4d1ea064e8409139d6f882f17dbd7630219aae702f203a616b41`.
Its original/restored tree digest is
`6b5d07426360868008ce614fef838f91da6159e9a1913a36007c2e4fe0bd227c`.
Object archive SHA-256:
`7cf9b778a584d21179805166d6e829b5061493db07f7691dd1d88cfdbe3e0ba4`.
These exact compressed bytes are now recovery dependencies. Keep their inventories
and proofs. Restore historical intermediates before a historical relink, and
recompile all affected translation units when headers/libraries change.
The MKV/VCDIFF and PCM commands in the historical section below are **retired**;
their archives have been deleted under the later owner instruction.

## Measurements, deviations and remaining scope

| Owner | Guarded wall seconds | Peak owned RSS bytes | Minimum host MemAvailable bytes | Outcome |
| --- | ---: | ---: | ---: | --- |
| Wine-prefix archive/restore | 583.560 | 339034112 | 29904691200 | 45 full trees verified and archived |
| Compressed-media retention | 102.150 | 51486720 | 42477838336 | 29 retained files and all published evidence unchanged |
| Object archive/restore | 8.125 | 131674112 | 43255906304 | 656 restored object hashes agree |

At most two maintenance owners overlap, within the four-local-owner ceiling.
All retain 4 GiB process/tree and 12 GiB host admission/8 GiB abort guards; no resource
abort occurs. Operations use low priority; compression is single-threaded.
These concurrent wall measurements are not isolated CPU benchmarks.

Media removal accounts for 7594281189 bytes, with 40083
bytes of retired descriptors retained as metadata. Wine-prefix archiving saves
27444802598 bytes and object archiving saves 88514824.
The net retained-payload reduction is **35127558528 bytes (32.72 GiB)**,
before other small new inventories/logs/scripts. This is file-content accounting;
unrelated filesystem free-space fluctuations are not counted as project savings.

Two preflight failures are preserved. The first media scan stops at the existing
zero-byte MP4 before deletion; a complete scan then identifies its repaired
replacement. A scoped attempt to stop old qualified-replay v2 Wine helpers
returns nonzero; that first helper did not persist stderr. No successful shutdown
is inferred. Both v2/v3 in-use prefixes and their services are excluded and remain
untouched by archive/removal. No game process or unrelated service is killed.

Final maintenance audit 1643 passes documentation CTest, all 37 selected and 272
historical hashes, retained-media identities, both archive hashes and restoration
proofs, all 177 recent task closures and the byte-exact 1642-row published results
prefix. It costs 2.830 s guarded wall; no scoring case is started.

The earlier pass below records the conditions and measurements at that time.
Its 34-replay/42-MP4 counts precede the later maintenance pass's 37 selected
replays/29 retained recordings. The current replay index is maintained separately
in [Native evidence](../../reports/native/README.md). Former media recovery
promises are superseded.

## Earlier exact-media archive pass (historical)

The first pass preserved exact raw-media bytes, before the owner explicitly
replaced that policy with compressed recordings only. Its VCDIFF/PCM archives and
adjacent recovery descriptors have since been retired. Those restore commands
are intentionally omitted here because their dependencies no longer exist.
The complete original instructions and receipts are retained in the
[fixed historical snapshot](https://github.com/N0zoM1z0/th08-solver/blob/cf277285d64429b4c3720fb5c30eb0f360e7229c/docs/experiments/2026-10-28-storage-cleanup.md#earlier-exact-media-archive-pass-historical)
and local `storage-cleanup-20261028-v1/` metadata.

| Historical measurement | Recorded value / scope |
| --- | --- |
| Initial inventory | 68,712 regular files; 50,455,898,706 logical bytes; 50,643,976,192 uniquely allocated bytes |
| Evidence gate | 34 selected replays and 272 historical payloads unchanged at that pass |
| Exact-media operation | 29 MKVs archived and fully decoded before removal; 42 MP4s then unchanged |
| Audio pass | 27 WAVs examined; seven compressed after full byte/hash verification; 20 initially unchanged |
| Original / archive bytes | 18,956,192,259 / 862,127,700 |
| Media-only reduction | 18,094,064,559 bytes (16.85 GiB) |
| Encode / streaming restore wall | 507.289 s / 67.577 s; excludes hashing, materialized probes, rejected compression and failed setup |
| Resource bounds | Two MKV workers and one audio worker, within the then four-owner ceiling; 4 GiB address space, 900 s child bound, host 12/8 GiB guards |
| Observed peak / minimum host available | 172,564,480 / 11,842,478,080 bytes; no resource abort |

The misplaced Zstandard `-o` option failed after 0.220 s with the source intact;
its error and cleanup remain recorded. Representative wine 57 and wine 34 exact
restores passed at that time. Those proofs establish the old operation's result,
not present-day recoverability after the later authorized archive deletion.
The retained recovery dependencies are only the two non-media archives listed
above, their inventories and their materialized restore proofs.
