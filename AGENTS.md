# Engineering rules

Read [README](README.md), [Status](docs/STATUS.md), the [Roadmap](docs/ROADMAP.md)
and affected interfaces before changing code. Follow the owner's latest scope.
On November 15, 2026 the owner re-authorized scoring; play is offline Native only.

## Scope and evidence

- Maintain a C++17 offline solver around the tracked Native headless TH08 import.
  Keep upstream production code in this repository with its third-party notices;
  do not replace it with a submodule, ignored checkout or separate branch.
- Targets: complete Lunatic 1 → 2 → 3 → 4A → 5 → Final B above 7 billion and above
  the October 27, 2026 all-shot human reference 7,053,332,460; Extra 4 billion
  (reference 3,178,460,830). All shots are permitted; Scarlet Team is the current
  record holder. Preserve Magic Team and solo Sakuya progress.
- Scoring is No Miss / No Bomb, captures optional; bomb experiments are paused.
  Keep strict NMNB/all-capture CLI defaults and independent bomb and capture
  controls. Record actual misses, bombs and capture outcomes for every trial.
- Before a new scoring episode, capture the original RNG seed from the owned Wine
  game at the observer's pre-RegisterChain boundary and record entry settings and
  receipt. Keep that seed fixed; never choose or reroll a seed after viewing
  Native scores. Wine is for seed acquisition and retail replay playback only;
  online/live Wine solving is retired. Replay names are the eight bytes `ysopwnab`.
- Compose strategies from explicit original-entry checkpoints: re-execute the first
  N owner inputs and carry the actual RNG, bullets, actors, items and lifecycle
  state. Never inject state fields or graft an old future onto a changed world.
- Unknown gameplay opcodes/state must stop the affected run. A failed or incomplete
  rollout cannot certify a prefix. Only a complete route establishes a route score;
  fresh/natural agreement does not prove retail physics equivalence.

## Code quality and verification

- Code comments and documentation are in English. Do not add a Python
  implementation or dependency.
- Separate immutable parsing, owned simulation, planning, replay and CLI/reporting.
  Use typed interfaces, not cross-tool globals. Prefer the smallest complete change
  and delete unused code rather than keeping speculative frameworks.
- Document ownership, update order, units, checkpoint assumptions and unsupported
  behavior; comments explain reasons rather than restate code. Use `.clang-format`.
- No fast-math, approximate collisions, speculative SIMD or unsafe state merging.
  Proposal search may approximate; Native rollouts decide safety.
- Do not silently change budgets, tie-breaking, RNG consumption or failure meaning to
  improve a benchmark. Charge retries, failed branches and selection work.
- Add focused regressions for meaningful contracts. Run the relevant checks and the
  normal CTest suite (with `TH08_HEADLESS_DAT` when the DAT is available) before
  publication. Measure Release builds, excluding I/O from simulation timing.

## Documentation, retention and publication

- Keep the compact document set linked from README; update the document that owns a
  fact. Use dates such as "October 27, 2026" and spaces before units. The
  `documentation` test checks links, anchors and prose spacing.
- The animated README diagram `docs/media/shadow-mpc-online.gif` is owner-selected
  and must be kept. Raw DAT/EXE/assets, `preparations/`, action tapes, run JSON and videos stay
  ignored. Never reintroduce `preparations/` into history.
- Preserve every selected replay's bytes and its seed/prefix lineage
  ([progress.tsv](reports/native/replays/progress.tsv)). Add a new `.rpy` only for a
  fresh-verified improvement of the overall Extra or complete-Lunatic record.
  Other results belong in [results.tsv](docs/experiments/results.tsv) and [Lessons](docs/LESSONS.md).
- Keep one cumulative task ledger (working copy in `reports/local/`, published copy
  [campaign-ledger.tsv](docs/experiments/campaign-ledger.tsv)). Never reset or reuse
  IDs or omit failures; the next ID follows the maximum first-column ID (2209 at the
  November 15 cleanup), not the line count.
- Publish tested work directly to `main`: fetch first, preserve others' changes,
  never force-push. Commit messages use `gpt-6.1-sol-max: <English summary>`.
- At a requested handoff, finish the running batch, record outcomes and leave no owned
  experiment or test driver running. Unused budget does not authorize a new batch.

## Resource ownership

- Local: at most two owned drivers (solvers, probes, verifiers and test owners count;
  fork children belong to their owner), two cores in total, low CPU/I/O priority.
  Each process is limited to 4 GiB address space and each owner tree to 4 GiB summed
  RSS. Admit work at 12 GiB host `MemAvailable` or more; abort below 8 GiB.
  Resource-aborted work is charged and certifies nothing. Compile with at most
  `--parallel 2`.
- Remote Native search on `yann@185.189.44.160` is paused after a host-identity
  change. Do not reconnect or replace host keys until the owner verifies the
  identity; 19 remote owners remain unresolved. If resumed: at most 32 owners /
  32 cores, 4 GiB per owner, 96 GiB aggregate owned RSS, 128 GiB hard service limit,
  no swap; admit at 128 GiB `MemAvailable`, abort below 96 GiB; bind
  source/binary/DAT/input/seed-receipt hashes and pass cross-host replay canaries
  before scoring; verify returned selections locally.
