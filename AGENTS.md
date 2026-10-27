# Engineering rules

Read README.md, docs/STATUS.md and the affected interfaces before changing code.
Follow the owner's latest scope. The owner authorized a tracked native headless TH08
import and solver experiments over complete real-data scenes. Keep upstream code in
this repository, not an ignored checkout, submodule or separate feature branch.
The October 26 scope is Native-first scoring for Extra and the complete Magic
Team Lunatic 4A-to-Final-B route. The latest continuation makes bomb permission
and full-capture requirements independent CLI controls: the latest scope is
No Miss, No Bomb, captures optional. Bomb experiments are paused. Preserve strict NMNB/all-capture
defaults and historical records, and record actual bomb use/captures for every
trial. Require a complete no-miss score above the corresponding human reference
under its declared constraints before Wine retail replay validation.
The owner resumed after the3B/5B Native milestones. The final comparison now
uses the highest published human score across all12 team/solo shots: Extra
above3,178,460,830 and Lunatic above7,053,332,460 in the October27 snapshot.
First reach6,000,000,000 on the complete Lunatic route. Preserve historical
same-shot references separately; use the all-shot reference for the new retail
replay gate. The current Magic Team episodes remain valid fixed-seed candidates.
The October27 owner permits all shots and prioritizes Scarlet Team
(Sakuya/Remilia, shot2). Its complete Lunatic route also uses4A-to-Final-B.
Preserve Magic Team progress; capture new original retail seeds for each new
shot/entry episode before Native scoring.
Before each new scoring episode, launch the owned Wine retail game and capture
its original RNG seed at the observer's pre-RegisterChain boundary. Record the
entry configuration, checkpoint and seed provenance before Native exploration.
Keep that captured seed fixed across the episode's offline candidates; never
replace it with a Native-selected or hindsight-selected seed. Historical fixed
benchmarks and regression fixtures keep their original identities. Wine is used
for seed acquisition and qualified retail replay playback; online MPC and live
Wine solving remain paused. New scoring replay names use the
owner-selected eight-byte `ysopwnab`; preserve historical replay bytes.

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
  DAT-controlled and source-faithful evidence separately. Fixed Native regression
  cases require no game launch; new scoring episodes follow the seed policy above

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
- The fork-based planner may approximate proposal search, pruning and cell merging.
  Native rollouts decide safety; failed or incomplete rollouts cannot certify a prefix
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

- All `preparations/` material is local-only. The owner requested a verified ZIP
  download and removal of that directory from all published Git history; never
  reintroduce its files through later commits or old branches
- The owner authorizes the selected animated README diagram in `docs/media/`.
  Keep its original `gifs/` bundle, MP4 and Python generator local; this does not
  introduce a Python implementation/dependency or authorize other video commits
- Keep only the compact current document set linked by README. Update the document
  that owns the fact instead of adding another roadmap, diary or duplicate checklist
- Keep current status, future work and historical/component evidence distinct.
  Update report generators and affected outputs together; run the documentation CTest
- Raw DAT/EXE/assets, generated reference translation units and local experiments
  remain ignored. Do not force-add them; preserve third-party notices
- Keep new generated replays, action tapes and experiment JSON under ignored
  `reports/local/`. Preserve the existing tracked artifacts as historical snapshots;
  progress commits contain code and maintained prose, not new or refreshed run artifacts
- The owner explicitly authorizes the selected `.rpy` files corresponding to the
  recorded Stage 4A, standalone Final B and complete 4A-to-Final-B story. Keep their
  exact identities/provenance in maintained documentation. This does not authorize
  committing videos, action tapes or new experiment reports
- The October 26 handoff also authorizes the selected highest-scoring Native Extra
  replay `reports/native/replays/extra-native-nmnb-2564311730-seed19732.rpy`.
  Preserve its exact bytes and provenance; other new experiment artifacts stay local
