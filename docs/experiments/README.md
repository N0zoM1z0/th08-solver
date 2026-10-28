# Experiment journal

The owner requested a durable record of every experiment and exploration on
2026-10-27. This directory records hypotheses, evidence, declared parameters,
costs, failures and conclusions. [Status](../STATUS.md) owns the current bests and
operating contract; [Native evidence](../../reports/native/README.md) owns the
published replay index.

| Record | Purpose |
| --- | --- |
| [Campaign lifecycle ledger](campaign-ledger.tsv) | All existing task allocations and original lifecycle notes, including repeated notes for the same ID. IDs are never reset or reused |
| [Per-task results and provenance](results.tsv) | One row per task: available entry/controls/argv, outcome, actual bombs/captures, costs, resource aborts, binary/source/DAT/input/seed-receipt hashes and evidence location |
| [October27 phase composition](2026-10-27-phase-composition.md) | All37 exact baseline boundaries, spell strategy experiments, complete combinations and the6.5B milestone |
| [October27 VPS exploration](2026-10-27-vps.md) | Questions, source/human evidence, parallel-search design, complete canaries, declared variations, failures and resulting decisions |
| [October28 storage cleanup](2026-10-28-storage-cleanup.md) | Exact-restore media archives, retained evidence, reclaimed bytes and recovery dependencies |

The TSV files use a header row. Tabs/newlines within values are escaped;
structured entry, controls, argv, cost and edit fields use compact JSON values. The `retail_observation` field
keeps actual observer/EXE/replay identities, partial capture/state observations,
explicit stops and any terminal retail validation separate from Native results.
`observation_replanned_no_certificate` preserves an explicitly stopped playback;
its partial zero-miss state is not a complete-route result.
An empty field means **unrecorded or unavailable**, never zero. The backfill
preserves every old ledger note. `historical_ledger_only` explicitly marks an
older task without an available structured owner receipt; it does not invent
measurements or convert that task into a success. `running_or_unresolved` needs
a terminal owner receipt before its result can be accepted. A complete report
from a resource-aborted owner still certifies nothing.

For each new batch record the hypothesis and decisive acceptance test before
launch, reserve IDs centrally, then reconcile each owner after exit. Include
original-entry seed/checkpoint lineage, changes from the comparison profile,
wall/search/trial budgets, all additional retries/target-selection costs,
actual misses/bombs/capture outcomes and the next decision. A high-scoring
prefix is not a complete-route result. Concurrent wall times are not isolated
CPU benchmarks. Fresh/index agreement is not an independent physics oracle or
retail equivalence.

Future `.rpy` retention uses the project's **overall Extra and complete-Lunatic
records**, across shots. Only a verified new overall record is retained and
published; other outcomes remain in this journal with their necessary local
input evidence. Previously owner-selected historical replay bytes are preserved.
Raw manifests, reports, rejected tapes and videos remain under ignored
`reports/local/`; the public journal is compact experiment metadata and analysis.

The current private metadata exporter is
`reports/local/vps-native-20261027/export-experiment-journal.cjs`. It joins the
original ledger to existing owner/verification receipts and copies only the
explicit public columns; it does not regenerate or rewrite historical reports.
Before publishing, reconcile in-flight rows, check TSV identities against their
receipts and run the documentation CTest. The public journal remains readable
without that local orchestration helper or proprietary game data.
