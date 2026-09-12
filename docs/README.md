# Documentation guide

All maintained documents describe the current C++ implementation unless explicitly
labeled as a historical regression or source contract. Start with the following
four documents; specialized evidence is linked from them.

| Read | Question answered | Authority |
|---|---|---|
| [Status](STATUS.md) | What is implemented, verified, and still missing? | Current capability ledger; generated reports supply exact counts |
| [Architecture](ARCHITECTURE.md) | Why this design, where is the code, and who owns each state transition? | Current interfaces, algorithms and contracts |
| [Coverage roadmap](COVERAGE.md) | What does complete coverage require, and what comes next? | Unfinished work and acceptance criteria, not implemented features |
| [Validation](VALIDATION.md) | How are the claims reproduced and kept current? | Build/test profiles, report commands and documentation checks |

## Specialized documents

| Document | Purpose |
|---|---|
| [Provenance](PROVENANCE.md) | Pinned source/data identities, independent comparison chain and numerical limits |
| [Performance](PERFORMANCE.md) | Implemented optimizations, current benchmark samples and excluded costs |
| [Wriggle world contract](WRIGGLE_WORLD_CONTRACT.md) | Source-derived first integration target: actual practice/stage entry, familiars, RNG and endings |
| [Motion fixtures](MOTION_FIXTURES.md) | Exact assumptions behind the two replayed 600-frame routes |
| [ECL context storage](ECL_CONTEXT_STORAGE.md) | Thirty context slots, entity/global ownership and compact stack layout |
| [Regression ledger](REGRESSIONS.md) | Minimized counterexamples retained as guards; past failures are not current failures |
| [Build artifacts](BUILD_ARTIFACTS.md) | Tracked reusable generators/tests versus disposable local build sources |
| [Third-party notices](THIRD_PARTY_NOTICES.md) | Preserved reconstruction notice |
| [Generated report index](../reports/native/README.md) | Which tool produces each current report and what its result means |

## Maintenance rules

The controlling requirements are [AGENTS.md](../AGENTS.md) and the user's
[original task](../preparations/th08solver_sth.txt). Maintained code/docs are English;
original preparation materials retain their original language and attribution.

- Update the capability ledger when behavior changes; update the coverage roadmap
  when an integration milestone is actually met. Do not use a chronological diary
  as the current status page.
- Keep semantics in Architecture and evidence in Provenance/Validation. Link to
  exact reports rather than repeating every counter in every document.
- Refresh report generators as well as their outputs if scope labels change.
  Reproduce the affected tests before updating a claim.
- Preserve regression counterexamples and comparison baselines. Their old failures
  explain why current guards exist; they are not obsolete solver implementations.
- Keep `preparations/` unchanged. Its archived interpreters and reports are evidence,
  not maintained code or current coverage. Never promote their larger counts into
  the native ledger without verification.
- Leave ignored game data, pinned source and build scratch untouched during ordinary
  documentation cleanup. Reusable tools belong in tracked source directories.

The native `documentation` CTest checks maintained local inline links, empty leaf
headings, the declared core test count and selected status/report counts. It does
not replace semantic review, validate external websites, or prove world completeness.
