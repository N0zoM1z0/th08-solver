# Experiment journal

This directory preserves the scoring campaign's hypotheses, parameters, evidence,
failures, costs and decisions. [Status](../STATUS.md) owns current records and the
stopped handoff; [Native evidence](../../reports/native/README.md) owns selected
replay identities. A dated journal entry describes the state at that time and
is not authorization to resume its proposed work.

| Record | Scope |
| --- | --- |
| [Stage/spell search, October 28–29](2026-10-28-stage-spell-search.md) | Source/human replay analysis, scene portfolios, complete futures, solo Sakuya, failures, clocks, rewards and owner handoffs |
| [Campaign lifecycle ledger](campaign-ledger.tsv) | Every allocation and original lifecycle note, including repeated notes for one ID; IDs are never reset or reused |
| [Per-task results and provenance](results.tsv) | One row per task with available entry, controls, argv, outcome, actual constraints, costs and source/input identities |
| [October 27 phase composition](2026-10-27-phase-composition.md) | Original phase maps, complete combinations, the 6.5 billion milestone and later Extra-first archives |
| [October 27 VPS investigation](2026-10-27-vps.md) | Historical remote protocol, source/human evidence, canaries, search portfolios and failures |
| [Storage maintenance](2026-10-28-storage-cleanup.md) | Retained recordings, verified Wine/object archives, byte accounting and restoration limits |

## Reading the tables

The TSV files have a header row. Tabs and newlines inside values are escaped;
entry, controls, argv, search costs and other structured fields use compact JSON.
Original identifiers, machine values and lifecycle notes retain their exact
spelling. An empty field means **unrecorded or unavailable**, never zero.

| Field group | Interpretation |
| --- | --- |
| Entry / controls / argv | Declared original episode, checkpoint, legal actions and measured invocation; requested parameters are not substituted for unavailable actual argv |
| Score / outcome / constraints | Actual misses, bombs and captures; a phase or road boundary is not a complete route |
| Costs / exits / resources | Available wall, Native/proposal/branch/I/O work, retries, peak tree RSS and host memory; concurrent wall is not an isolated CPU benchmark |
| Source / binary / data / receipt / input | Hash-bound producer provenance; a private overlay's declared source base is separate from the workspace head or old deployment bundle |
| Phase plan / metrics / stage results | Actual component strategy, world lineage and measured outcomes; no scalar comparison implies world equivalence |
| Replay retention / retail observation | Selected or pruned Native exports and separately scoped original-game observations, including parser failures and partial playback |

A successful wrapper is not a successful child. Read the original owner exit,
result and score audit. `complete` can be a valid below-target outcome; a failed,
incomplete or resource-aborted run cannot establish a score record. Fresh/index
agreement is not an independent physics oracle or retail validation.

## Coverage and historical evidence gaps

The original October 27 request authorizes detailed maintained experiment records,
including failures, setup errors, retries and missing historical measurements.
The published tables preserve these distinct uncertainty states:

- `historical_ledger_only`: 231 older allocations lack a structured owner receipt.
- `running_or_unresolved`: 29 older records lack the selected terminal receipt.
  This historical uncertainty is not a live queue.
- `unresolved_remote_identity_change`: 19 original remote closures remain unavailable
  after the host-key change. Neither remote success nor termination is inferred.
- `setup_failed_no_native_started`: argument/dispatcher failures before simulation;
  original receipts and costs remain charged, including the final card 74 retry.
- `resource_aborted`, `owner_cancelled`, `host_restart_interrupted_no_certificate`
  and `observation_replanned_no_certificate`: their partial observations are not
  complete gameplay certificates. Missing terminal measurements remain unavailable.

| Older group | IDs lacking allocation-specific closure evidence |
| --- | --- |
| Reused output names / continuation attempts | 176, 177, 193, 194, 211, 212, 213 |
| Early Native scoring/pacing | 219, 222, 226, 227, 228, 229 |
| Read-only income/pool/chain probes | 205, 209, 318, 319, 320, 321, 329, 330, 336, 353, 354 |
| Earlier seed acquisitions | 262, 263, 264, 265, 266 |

Do not rerun gameplay, reacquire a seed, reuse an ID or assign one reused-directory
result to several owners to fill a metadata gap. A backfill needs the original
allocation-specific receipt and explicit correction provenance. Later model, seed,
retail and maintenance evidence keeps its own scope.

## Export and publication contracts

The ignored workspace exporter is
`reports/local/vps-native-20261027/export-experiment-journal.cjs`. It joins the
original ledger to existing receipts; it does not regenerate historical payloads.
Receipt selection is deterministic: terminal/result completeness comes first,
previously published choices for duplicated owners remain fixed, and equally
complete imports do not erase the original remote receipt or retention evidence.
Reversing traversal must leave both public TSVs byte-identical.

Export only the explicitly supported public columns. The DAT hash must be bound
by the immutable worker bundle or an actual local DAT argument; a private build
must identify its own source base and file hashes. Preserve explicit corrections
for audit/owner clock offsets, literal-template lineage and setup classifications
rather than rewriting raw manifests. New continuation rows may expose additional
search metadata without changing previously published rows.

For an authorized batch, declare the hypothesis, acceptance test, budgets, seed
and actual checkpoint before launch. Reconcile all owners after exit, charging
canaries, retries, selection and composition work. Preserve genuine negative
outcomes. Only a fresh-verified improvement of the overall Extra or complete-
Lunatic record may retain a new scoring `.rpy`; previously selected historical
bytes remain unchanged. Raw actions, run JSON, model datasets and videos stay local.
At an owner-requested handoff, finish only the current batch and close its drivers;
an unused budget or proposed next experiment does not authorize another batch.
