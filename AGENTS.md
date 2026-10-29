# Engineering rules

Read [README](README.md), [Status](docs/STATUS.md) and affected interfaces before
changing code. Follow the owner's latest scope. Current maintenance does not
resume scoring: the owner cleared the persistent goal and requested a batch
handoff. New scoring needs renewed instruction.

## Scope and evidence

- Maintain a C++17 offline solver and the tracked Native headless TH08 import.
  Keep upstream production code in this repository, with its third-party notices;
  do not replace it with a submodule, ignored checkout or separate feature branch.
- The project targets are Extra 4 billion and complete Lunatic 7 billion. The
  6.5 billion milestone is achieved. All shots are permitted, with Scarlet Team
  preferred; preserve Magic Team and solo Sakuya progress. Team story routes use
  1 → 2 → 3 → 4A → 5 → Final B.
- Scoring is No Miss / No Bomb, with captures optional. Bomb experiments are
  paused. Preserve strict NMNB/all-capture CLI defaults and independent bomb and
  capture controls. Record actual misses, bombs and capture outcomes for every trial.
- The October 27, 2026 all-shot human references are Extra 3,178,460,830 and
  Lunatic 7,053,332,460. A complete no-miss result must exceed its corresponding
  reference under the declared constraints before Wine replay validation.
  Reaching 7 billion alone does not pass the story gate. Preserve historical
  same-shot references separately.
- Before a new scoring episode, launch the owned Wine game and capture its original
  RNG seed at the observer's pre-RegisterChain boundary. Record entry settings,
  checkpoint, original receipt and source provenance before Native exploration.
  Keep that seed fixed; never choose or reroll a seed after viewing Native scores.
  Existing episodes, historical benchmarks and regression fixtures keep their identities.
- Wine is for local seed acquisition and qualified retail replay playback. Online
  MPC and live Wine solving are paused. New scoring replay names use the exact
  eight-byte `ysopwnab`; preserve historical names and replay bytes.
- Optimize complete spell/scenario segments and compose strategies from explicit
  original-entry checkpoints. Carry actual RNG, bullets, actors, items, shots and
  lifecycle state; reset only what the real transition resets. Reexecute the first
  N owned inputs, rather than injecting selected state fields or grafting old futures.
- Do not require menu, camera or practice-prelude reconstruction for a supported
  checkpoint. Named, seeded and recorded visual RNG hooks are permitted, but never
  silently replace gameplay RNG or share an action-dependent future across branches.
- Unknown gameplay opcodes/state must stop the affected case. Distinguish synthetic,
  DAT-controlled and source-faithful evidence. A failed or incomplete rollout cannot
  certify a prefix. Only complete-route verification establishes a route score;
  fresh/index agreement does not independently prove retail physics equivalence.

## Code quality and verification

- Keep code comments and maintained documentation in English. Do not introduce a
  Python implementation/dependency or alter original preparation artifacts.
- Separate immutable parsing, owned simulation, planning, replay and CLI/reporting.
  Use typed interfaces, not cross-tool globals or `main()` symbols. Keep case-specific
  adapters out of generic kernels. Prefer the smallest complete change and remove
  genuinely unused scaffolding rather than building a speculative framework.
- Document ownership/lifetimes, update order, units, checkpoint assumptions,
  unsupported behavior and terminal boundaries. Use named structures, small functions
  and `.clang-format`; comments should explain reasons rather than restate code.
- Share immutable data and own mutable state. Reuse buffers when measurement justifies
  it. No fast-math, approximate collisions, speculative SIMD or unsafe state merging.
  Proposal search/pruning/cell merging may be approximate; Native rollouts decide safety.
- Do not silently alter budgets, tie-breaking, RNG consumption or failure meaning to
  improve a benchmark. Charge retries, failed branches and target-selection work.
- Add focused regressions for meaningful contracts. Prefer a few complete-duration,
  transition, deterministic replay and failure cases over redundant implementation tests.
  Keep useful independent source checks; do not delete inconvenient failures.
- Run relevant checks and the normal CTest suite before publication. For changed
  DAT/runtime behavior, run the affected real-data command when data is available.
  Retain existing sanitizer coverage without making every edit a sanitizer project.
- Measure Release builds, excluding I/O from simulation timing. Record seed, profile,
  outcome, budget and environment. A faster failing solver is not a performance win.

## Documentation, retention and publication

