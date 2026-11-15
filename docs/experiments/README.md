# Experiment records

[LESSONS](../LESSONS.md) summarizes what these records show. The two tables are raw
records and are appended to, never rewritten.

| File | Content |
| --- | --- |
| [campaign-ledger.tsv](campaign-ledger.tsv) | One line per allocated task ID (and later lifecycle notes for the same ID). IDs are never reused or reset |
| [results.tsv](results.tsv) | One row per task with entry, controls, argv, outcome, actual misses/bombs/captures, costs, per-stage results and source/input hashes |

Values containing tabs or newlines are escaped; structured fields are compact JSON.
**An empty field means unrecorded or unavailable, never zero.** A `complete`
outcome can be a valid run below target; failed, partial or resource-aborted runs
never establish a route score. Per-stage results (`stage_results`) give displayed
score, point items, time orbs, graze, point value and night clock at each stage clear.

The working copy of the ledger lives in ignored `reports/local/`; these published
copies are refreshed at publication. The last published task is 2209.
