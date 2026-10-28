# October28 local storage maintenance

## Current retention and authority

The owner's later instruction retains only compressed versions of earlier videos
and permits deleting other video artifacts. This explicitly supersedes the
first pass's exact-MKV/PCM recovery requirement. Current local recordings are
**29 complete H.264 MP4 files**, totaling16382582293 bytes.28 include AAC;
wine29 is the pre-existing seven-second silent recording and remains silent.
No new video transcoding or audio removal from retained files occurred.

The maintenance operator is the repository assistant, using
`gensokyo-skills:sakuya-checkpointed-execution`. The explicit deletion authority
is the owner's local cleanup and subsequent compressed-video-only instruction.
Private plans, inventories, source identities, operation logs, guards and proofs
are under `reports/local/storage-cleanup-20261028-v2/`. No new scoring task,
retail gameplay, seed acquisition or remote-host cleanup starts in this pass.

| Scope | Completed action | Recovery / unchanged evidence |
| --- | --- | --- |
| Video/media derivatives | Remove111 files: redundant previews/encodes, one zero-byte MP4, raw audio, earlier lossless media archives/descriptors and disposable pacing fixtures | Keep29 full compressed recordings; original raw MKV/PCM bytes are no longer exactly recoverable |
| Inactive owned Wine scratch prefixes | Archive45 directories,27666527886 bytes, into a standalone221725288-byte tar.zst | Full materialized restore verifies44915 entries before unlink; preserve10 existing prefix environments, including valid seeds, current replay environment and two in-use historical prefixes |
| Private compile intermediates | Archive656 reports/local .o files,98780186 bytes, into10265362 bytes | Full materialized restore verifies all object hashes before removal; source files, producer ELFs, libraries and current build directories are unchanged |
| Replay/report provenance | Recheck all37 selected replay hashes and272 historical payload hashes | No replay, action tape, seed receipt, result, failure log or immutable source/binary manifest is removed |

The removed media include12 redundant nonzero MP4 variants and one zero-byte
stub. For wine31 and34, the existing full-duration compact encodes replace the
larger local versions. wine30 retains its full audio/video recording; wine33
retains the full practice recording rather than its20-second preview. The
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
2. Probe all42 original MP4 paths. One is a pre-existing zero-byte stub; retain
   its valid repaired counterpart. Select complete existing compressed encodes
   with audio where the episode had audio; record full hashes before deletion.
3. Before irreversible media removal, persist the exact deletion plan and the
   owner's updated retention authority. Copy old restore descriptors centrally,
   recheck retained/candidate identities, then unlink only listed media. Verify
   retained hashes and published evidence after removal. No exact-media rollback
   is claimed: the owner's accepted replacement is the compressed recording.
4. Inventory inactive Wine trees without following symlinks. Interleave matching
   relative paths in tar so duplicate DLL bytes compress together. The archive
   uses explicit owned paths, one Zstandard thread and a128MiB long window; it is
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
| Wine-prefix archive/restore |583.560 |339034112 |29904691200 |45 full trees verified and archived |
| Compressed-media retention |102.150 |51486720 |42477838336 |29 retained files and all published evidence unchanged |
| Object archive/restore |8.125 |131674112 |43255906304 |656 restored object hashes agree |

At most two maintenance owners overlap, within the four-local-owner ceiling.
All retain4GiB process/tree and12GiB host admission/8GiB abort guards; no resource
abort occurs. Operations use low priority; compression is single-threaded.
These concurrent wall measurements are not isolated CPU benchmarks.

Media removal accounts for7594281189 bytes, with40083
bytes of retired descriptors retained as metadata. Wine-prefix archiving saves
27444802598 bytes and object archiving saves88514824.
The net retained-payload reduction is **35127558528 bytes (32.72GiB)**,
before other small new inventories/logs/scripts. This is file-content accounting;
unrelated filesystem free-space fluctuations are not counted as project savings.

