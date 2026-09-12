# First complete spell: execution plan

Decision: 2026-09-12. Finish one vertical slice before expanding families or
optimizing unrelated components. This plan records work to do, not a solved spell.
The current implementation ledger remains [Status](STATUS.md).

## Selected case

Start with **Wriggle ID 2, Easy, spell practice**, using `ecldata1sp.ecl`
timeline 0, wrapper sub42 and spell sub24. The selected spell instruction is
sub24 PC10, offset18116, mask0xf1; the execution difficulty bit is0x01.
Preserve the input hashes and native float32 profile in the
[source contract](WRIGGLE_WORLD_CONTRACT.md).

This is a small engineering target, not a claim that it is the game's easiest
spell or that all entry states are equivalent. Comparing the native DAT shows:

| Candidate | Additional integration cost | Decision |
|---|---|---|
| ID2 Easy, sub24 | Two familiar waves, but rotating bullet callees already have tested kernels; Easy omits later difficulty-only volleys | First target |
| IDs0/1, sub8 | Main and child contexts, repeated rings of spawned enemies, more transform behavior | Defer |
| ID6 Easy, sub30 | Additional enemy spawns and randomized shooting | Defer |
| ID10 Normal, sub34 | No direct familiar spawn in the main sub, but sub35 repeatedly replaces bullet sprites and chains transforms | Defer |

Do not run another broad candidate survey unless this target exposes a decisive
new dependency that invalidates the choice.

## Three acceptance gates

1. **Actual entry to spell start.** Own timeline, actor allocation, immediate ECL,
   frame tails and post-spawn stores. Run sub0's real effect51 prelude, wrapper42,
   EX19 and required player/background/ANM state. Reach ID2 at life1500 with the
   effective practice callback41. Retain the first unsupported instruction and
   its pending transaction; never acknowledge an unexecuted effect.
2. **Complete world to a real ending.** Implement only this case's reachable
   actor, bullet, effect, player and damage dependencies. Preserve shared RNG,
   immediate child execution, movement, collision and terminal phase ordering.
   First accept a correctly executed `SURVIVED_TIMEOUT` route as a full-duration
   survival result, explicitly not normal capture. `CAPTURED` is a separate
   subsequent acceptance requiring actual player-shot damage. A horizon limit,
   returned callee or retry-menu request alone is not a winning result.
3. **Solve and independently replay.** Search legal per-frame player actions
   against the complete world. Fork every candidate-sensitive state, not just
   player position. Replay the entire route through the terminal transition
   using independent collision scans and source-backed ordered-event checks.
   Persist inputs, hashes, actions, ending and first-divergence diagnostics.

Each gate needs a runnable C++ regression, a reproducible native DAT command and
an accurately labeled report. A blocked run is useful progress but passes no gate.

## Input and output contract

The first search policy uses no bombs and initially no player shooting, targeting
timeout survival. This is an explicit legal-action restriction, not permission
to assume invulnerability, disable collision, or suppress enemy behavior. Fix
character/shot type, power, initial player state, RNG and timing profile explicitly
before a complete run. Derive their initialization from the selected entry; if a
supplied checkpoint is used during development, label it supplied and retain its
provenance. An unknown entry value remains unknown until its first consumer.

Output must distinguish `CAPTURED`, `SURVIVED_TIMEOUT`, `PLAYER_DEAD`,
`UNSUPPORTED_WORLD_EFFECT`, `MISSING_ENTRY_STATE` and `SEARCH_LIMIT`. No complete
world or solver should turn a missing input or bounded search into success.

## Work rule and next executable boundary

Follow the earliest blocker in the selected run. Add the smallest correct behavior,
test it against pinned source evidence, rerun the same entry and commit the verified
change. Keep the same entry command and improve its reachable prefix; do not replace
it with disconnected microbenchmarks. Unknown callbacks, visual-state consumers,
pool lifetimes and RNG draws must stop the run rather than becoming NOPs.

The starting boundary is timeline offset39684: spawn sub0, position(30,-16),
life20, drop-2, score1000. Its immediate ECL sets interaction flags at offset244,
then requests sixteen effect51 particles at offset260. The initial implementation
retains the selected actor and pending spawn while blocked there, without
advancing the timeline or applying the post-spawn metadata.

Run the same diagnostic as implementation advances:

```sh
./build/th08_first_spell game_data_donottrack/th08.dat .cache/th08 reports/native
./build/practice_entry_tests game_data_donottrack/th08.dat
```

The [current report](../reports/native/first_spell_summary.json) is explicitly
`UNSUPPORTED_WORLD_EFFECT` at effect51, with supplied GUI gates and unknown RNG/player
state. The first empty timeline phase and the suspended immediate-ECL boundary are
recorded in [the trace](../reports/native/first_spell_trace.tsv). This is not a
per-frame world replay; all three acceptance gates remain unpassed. Successful
diagnostic process exit means the report was produced, not that the spell was solved.

No universal renderer, all-opcode engine or all-spell optimizer is a prerequisite.
However, a visual consumer cannot be projected out unless RNG, allocation,
lifetimes and gameplay-observable state remain unchanged. Measure complete verified
batches before adding further optimization.

## Expansion after the first route

After gate3, add capture, character/form choices, seeds and remaining difficulties
as separately verified cases; derive stage entry separately. Then choose the next
family by reachable dependency cost. The all-case requirements in
[Coverage](COVERAGE.md) remain the final scope, not a prerequisite for the first route.