- Keep the compact current document set linked by README. Update the document that
  owns a fact. Separate current status, proposed work and dated historical evidence.
  Review all maintained Markdown, including model and evidence indexes. Use normal
  English spacing, dates such as "October 27, 2026", and spaces before physical units.
  Preserve code, identifiers, paths, hashes and recorded numeric values when editing prose.
- Update affected generators and outputs together; run the documentation CTest.
  Preserve original reports, lifecycle notes, failures and missing measurements.
  Historical journals may be edited for clarity, but must not be presented as current plans.
- `preparations/` is local-only and was removed from published history after a verified
  ZIP download. Never reintroduce it. Raw DAT/EXE/assets, generated reference translation
  units, new action tapes, experiment JSON and videos remain ignored; do not force-add them.
- Preserve all selected historical replay bytes and their original seed/prefix lineage.
  [The progress index](reports/native/replays/progress.tsv),
  [evidence index](reports/native/README.md) and [Validation](docs/VALIDATION.md)
  identify the owner's authorized progress replays, including online recordings and
  later Scarlet/Magic improvements. For future work, retain/publish a new `.rpy` only
  for a freshly verified improvement of the project's overall Extra or complete-Lunatic
  record. Other shot results and negative experiments belong in the journal.
- The selected animated README diagram under `docs/media/` is authorized. Its original
  `gifs/` bundle, MP4 and Python generator remain local; this exception authorizes no
  other video commits. Compact learned policy/risk snapshots under `models/` are also
  authorized with exact weights, schema, hashes, provenance and evaluation limitations.
- Follow the [storage retention record](docs/experiments/2026-10-28-storage-cleanup.md).
  Retain the 29 compressed gameplay MP4s, current seed/replay environments, two in-use
  old Wine prefixes and all selected replay/experiment/source/binary evidence. Deleted
  MKV/PCM originals and retired recovery archives are not restorable promises. Keep the
  verified archives, inventories and restore proofs for 45 inactive Wine prefixes and
  656 private object intermediates; restore them before using retired paths.
- Publish tested work directly to `main`: fetch first, preserve others' changes, and
  never force an unrelated update. Use `gpt-6.1-sol-max: <English summary>` for new
  assistant commits; do not rewrite old commits solely to change their prefix.
- At a requested handoff, finish the already-running authorized batch, record outcomes
  and leave no owned experiment queue/test driver running. Unused budget or a proposed
  next step does not authorize another scoring batch during cleanup.

## Resource ownership

- The last local profile permits at most two owned drivers, in one user systemd service,
  with a total two-core quota and low CPU/I/O priority. The standing ceiling is four
  drivers, including solvers, probes, replay verifiers and test owners; fork children
  belong to one owner. Do not add a test driver beside a full solver allocation.
- Bound each process to 4 GiB address space and each owned solver tree to 4 GiB summed
  RSS. The local service uses MemoryHigh 6 GiB, MemoryMax 10 GiB and no swap. Admit
  with at least 12 GiB host MemAvailable; abort owned work below 8 GiB. Resource-aborted
  work remains charged and certifies nothing. Compile with at most `--parallel 2`.
- Keep one cumulative campaign ledger at `reports/local/shadow-runtime-v1/tasks.tsv`.
  Never reset it, reuse IDs, omit failures or infer IDs from line count. Read the current
  cap and recorded extensions in [Status](docs/STATUS.md#operational-resume-contract).
  Standing authorization permits further 64-task extensions, recorded before use;
  it does not authorize scoring after the owner's stop instruction.
- Remote Native search on `yann@185.189.44.160` is paused after a host-identity change.
  Do not reconnect, replace host keys or infer closure of the 19 unresolved owners.
  If explicitly resumed after identity verification, preserve unrelated workloads and
  the old checkout, use isolated compute under the owned user systemd manager, and
  keep local seed acquisition plus local verification of returned selections.
- The standing remote ceiling is 32 Native owners / 32 cores, low priority, 4 GiB
  address space/tree RSS per owner, 96 GiB aggregate owned RSS and a 128 GiB hard
  service limit with no swap. Admit with 128 GiB host MemAvailable; abort below 96 GiB.
  Reserve IDs from the same ledger before dispatch. Bind source/binary/library/DAT/
  input/seed-receipt hashes and pass complete cross-host execution/row-byte canaries
  before scoring. No remote allowance increases the local concurrency ceiling.