Two preflight failures are preserved. The first media scan stops at the existing
zero-byte MP4 before deletion; a complete scan then identifies its repaired
replacement. A scoped attempt to stop old qualified-replay v2 Wine helpers
returns nonzero; that first helper did not persist stderr. No successful shutdown
is inferred. Both v2/v3 in-use prefixes and their services are excluded and remain
untouched by archive/removal. No game process or unrelated service is killed.

Final maintenance audit1643 passes documentation CTest, all37 selected and272
historical hashes, retained-media identities, both archive hashes and restoration
proofs, all177 recent task closures and the byte-exact1642-row published results
prefix. It costs2.830s guarded wall; no scoring case is started.

The earlier pass below records the conditions and measurements at that time.
Its34-replay/42-MP4 preservation counts are historical, not the current37 selected
replays/29 retained recordings. Its former media recovery promises are superseded.

## Earlier exact-media archive pass (historical)

The owner requested cleanup of `reports/local/` while Native score exploration
continues. The initial inventory records 68,712 regular files, 50,455,898,706
logical bytes and 50,643,976,192 uniquely allocated bytes. Old recording MKVs
account for about17.37GiB; final MP4s account for19.11GiB. This is storage
maintenance, not a scoring trial or a change to campaign budgets.

The operator is the repository assistant, under the owner's explicit cleanup
instruction. The checkpointed execution method is
`gensokyo-skills:sakuya-checkpointed-execution`. Detailed private inventory,
tool identities and per-file receipts live in
`reports/local/storage-cleanup-20261028-v1/`.

### State and scope

Keep every final MP4, selected replay, action tape, seed receipt, result, failure
log and source/binary manifest. Keep the original README animation materials.
Do not touch user images, other directories, the remote host or active files.
The only initial candidates are completed recordings named
`reports/local/shadow-runtime-v1/wine-N/capture.mkv`, with a retained final MP4
in the same directory. No consumers need the raw MKV for ongoing Native scoring.

Replace a raw MKV with a lossless VCDIFF archive against its retained MP4 only
when the archive is smaller and a complete decode reproduces the original
SHA-256. This preserves the exact MKV container, timestamps and original bytes;
a remux or lossy video transcode would not provide that recovery contract.
Abort on a changed source/base, symlink, insufficient available memory, failed
child process, failed hash, missing evidence or unknown intermediate state.
There is no deadline requiring a failed gate to be bypassed.

### Ordered checkpoints and gates

1. Inventory files and confirm that recording/solver/test owners have finished.
   Record identities of all34 selected progress replays and272 historical
   payloads. Keep the inventory before changing storage.
2. Download and extract Debian's `xdelta3` package into the ignored cleanup
   directory, without installing a system package or adding a solver dependency.
   Record package and executable hashes. A small recording is the first probe;
   a large complete-story recording checks the representative recovery path.
3. Encode each archive at low priority with bounded memory. Decode it completely,
   compare the restored byte count and SHA-256 with the original, then recheck
   source/base identity. The first probe also writes an actual restored file
   and compares it byte for byte. A failed or ineffective archive leaves the
   original intact.
4. The automatic unlink gate requires the owner's existing cleanup authority,
   a durable per-file receipt, a verified archive and an unchanged retained
   base. This is the last safe abort point. Remove only that original MKV,
   immediately record completion and continue within its bounded worker. A partial run
   leaves already verified archives recoverable and remaining originals intact.
5. Recheck all published evidence hashes, retained bases and archive identities;
   measure exact reclaimed bytes. Run the documentation check before publishing
   the journal. Retain the private receipts and restore instructions.

Each child operation has a bounded observation window; an overrun stops the
current file without unlinking its source. The assistant observes progress
throughout. No other storage policy is inferred from this cleanup.

### Recovery and limits

Encoding is reversible by deleting its temporary output. Unlinking a verified
source is reversible by decoding its archive with the recorded base:

