# Generated evidence index

Current solver handoff: 2026-10-02. This directory mixes current solver records with
explicitly scoped older component baselines. A report's producer/input/scope controls
its meaning; its presence is not a claim that every original spell or stage runs.
See [Status](../../docs/STATUS.md), [Scenarios](../../docs/SCENARIOS.md) and
[Validation](../../docs/VALIDATION.md) for interpretation and reproduction.

| Producer | Records | Meaning |
|---|---|---|
| `th08_headless` + `th08_headless_probe` + `tests/headless_real_data.cmake` | `headless_summary.json` | Native Stage 1, ID179 and portfolio IDs 85/89/193/195/199, ID85 pooled and ID89 direct ECL baselines, Stage 6b collision/input-latch witnesses, replay and O0/O3 comparison |
| `th08_scenario_cases` | `scenario_summary.json` | Three synthetic 7200-frame profiles, three baseline strategies; includes genuine failures |
| `th08_scenario_cases` | `scenario_escape.json` | Explicit left-goal escape witness; not a general algorithm fix |
| `th08_scenario_cases` | `scenario_recovery.json` | Geometry-derived retry, shared budget and target-scan cost; full replay |
| `th08_scenario_cases` | `scenario_long.tsv` | 72000-frame synthetic relay; not an original stage |
| `th08_spell_cases` | `spell179_summary.json`, `spell179_*_seed*_route.tsv` | Actual-DAT controlled ID179, three seeds/four strategies, success and failure-prefix replay |
| `planner_bench` | `planner_benchmark.json` | Fixed-model reference comparison, exact-successor query counts and local timings |
| `th08_audit` | `summary.json`, member/sub/opcode/spell/shot TSVs | Structural identities; legacy source-faithful complete-spell count remains 0 |
| `th08_audit` resource reports | `resource_summary.json`, timeline/SHT/ANM/STD TSVs | Parsed fields and restricted certificates |
| `th08_slices` | `slice_summary.json`, `slice_matrix.tsv`, `emitter_examples.tsv`, `emitter_benchmark.json` | Restricted ECL execution and component timing |
| `th08_motion_cases` | `motion_summary.json`, `motion_sub*_frames.tsv`, `motion_sub*_route.tsv` | Two older 600-frame particle fixtures, not complete Wriggle spells |
| `th08_animation_cases` | `animation_control_summary.json`, `animation_control_cases.tsv` | Restricted ANM scalar/control profiles |
| `th08_timeline_cases` | `timeline_control_summary.json`, `timeline_control_cases.tsv` | Timeline execution without complete world effects |
| `th08_first_spell` | `first_spell*` | Historical entry-prefix/supplied-state diagnostics; not the current required entry path |
| `source_oracle` | `source_oracle.json` | Pinned component source comparisons; zero mismatches is not whole-game validation |
| Other `*_bench` tools | Corresponding benchmark JSON | Scoped component timings; not additive estimates of complete solve cost |

Current controlled ID179 success does not rewrite the legacy audit/motion/entry fields
into source-faithful world completion. Synthetic and controlled scenarios must remain
separately labeled. A `SEARCH_LIMIT`, collision, unsupported opcode or missing context
is evidence to inspect, not a success or mathematical impossibility proof.

Native headless records use the tracked source revision and original calc-chain/shared
RNG/feedback under an explicit no-draw native float32 profile. They add coverage without
relabeling historical subset reports. Native timing samples use AMD EPYC 7B12, GCC 12.2,
Linux x86_64; wall-clock waiting is removed and initialization/output are excluded.
Regenerate with the optional real-data CTest, then copy its checked
`build-headless/headless-regression/summary.json` here. Keep comparison replay enabled
when refreshing O0/O3 evidence; tapes remain in the ignored build directory.

Experiment in ignored `reports/local/`; regenerate tracked records deliberately from
their maintained C++ producer. Do not commit raw game assets. Preserve large digests
without floating-point rounding and compare semantic outcomes before wall-clock time.
