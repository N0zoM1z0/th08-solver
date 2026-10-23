# Saved learned artifacts

These compact policy/risk snapshots are intentionally tracked at the owner's
request. Raw training rows, new action/replay tapes and per-run experiment reports
remain under ignored `reports/local/`. Saving a model does not promote its
training data or evaluations to certified expert routes.

`artifacts.tsv` records each table's relative path, SHA-256, byte length and number
of data rows (excluding its header). The artifact CTest verifies the complete file
set and exact identities. The native policy contract also loads every legacy pack
through the production parser. Changes need an explicit new snapshot/version;
historical weights are not refreshed from later experiments.

## Legacy empirical risk, version 1

| Directory under `legacy-risk/` | Supplied role | Status |
| --- | --- | --- |
| `teacher-near-v1` | Teacher observations near the player | Historical comparator |
| `teacher-all-v1` | Broader teacher observations | Historical comparator |
| `teacher-plus-student-v1` | Teacher plus student observations | Exact pack used in the maintained learned-risk experiments |

The twelve tables are copied byte-for-byte from the October 22 preparation
archive. Its original ZIP SHA-256 is
`dc639553abc3932f426c8bfc9635781393536025fa69eef2190bea2a1d19f275`.
The supplied histogram aggregation script has SHA-256
`d864dbc990cae8de693cd9166ea89e083cf5398bc4145118026424e5c113eea0`;
it is historical provenance, not a Python implementation/dependency of this
repository. The original preparation inputs remain unchanged. The current remote
four-file pack independently matches `teacher-plus-student-v1` exactly.

The supplied corpus used shot 0. Exact contributing seed/episode membership,
sampling choices and a disjoint held-out evaluation cannot be recovered from a
complete manifest available here. Those parts of lineage are **unknown**. Treat
previous diagnosis cases as potentially overlapping; these tables alone establish
neither independent-seed generalization nor model quality on other shots.

Each directory is a complete `--risk-dir` input:

- `risk_table_merged.tsv`: segment, sprite, width times ten, speed/age buckets,
  horizon, count, binned error q99/q999 and empirical frequency above two pixels.
- `risk_backoff_seg.tsv`: segment/speed/age/horizon q99 backoff.
- `risk_backoff_global.tsv`: speed/age/horizon q99 backoff.
- `nowarn_merged.tsv`: segment and 16-pixel cell coordinates, event count and
  empirical no-warning births per observed update.

Horizons are 4/8/16/32 updates. The supplied error histogram edges are
0.5/1/2/3/4/6/8/12/24 pixels; its overflow bin is represented by 24, so these
quantiles are not hard error bounds. The production loader uses q99, retains
fine/segment rows with count at least 200 and accepts global backoff without that
cutoff. The controller's margin caps and optional soft/hard/phantom rules are
separate policy choices. No table certifies future collision safety.

For example, pass `--risk-dir models/legacy-risk/teacher-plus-student-v1` to
`th08_online`. Loading is explicit; no saved model is enabled by default.

New models must record their input/label contract, training/tuning/evaluation
splits, action-space assumptions, training implementation/configuration and
validation limitations alongside their small weights and identity entry. Teacher
future may label offline examples; student features must contain only information
available at the corresponding observation.