```sh
reports/local/storage-cleanup-20261028-v1/tools/root/usr/bin/xdelta3 \
  -d -D -R -s retained-final.mp4 capture.mkv.vcdiff capture.mkv
```

Use the actual paths and SHA-256 values in the private receipts; verify the
restored hash. Recovery requires free space for the original and time to decode
and read it. The restore tests measure those costs. This does not recreate inode
identity or filesystem metadata, and is not an independent backup against disk
loss. **The retained MP4 is now a required archive dependency**: do not delete or
rewrite it without first restoring the MKV or rebasing and verifying its archive.

### Execution and final verification

Completed on October28. All29 candidate MKVs were archived and their originals
removed only after exact full restoration passed. All42 MP4 files retain their
original file identity, size and modification time; all29 archive bases also
pass fresh SHA-256 checks. The final verification rechecks all34 selected replay
hashes and272 historical payload hashes. No selected replay, action tape, seed
receipt, result or source/binary identity was removed.

The wine57 probe reduces149,453,320 bytes to104,447 bytes, with0.840s encoding
and0.454s streaming restore verification. A materialized restoration also passes
`cmp`. The representative wine34 complete-story recording reduces2,563,399,567
bytes to564,576,577 bytes:400.862s encoding and13.146s streaming restore, plus
a materialized restore and `cmp`. Its recovered original SHA-256 is
`15e565acc229a6024de13ac712fcde1921158b22dcf53821df259b5ca06888c4`.
Its retained MP4 SHA-256 is
`ac7fe1f8abdeb0ecee88d5b7e7b2e26cfa76ae0ee20b56076263dbd9b77fbe03`.
Different timings and archive ratios are observations, not a uniform speedup.

The subsequent PCM pass examines27 old `wine-N/audio.wav` files. Seven gain at
least5% from lossless Zstandard compression and pass full decoded byte-count/
SHA-256 checks before removal;20 remain unchanged because compression is not
useful enough. The first retained audio archive also passes a materialized
restore and `cmp`. Its recovery command is independent of the MP4:

```sh
zstd -d -o audio.wav -- audio.wav.zst
```

An initial Zstandard invocation placed `-o` after `--`. It exited1 after0.220s,
leaving the original intact and an incidental default output. The failed
receipt/log were preserved; the output was isolated, the option ordering fixed
and the source/replacement checked before that duplicate was removed. This was
a preparation error, not a failed lossless hash or an unverified source deletion.

Media accounting:18,956,192,259 original bytes were replaced with862,127,700
archive bytes, reclaiming **18,094,064,559 bytes (16.85GiB)**. The initial regular
file inventory was50,455,898,706 bytes (46.99GiB); the corresponding final
inventory is32,381,937,574 bytes (30.16GiB), before writing its18,556,430-byte
inventory file. These tree totals include new private experiment/cleanup
metadata, so their difference is not the media-only reclaim count. No unrelated
filesystem free-space fluctuation is reported as reclaimed project storage.

Two low-priority MKV workers and one audio worker ran within the existing four
local-owner ceiling. Child address space is bounded to4GiB, each operation to900s,
with host12GiB admission/8GiB abort checks. Largest observed single child RSS is
172,564,480 bytes; minimum observed host available memory is11,842,478,080 bytes,
above the abort floor. No resource abort occurred. Successful retained-archive
encode/stream-restore operations sum507.289s/67.577s; this excludes hashing,
materialized probes, rejected audio compression and the failed setup, whose
individual receipts remain available. It is not an end-to-end CPU benchmark.

Every removed source has an adjacent `.restore.json` file with its original
hash, archive hash, dependencies, exact command and central receipt path. Final
verification completes at08:12:46 UTC. All cleanup workers have exited. The
recorded exact-restore dependency is the remaining operational constraint;
there is no unverified or half-removed source awaiting recovery.
