# Current native report index

Refreshed: 2026-09-12. These are current generated C++ component baselines, not the
historical preparation interpreter's output. Runtime baseline:
`6317947c8bd289722314b267e8522ce75eb77b1d` for the existing broad component reports.
The new first-spell report separately records the owned entry-prefix implementation;
it does not change those older component results or claim a completed world.
Input hashes and reference boundaries are in [Provenance](../../docs/PROVENANCE.md).

| Producer | Reports | Meaning |
|---|---|---|
| `th08_audit` | `summary.json`, `members.tsv`, `subprograms.tsv`, `opcodes.tsv`, `spell_sites.tsv`, `shot_sites.tsv` | DAT/ECL structural identities; zero complete spell solutions |
| `th08_audit` resource reports | `resource_summary.json`, `timelines.tsv`, `sht_*.tsv`, `anm_*.tsv`, `std_*.tsv` | Parsed fields and restricted ANM timing certificates, not complete resource execution |
| `th08_slices` | `slice_summary.json`, `slice_matrix.tsv`, `emitter_examples.tsv`, `emitter_benchmark.json` | Restricted entry/mask attempts, first blockers, owned payload checks and scoped scheduling timing |
| `th08_motion_cases` | `motion_summary.json`, `motion_sub40_frames.tsv`, `motion_sub40_route.tsv`, `motion_sub41_frames.tsv`, `motion_sub41_route.tsv` | Two fixed-entry particle models and independently replayed routes |
| `th08_animation_cases` | `animation_control_summary.json`, `animation_control_cases.tsv` | Unseeded 600-call ANM control/scalar profile; context blockers remain explicit |
| `th08_timeline_cases` | `timeline_control_summary.json`, `timeline_control_cases.tsv` | Timeline control with unknown world observations, not completed world effects |
| `th08_first_spell` | `first_spell_summary.json`, `first_spell_trace.tsv` | Selected ID2 Easy entry prefix with supplied GUI gates; pending effect51 blocker, no full entry/world/solution |
| `source_oracle` | `source_oracle.json` | Pinned native source-body comparisons by category; no game execution |
| `geometry_bench`, `planner_bench`, `bullet_slots_bench` | Corresponding `*_benchmark.json` files | Scoped component comparisons, not full-world performance |

Exact counts belong in the JSON summaries and per-case identities in TSVs. A
`PASSED` audit means its checks passed, not that all its entries executed to a
terminal world. Likewise, `FOUND_AND_REPLAYED` in the motion fixture report does
not mean a whole spell was captured. Fields named `*_atomic_failures` in the source
oracle count deliberate rejection/rollback test cases, not failed test results;
the corresponding mismatch count must be zero.

Timing fields vary by run. Preserve large integer digests exactly when processing
JSON: some consumers round integers above 2^53. Prefer the original file or a
lossless integer parser, not a floating-point reserialization of state digests.

Use [Validation](../../docs/VALIDATION.md) for reproduction. Experiments, alternate
seeds and sanitizer timing go to ignored `reports/local/`. Do not edit generated
numeric results to match a document; rerun their producer and review the difference.