- The Native-first continuation also authorizes the selected complete progress
  replays `extra-native-nmnb-2589417330-seed10025.rpy` and
  `story-native-nmnb-4a-finalb-3890487560-seed40755.rpy` under
  `reports/native/replays/`. Preserve exact bytes and provenance; failed tapes,
  experiment reports and videos remain local
- The owner's continuing progress-replay authorization also covers the selected
  story improvement `story-native-nmnb-4a-finalb-3903603650-seed40755.rpy` in the
  same directory. Preserve earlier selected replays unchanged
- Continuing progress-replay authorization also covers the selected Native
  story `story-native-nmnb-4a-finalb-4126102910-seed40755.rpy`; retain its exact
  checkpoint-continuation provenance and all earlier selected bytes
- Continuing progress-replay authorization covers the selected retail-seeded
  Native story `story-native-nmnb-4a-finalb-4160457200-seed1805.rpy`; preserve
  its seed receipt, full-entry provenance and all earlier selected bytes
- Continuing progress-replay authorization covers the selected retail-seeded
  story `story-native-nmnb-4a-finalb-4311260760-seed1805.rpy`; preserve its exact
  prefix lineage, full-entry verification and earlier selected replays
- Continuing progress-replay authorization covers the selected retail-seeded
  Extra `extra-native-nmnb-2504560470-seed46997.rpy`; preserve its exact source,
  prefix lineage and earlier selected Extra records
- Continuing progress-replay authorization covers the selected retail-seeded
  Extra `extra-native-nmnb-2508068110-seed46997.rpy`; preserve its exact bytes,
  verification and earlier selected progress files
- Continuing progress-replay authorization covers the selected retail-seeded
  Extra `extra-native-nmnb-2603278620-seed46997.rpy` and
  `extra-native-nmnb-2620568090-seed46997.rpy`; preserve their full-entry/prefix
  lineage, exact verification and all earlier selected bytes
- The October27 owner explicitly selects the verified milestone replays
  `extra-native-nmnb-3010040230-seed46997.rpy` and
  `story-native-nmnb-4a-finalb-5025195380-seed1805.rpy` for publication under
  `reports/native/replays/`. Preserve exact hashes, original retail receipts,
  complete input-prefix lineage and all earlier selected bytes. This does not
  authorize raw tapes, experiment reports, videos or other new replay binaries
- The owner also authorizes selected compact learned policy/risk snapshots under
  `models/`. Preserve exact weights/tables with schema, hashes, provenance and
  training/evaluation limitations. This exception does not include raw datasets,
  action/replay tapes or experiment reports; those remain under `reports/local/`
- For authorized assistant changes to this N0zoM1z0 repository, use
  `gpt-6.1-sol-max: <English summary>`. Do not rewrite older commits solely for their prefix
- The owner requires low process concurrency on the shared host. The October26
  continuation authorizes cumulative budget extensions while bounding memory;
  the active window is768 total campaign tasks, without resetting the ledger.
  The October27 continuation records successive64-task extensions from640 and704
  before use, under that standing authorization.
  Record further extensions before using them; this authorization does not raise
  the four-concurrent-driver limit, including solver, probe, replay-verification
  and test owners. Native rollout fork children belong to their single owner;
  do not start four solvers plus an extra test driver. The owner's latest
  October27 authorization raises the entire phase's former2GiB limits to4GiB.
  New owners use a4GiB per-process
  address-space limit,
  a4GiB summed-RSS limit per owned solver tree, and host-memory admission/abort
  guards. Preserve12GiB MemAvailable before admission; stop owned work below8GiB.
  Resource-aborted work remains charged and cannot certify a gameplay result
- At an owner-requested session handoff, finish the already-running authorized
  batch, record its outcomes and leave no owned experiment queue/test driver
  running. A proposed next step or unused budget is not authorization to start
  another batch during cleanup
- The owner requested direct main progress commits, not feature branches/PRs.
  Fetch first, preserve others' work, never force an unrelated change, and publish
  only tested scope. Do not infer permission to continue beyond the latest request
