# th08-solver engineering contract

Read `docs/STATUS.md`, `docs/ARCHITECTURE.md`, and the affected interfaces first.
The current user scope is offline component/solver development. Do not launch
Wine, a Linux game port, or an input controller as an implicit verification step.

## Current solver-first scope (2026-10-02)

- Start from explicit, reproducible spell/scenario checkpoints. Reconstructing
  menus, practice preludes, camera or rendering is not a prerequisite for solving.
- Prioritize complete-duration continuous-danmaku scenarios, algorithm comparisons,
  replay and end-to-end performance. A short safe horizon is not a completed scene.
- A controlled profile may replace explicitly identified visual RNG consumers
  with deterministic, seeded hooks. Record that policy; do not claim its trace is
  retail-equivalent. Gameplay randomness and candidate-dependent state remain real
  dependencies, not arbitrary NOPs or one tape reused across divergent branches.
- Distinguish synthetic stress scenes, DAT/ECL-derived controlled scenarios and
  source-faithful spell/stage runs. Stage transitions preserve carried state unless
  the scenario explicitly models a reset. Unknown gameplay semantics still stop.
- Work directly on main and push verified checkpoints; no PR/new working branch.

## Implementation

- Write maintained documentation, code, comments, and commit messages in English.
  Communicate with the user in Chinese. Preserve original preparation artifacts
  in their source language rather than rewriting historical evidence.
- Use C++ for maintained components, tools, tests, and benchmarks. No Python
  implementation or Python dependency; old preparation archives remain evidence.
- Keep code reviewable: named structures, explicit units and ownership, small
  responsibilities, `.clang-format`, and comments for non-obvious engine rules.
- Prefer contiguous storage, reuse scratch memory, and measure realistic batches.
  Do not use fast-math, unsupported SIMD assumptions, or approximate geometry
  without an independent correctness check and an explicit domain contract.
- Keep raw game data and extracted assets in ignored directories. Track the
  user's original `preparations/` unchanged. Do not force-add ignored data.
- Save verified progress in frequent, focused commits. New commit subjects use
  `gpt-dots: <English summary>`; do not rewrite older subjects solely for this convention.

## Evidence

- State exactly what is covered: structure, restricted execution, model route,
  complete offline world, or original-game validation are distinct statuses.
- Unknown opcodes, inherited state, callbacks and RNG consumers must stop a
  dependent execution or optimization; do not invent defaults or silent NOPs.
- Preserve file/sub/PC/mask identities; 222 IDs are not 222 identical entry cases.
- Shot requests are not successful bullet-pool allocations. Search exhaustion
  is not mathematical impossibility. A finite-horizon path is not a full spell.
- Maintain the source/DAT hashes and first blocker in reproducible outputs.

## Verification and handoff

Run relevant CTest cases and native DAT checks for changed parser/runtime code.
For geometry changes run brute-force differential tests and, when available,
the pinned source oracle. Use sanitizers for binary parser/ownership changes.
Update `docs/STATUS.md` with completed work and the next concrete missing behavior.
Use `docs/README.md` as the documentation map. Keep implemented status separate from
`docs/COVERAGE.md` acceptance criteria; refresh affected report generators and outputs
together. Run the native documentation CTest after maintained documentation changes.
