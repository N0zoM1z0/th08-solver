# October28 local storage cleanup

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

## State and scope

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

## Ordered checkpoints and gates

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

## Recovery and limits

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

## Execution and final verification

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
