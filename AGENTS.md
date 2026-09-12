# th08-solver engineering contract

Read `docs/STATUS.md`, `docs/ARCHITECTURE.md`, and the affected interfaces first.
The current user scope is offline component/solver development. Do not launch
Wine, a Linux game port, or an input controller as an implicit verification step.

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
  `gpt-6-astra: <English summary>`; do not rewrite older subjects solely for this convention.

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
