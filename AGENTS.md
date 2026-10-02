# Engineering rules

Read README.md, docs/STATUS.md and the affected interfaces before changing code.
Follow the owner's latest scope. The owner authorized a tracked native headless TH08
import and solver experiments over complete real-data scenes. Keep upstream code in
this repository, not an ignored checkout, submodule or separate feature branch.

## Solver-first scope

- The goal is accurate, efficient offline planning over complete spell/scenario
  segments, eventually genuine continuous stages
- Explicit reproducible checkpoints are valid starting points. Do not make menu,
  practice-prelude, camera or rendering reconstruction a prerequisite
- Controlled visual RNG hooks are allowed when named, seeded and recorded. Never
  silently substitute gameplay RNG or use one future for action-dependent branches
- Preserve carried bullets, RNG, actors and lifecycle state across transitions.
  Reset only where the modeled transition actually resets them
- Unknown gameplay opcodes/state must stop the affected case. Classify synthetic,
  DAT-controlled and source-faithful evidence separately. No game launch is required

## Code quality

- Maintain C++17 code/tools/tests and English code comments/docs. Keep original
  preparation artifacts unchanged; no Python implementation/dependency
- Keep responsibilities separate: parsing/immutable programs, owned simulation,
  planning, replay and CLI/reporting. Case-specific adapters must not spread into
  generic kernels. Use explicit typed interfaces, not cross-tool globals or main()
  symbols as dependencies
- Prefer the smallest complete change. Do not build a general engine/framework for
  one narrow case, add speculative features, or preserve unused scaffolding
- Document why, ownership/lifetimes, update order, units, checkpoint assumptions,
  unsupported behavior and terminal boundaries. Avoid comments that merely restate code
- Use named structures and small functions; apply .clang-format. Share immutable
  data, own mutable state, reuse buffers where measurement justifies it
- No fast-math, approximate collision shortcuts, speculative SIMD or unsafe state
  merging. Optimize against a measured workload while preserving stated semantics
- Do not silently change budgets, tie-breaking, RNG consumption or failure meaning
  to improve a benchmark. Record additional retry/target-selection costs

## Proportionate verification

- Add a focused regression that reproduces the bug or guards a meaningful contract
- Prefer a small number of end-to-end scenarios covering a complete duration,
  transitions, deterministic replay and a failure case over many redundant tests
- Keep useful independent source/differential checks. Delete tests only when unused,
  duplicated or made obsolete by a replacement; do not delete an inconvenient failure
- Run relevant tests and the normal CTest suite before publishing. For changed
  DAT/runtime behavior, run the affected real-data command when data is available
- Existing CI sanitizer coverage remains useful. Do not create elaborate new
  ASan/UBSan infrastructure or make every small change a sanitizer project
- Measure Release builds, exclude file I/O from simulation timing, and record seed,
  profile, outcome, budget and environment. A faster failing solver is not a win
- Fresh replay checks execution/index agreement; it is not an independent physics
  oracle or proof of retail equivalence

## Documentation and publication

- Keep only the compact current document set linked by README. Update the document
  that owns the fact instead of adding another roadmap, diary or duplicate checklist
- Keep current status, future work and historical/component evidence distinct.
  Update report generators and affected outputs together; run the documentation CTest
- Raw DAT/EXE/assets, generated reference translation units and local experiments
  remain ignored. Do not force-add them; preserve third-party notices
- For authorized assistant changes to this N0zoM1z0 repository, use
  `gpt-6.1-sol: <English summary>`. Do not rewrite older commits solely for their prefix
- The owner requested direct main progress commits, not feature branches/PRs.
  Fetch first, preserve others' work, never force an unrelated change, and publish
  only tested scope. Do not infer permission to continue beyond the latest request
