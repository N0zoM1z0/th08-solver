# Current handoff status

Updated 2026-10-27. Public scoring controls: `d100e92`, including the optional
October27 timeout-farm proposal; selected replay milestone:
`6b01b24`. The owner resumed Native score attack on 2026-10-26: download and study top human
replays, document source-backed insights, then test Extra and the complete
Lunatic route offline. Save tested progress directly on main. The preceding
handoff batch is finished; its artifacts and failures retain their identities.

Current verified Magic Team Native bests are **Extra 2,650,283,240, NMNB 14/14**,
and **Lunatic 1→2→3→4A→5→Final B: 4,444,083,080, NMNB 36/37**. The preserved
all-capture story improves to4,403,879,050,37/37. The next score gates
remain strictly above 3,021,639,920 and 6,168,365,790 respectively. The owner
subsequently made the constraints independent CLI controls. The latest scope is
**No Miss, No Bomb, captures optional**; bomb experiments are paused. Preserve
NMNB/all-capture records separately.
Optimize Native offline first; retail Wine replay validation waits for a
complete no-miss result above its human reference under the declared goal.
Online MPC remains paused. Stage4B is outside this route. Replay name is exactly
`ysopwnab`.

## Operational resume contract

Read this section, [verified results](#verified-results),
[unresolved work](#unresolved-problems-and-proposed-next-work) and the
[current command](VALIDATION.md#resume-extra-scoring-on-main) before launching.
Later sections preserve chronological/component evidence, not a second active plan.

| Requirement | Active contract |
| --- | --- |
| Workspace / publication | `/home/yann/yann/touhou/th08-solver`, direct `main`; fetch first and preserve others' work. English `gpt-6.1-sol-max: ...` commits and push tested progress |
| Concurrency | At most 4 owned solver/test/probe drivers, including replay verification. One driver may fork Native rollout children; do not start 4 solvers plus a fifth test driver. Build with `--parallel 2` |
| Campaign ledger | `reports/local/shadow-runtime-v1/tasks.tsv`, **415/640 allocated task IDs**. Append before launch; failed setup, rejected/partial/resource-aborted attempts and verification remain charged. Never reset or reuse IDs |
| Extensions | Further cumulative windows are owner-authorized under the same memory bounds; record an extension before using it. Spare CPU/RAM does not authorize higher concurrency |
| Memory | 2 GiB address space per solver process, 2 GiB summed RSS per owned tree. Admit only at 12 GiB host MemAvailable; abort owned work below 8 GiB or above the tree limit. Resource aborts cannot certify |
| Episode | Reuse the validated Extra46997 / story1805 receipts below. Capture a new original retail pre-RegisterChain seed before a new episode; never reroll after seeing score. Regression fixtures/historical seeds are separate |
| Objective | Proper entry, No Miss, No Bomb, higher complete terminal score. Current trials use `--allow-bombs 0 --require-all-captures 0 --verify-captures 0`; record actual bombs (must be zero) and capture outcomes. Restore `--require-all-captures 1 --verify-captures 1` for all captures |
| Checkpoint | Literal first N inputs from the same owned episode, executed from original entry. N counts Native owner updates, not spell seconds or replay-row line counts. Preserve RNG, bullets, actors, items, latch and observer history |
| Acceptance | Inspect each `exit.json`, `run/result.json` and `run/score-audit.json`. A driver exit 0 is not a candidate success. Exit 5 may be valid below-target progress; abort/incomplete/failed rollouts never certify |
| Replay gate | For selected improvements require original-entry fresh exact 41-word terminal agreement and natural original-writer score/no-miss/bomb-count/capture-outcome/name agreement under the declared constraints. This is execution agreement, not an independent physics oracle |
| Reproducibility | Freeze source, binary, exact args, seed receipt, DAT hash, prefix hash, budget, outcome and all additional costs. Keep stable tie order; label combined profiles and heuristic assumptions |
| Verification | Release timing; Native CPU excludes I/O. Relevant tests plus normal CTest before code publication. Freeze binaries during CTest; never relink concurrently. Documentation-only cleanup reruns the documentation test |
| Artifacts | New JSON/CSV/actions/replays stay in ignored `reports/local/`; publish only explicitly selected progress replays with exact provenance. Preparations, raw DAT/EXE/assets, original GIF bundle/generator and videos stay local |

The active local launcher is
`reports/local/retail-score-offline-20261026/run-batch.cjs`, with
`memory-guard.cjs` and `verify-runs.cjs` in the same directory. Use the current
guarded driver and fresh output names; do not use the older interrupted queue.
The driver verifies the seed receipt, freezes producer sources/binary/controls,
appends a task, applies `prlimit` and watches only its owned process group.
Do not terminate other agents' processes on this shared host. The ledger has
legacy repeated lifecycle rows: allocate the next ID from the maximum first
column, not TSV line count. The untracked owner file `image.png` predates this
work; leave it untouched and unstaged.

Public controls `--drop-target` and `--economic-proposals` default off.
`--economic-proposals 4` plus the recorded60879-input parent prefix reproduces
the selected2.620B full input tape and terminal in current public code. The
full-entry2.603B parent also uses private hit-window pulse/future-repair code;
main alone is not claimed to regenerate that entire parent. Recompile every
affected private translation unit against current headers/libraries after an
interface change; do not relink old `.o` files across `online.hpp` or rollout
ABI changes. Frozen standalone binaries remain the original run evidence.

Current local entry points, relative to
`reports/local/retail-score-offline-20261026/`:

| Evidence / producer | Directory |
| --- | --- |
| Selected full-entry Extra parent | `extra-drop-target-h256-v1/`; full72133-input tape, original replay and fresh/natural verification |
| Latest verified Extra | `oct27-extra-strong-focus-h128-v4/`; full72461-input tape,60879-input incumbent prefix, fresh/natural checks; bytes local |
| Latest verified capture-optional story | `oct27-story-58-prefix-safe-tail-h128-v3/`; full143157-input tape,59476-input timeout58 prefix; bytes local |
| Latest verified all-capture story | `oct27-story-incumbent58-h128-control-v4/`; full142845-input tape,58764-input original prefix; bytes local |
| Preserved highest tracked Extra | `extra-newbest-cash-compare-v1/`; full72557-input tape,60879-input parent prefix, frozen prototype and exact verification |
| Current public reproduction | `extra-main-cash-confirm-v2/`; same tape/terminal, final public source/binary hashes; `extra-main-cash-compare-v1/verification/` owns earlier public fresh/natural checks |
| Preserved highest tracked story | `story-retail-early-prefix-safe-tail-v3/`; full136061-input tape; original-entry early-graze prefix49753 plus conservative continuation; full verification |
| Source-faithful private repair / pulse | `future-repair-v3/`, `suppressed-hit-pulse-v1/period2/`, `drop-target-v1/`; frozen C++/compile/link inputs, not portable old object files |
| Economic prototype sources | `cash-compare-v1/` matches public32-update comparison; `cash-compare-pulse64-v2/` and `piv-potential-v3/` are non-winning variants |
| Read-only diagnostics | `point-income-probe-v7/`, `pool-pressure-probe-v8/`, `pool-transition-probe-v9/`, `chain-probe-v1/`; exact stock-row gates, source/CSV identities |
| Final batch, negative profiles | `extra-family-focus-{human,neutral}-margin-v1/`; source under `family-focus-v1/`; audited owner results, unselected and not freshly replay-promoted |
| Current full test suite | `oct27-timeout-farm-normal-v1/{stdout.log,exit.json,resources.txt}`:60/60 in1050.11s. Earlier deliberately interrupted suites stay recorded |

For every run, read `manifest.json`, `frozen-bin/`, `exit.json`,
`run/{result.json,score-audit.json,plan.actions,native.rpy}` when present, and
`verification/{verified.json,identities.json}` for selected successful exports.
A missing verification directory on a rejected profile does not make it a
verified result. [Validation](VALIDATION.md#selected-native-progress-replays)
owns the published file sizes, exact hashes and producer identities.

## Validated retail episodes

Extra seed10025 is a historical Native benchmark, not a fresh retail acquisition
under the current rule. Story seed40755 retains its historical Wine provenance;
do not relabel old experiments as new receipts.

The active episodes have validated original retail receipts, acquired before
any Native candidate in those episodes:

| Episode | Seed | Raw entry stage / difficulty / shot / prior FinalA |
| --- | ---: | --- |
| `extra-entry-v4` |46997 |8 /4 /1 /false |
| `story-4a-finalb-entry-v1` |1805 |0 /3 /1 /true |

Both use story=true, spell=-1 and dialogue advance. Each acquisition exits0
after one228-byte observed update, without a command producer or replay. The
initial three Extra startup attempts fail in menu automation before any valid
`start.txt`; they remain charged. The working driver reacquires the owned game
window after Wine startup replaces it and waits for menu readiness. Receipts,
frozen launcher/observer, entry configurations and startup logs remain local in
`reports/local/retail-seed-episodes-20261026/`; [Validation](VALIDATION.md#native-score-first-workflow)
owns their exact receipt identities. Use these captured seeds for the next
candidate batches, keeping each fixed across strategy changes and continuations.

## Verified results

All scores are displayed points. These are Marisa/Alice (shot 1); Extra uses
proper non-practice entry, difficulty 4, story=true, no prior-Final-A history.
In this verified-results table, "complete" requires NMNB, the declared capture
goal, exact fresh41-word terminal agreement and natural original-writer replay
with matching score/capture outcomes. Capture totals remain explicit.
Fresh replay checks execution agreement, not independent physics or retail equivalence.

| Evidence | Score / captures | Scope |
| --- | ---: | --- |
| **Current Native Extra** | **2,650,283,240 /14 of14** | Seed46997,60879-input incumbent prefix, H128/focus10 late continuation; new bytes local |
| **Current Native Lunatic4A→FinalB** | **4,444,083,080 /36 of37** | Seed1805, isolated58-timeout prefix59476 and H128 capture continuation; only58 times out; new bytes local |
| **Current all-capture Native story** | **4,403,879,050 /37 of37** | Seed1805, original captured58 prefix58764, same H128 tail; new bytes local |
| Preserved Native Extra |2,620,568,090 /14 of14 | Captured retail seed46997, actual drop-target prefix and bounded Native cash comparison; selected replay |
| Selected full-entry retail-seeded Extra |2,603,278,620 /14 of 14 | Seed46997, source drop-target/hit-window profile; selected parent replay |
| Earlier Native Extra benchmark |2,589,417,330 /14 of 14 | Historical seed10025, complete from entry,500ms offline search; original bytes preserved |
| **Preserved all-capture Native story** | **4,311,260,760 /37 of37** | Original retail seed1805, early-graze prefix plus audited conservative continuation; selected replay |
| Selected retail-seeded full-entry story |4,160,457,200 /37 of 37 | Seed1805, audited Native future-repair v3; earlier bytes preserved |
| Selected conservative story continuation |4,126,102,910 /37 of 37 | Seed40755, offline checkpoint continuation; earlier bytes preserved |
| Selected survival-priority story |3,903,603,650 /37 of 37 | Seed40755, source-survival form priority and bias5; earlier bytes preserved |
| Native story price baseline |3,890,487,560 /37 of 37 | Seed40755, complete from entry,500ms offline search; selected earlier progress replay |
| Selected Native v15 snapshot | 2,564,311,730 /14 of 14 | Seed19732, frozen prototype; original bytes preserved |
| Main price/staging controls | 2,453,582,120 /14 of 14 | Seed 10025, complete from entry, reserve 1.2/120 |
| Main successful prefix + expert recovery | 2,464,473,300 /14 of 14 | Seed 19732, separately charged offline checkpoint continuation |
| Main rerun with v15-style settings | 1,683,795,810 partial /13 of 14 | Seed 19732, final-card hit at 65137; failed NMNB |
| Actual online Extra PB | 1,166,969,620 /14 of 14 | wine-35, real seed 19732, frozen H64 client |
| Actual online Lunatic 4A→FinalB | 2,189,965,070 /33 of 37 | Recorded story, real seed 40755; missing captures remain known |

The [selected v15 replay](../reports/native/replays/extra-native-nmnb-2564311730-seed19732.rpy)
is 26949 bytes, SHA-256
`2bf469a6cbe59732f53b2a7b2ef208d3b450fc272e4c9c172eff244502b0c2d4`.
It records the frozen v15 prototype, not a promise that current-main searches
regenerate the same tape. [Validation](VALIDATION.md#selected-extra-score-replay)
owns its exact input/binary provenance, verification commands and retail limits.

The human reference is the 2026-10-25 ASL snapshot: 3,021,639,920, Magic Team,
14/14, no misses and **one bomb**. Its replay reproduces that score exactly in
Native. The historical v15 NMNB score is84.86% of it; the reference is not a matching
NMNB upper bound. Human inputs were measured, never copied into our controller.
The source links and dated scoreboard snapshot are in [README](../README.md).

## October27 Native continuation

The owner resumed No Miss/No Bomb scoring with captures optional. Extra46997
and story1805 retain their original retail receipts. Task395 passes2/2 focused
policy/contract tests in0.97s. The new default-off Native `--farm-to-timeout 1`
keeps only a positive sampled ordinary farm past its capture reserve, refreshes
realized income every240 updates and restores fire if income disappears. It
retains the existing heuristic bonus proxy; full-terminal score remains the
acceptance criterion. Strict capture CLI goals reject the option.

Tasks396/397 are the first named combined profiles: H512/publication256/lead1,
replan128,30s search cap, beam384, current drop/hit goals,1.5/240 capture reserve
and goal-preserving recovery. They reconstruct5400 Extra or33000 story inputs
from the selected incumbent before applying the new preference. Immutable
binary/sources, receipts, prefixes, DAT/argument identities and memory guards
are frozen in `oct27-{extra,story}-timeout-income-v1/` under the existing local
campaign directory. Both fail NMNB: Extra at49318/card202 with1,114,545,830 partial
score,0 bombs,8 captures; story at74326/card70 with1,196,955,920 partial,0 bombs,
12 captures. Ordinary timeouts are196/198 and36/43/47/54/58/62 respectively.
They charge224072+7 /240580+22 branch/proposal updates and187.603s/192.208s;
neither has rollout timeouts or resource aborts. These are failures, not scores.

Broad timeout farming loses cash, capture rewards and point opportunities.
At the end of story stage3, the trial has PIV515800 versus498580 but only
790,926,970 cash versus909,002,910 and298 points versus313. Extra's two ordinary
timeouts are followed by a90-point deficit from the corresponding captured
attacks; new field-item timing can add other pickup losses. Source timer callbacks
also use `DetachEnemyChain(0)`, unlike rewarded chain termination. No allocator
or scoring rule is changed to erase these costs.

Task399 isolates the incumbent prefix up to56276 before card58; it times out58/62
then fails in4A. Card58's interval has10505 orbs versus4622. At card62 start it
has PIV575370 versus547770, cash1,132,642,200 versus1,148,375,220, and168
stage points versus183. These are actual checkpoint economics, not a guaranteed
future profit. Tasks401/402 retain only its first59476 safe inputs, disable
timeout farming, and test H512/publication256/interval128/factor2/padding480 or
H128/publication64/interval32/factor1.5/padding240 capture-fire continuations.
Both preserve the entire carried world by reconstructing from original entry.
The original failed parent remains charged. Task400 separately starts from the
incumbent Extra31814-input prefix and tests factor1/padding0 with four bounded
cash alternatives, without forcing timeout; it fails at51318,0 bombs,8 captures,
1,186,091,310 partial points,231.274s. Cards198/199 time out. Task401 completes
H512 at4,181,877,290,0 bombs,36/37 in278.040s. Task402's H128 continuation reaches
**4,444,083,080,0 misses,0 bombs,36/37**, improving132,822,320 over the preserved
all-capture best. Only58 times out. It costs335.835s,346664 branch+9 proposal
updates and232 rejected future deaths, no rollout timeout.

Task403 retains the original60879 Extra inputs and tests a named H256/
publication128/interval64, focus/survival bias10,1.2/120 reserve, four-cash-proposal
profile. It reaches **2,624,965,020,0 misses,0 bombs,14/14**, a4,396,930 gain,
at72597 inputs:107.637s,127219 branch+2 proposal updates,276 rejected future
deaths and no timeout. Its terminal has1015 points/91692 orbs/PIV758460.
The new story collects1321 large points, versus1340 in the old best;
its FinalB entry PIV762670 and terminal PIV1001230 exceed the old699610/918390.
These are combined strategy/controller profiles, not isolated component gains.

Tasks404–406/409 verify both new bests from original entry: fresh exact41-word
terminal, natural original-writer score, zero misses/bombs, exact capture
outcomes and name agree. Exact bytes remain under ignored
`reports/local/score-continue-20261027/selected/`; [Validation](VALIDATION.md#october27-verified-native-progress)
owns hashes and lineage. Earlier selected replays remain unchanged. Both human
reference gates remain unmet; no Wine playback is started.

Task398 passes the normal Release Native/DAT suite60/60 in1050.11s. Tasks407/408
are separately charged follow-ups: an H128 story tail from the original captured58
prefix58764, and an H128 Extra late-tail profile, to measure controller/prefix
alternatives without crediting only the winning targets. Both complete NMNB:

| Follow-up | Score / captures | Solve seconds / branch+proposal updates / rejected deaths |
| --- | ---: | ---: |
| Story captured58-prefix H128,407 |4,403,879,050 /37 of37 |340.457 /354561+18 /396 |
| Extra H128 focus10 late tail,408 |2,650,283,240 /14 of14 |104.441 /124279+2 /562 |

Both have zero bombs and zero rollout timeouts. The all-capture story improves
92,618,290 over the old4.311B snapshot; the timeout58-prefix version finishes
40,204,030 above that phase-aligned H128 alternative. This conditional full-route
comparison includes changed RNG, geometry, items and later controller inputs;
it is not a universal per-card valuation or permission to transplant timed tails.
The final Extra improves29,715,150 over the old2.620B, with unchanged1015 collected
points but93708 orbs/PIV768540. The H256 intermediate remains local and unchanged.
Tasks411–414 freshly/naturally verify both follow-ups with exact terminal,
score, no-miss/no-bomb, name and complete capture-outcome agreement.
All published historical replay bytes remain unchanged; new files stay local.
The complete batch has finished; no owned solver, test or queue remains running.
Neither human score gate is met, so Wine playback remains unlaunched.

## Human replay study, 2026-10-26

A fresh [world-record table](https://maribelhearn.com/wr#IN) snapshot still lists
Magic Team ASL3,021,639,920 and Alan6,168,365,790. Eight original replay files
were downloaded into ignored `reports/local/human-score-research-20261026/`.
`cases.json` binds URLs, timestamps and SHA-256; each stock run records its
frozen checker/DAT hashes, resource guard, exact rows and source score audit.
Tasks357–364 are read-only evidence, not new scoring episodes or seed selection.
Scores below are actual natural Native playback, not inferred from filenames.

| Player / source | Native displayed score | Bombs / captures | Collected large points / stage-orb sum |
| --- | ---: | ---: | ---: |
| [ASL](https://maribelhearn.com/media/replays/th8_udExMT.rpy), Extra WR |3,021,639,920 |1 /14 |1081 /105531 |
| [yeswill](https://maribelhearn.com/replays/royalflare/th08/th8_ud0a27.rpy), Extra |2,741,980,300 |4 /14 |1083 /83415 |
| [A.War](https://maribelhearn.com/replays/royalflare/th08/th8_ud29dc.rpy), Extra |2,633,042,840 |3 /14 |1080 /73573 |
| [Alan](https://maribelhearn.com/media/replays/th8_udLuMT.rpy), Lunatic FinalB WR |6,168,365,790 |8 /33 |1660 /208581 |
| [Clio](https://maribelhearn.com/replays/royalflare/th08/th8_ud29d6.rpy), Lunatic FinalB |6,005,139,940 |12 /32 |1628 /204074 |
| [Mokou Sheep](https://maribelhearn.com/replays/royalflare/th08/th8_ud2bbe.rpy), Lunatic FinalB |5,675,333,250 |11 /30 |1611 /187363 |
| [less](https://maribelhearn.com/replays/royalflare/th08/th8_ud285a.rpy), Lunatic FinalB |5,382,503,000 |7 /31 |1640 /170443 |

The RF Extra LET2,556,875,270 file is retained but excluded from strategy
comparisons: Native naturally terminates at940,494,920 with5 captures, unlike
its advertised result. Its SHA-256 is
`14ae0d73479b816a5cf6088c0f7721743ef4e524725059dbfcfb7af84f49b417`.
No cause or retail mismatch is established. Death-row counts are observations
of dying/dead state, not numbers of misses. Human records are unrestricted
score references; none of these seven reproduced files is an NMNB certificate.

ASL waits3349/3591/3395 updates on cards192/193/194 and5376/4180 on198/199,
but kills195/196/197/200/201/202 in824/1648/1249/1162/1172/1032. Our2.620B
tape instead spends1975/4079/2388/3649/3988/3651 on those latter six. Card199
collects16998 interval orbs for ASL versus10376 for us; conversely our198
collects16233 versus11455. Literal lifecycle intervals include carried items
and unsettled rewards, so these differences are timing evidence, not causal
per-card profit or a safe schedule to copy across seeds.

The complete-route gap starts early: after stage3, Alan/Clio have PIV761140/
756770; ours498580. Their point totals559/551 versus ours515 alone cannot
explain the gap. At FinalB entry Alan carries PIV1097240 versus699610; at the
end he has450 stage points versus310. Our FinalB43756 orbs are already near
his49944. This prioritizes earlier orb generation and actual point production,
not only the final five timeout cards. Human tapes remain descriptive evidence,
not controller action tables. Action-dependent RNG prevents treating their timed
routes as interchangeable tails, even under the relaxed capture goal.

Source implications: `Item::CollectTimeOrb` gives cash, PIV through
`AddTimeOrbs`, and capped `AddBonusProgress(8000)`; the latter also raises
future decay below the cap. `Spellcard::EndSpell/OnUpdateImpl` defer capture
cash and direct orb/PIV rewards by130 updates. A cash-only short rollout can
therefore compare opposite sides of settlement. Ordinary capture orbs also
fall with remaining time. The existing pacing prior's flat8000/orb is not a
source valuation once capped, frozen or decaying. A bounded experiment should
compare selective farming/capture and pending settlement explicitly, then accept
only better complete no-miss score under the declared constraints from the fixed
retail entry. This insight is not an implemented valuation change.

Tasks373–381 add a read-only `ItemManager` request/spawn/pickup probe for the seven
reproduced human tapes and both current Native bests. Every probe's complete
25-word row stream agrees exactly with its stock checker stream. The frozen
source/binary, CSV, row hashes and resource costs stay in the local research
directory. Actual large-point income is:

| Tape | Requested / spawned / collected points | Point income | Static same-item price headroom |
| --- | ---: | ---: | ---: |
| ASL Extra |1130 /1084 /1081 |1,062,330,020 |0 |
| yeswill Extra |1129 /1083 /1083 |978,369,580 |1,342,560 |
| A.War Extra |1081 /1080 /1080 |941,061,780 |9,619,700 |
| Current Native Extra |1042 /1041 /1015 |825,841,790 |55,681,290 |
| Alan story |1662 /1662 /1660 |2,698,355,080 |82,610,980 |
| Clio story |1631 /1631 /1628 |2,636,087,670 |40,710,110 |
| Mokou Sheep story |1631 /1614 /1611 |2,504,782,800 |71,424,000 |
| less story |1640 /1640 /1640 |2,335,441,590 |87,518,710 |
| Current Native story |1368 /1368 /1340 |1,263,269,100 |192,755,140 |

Headroom holds the same collected items and their current PIV fixed; it is not
an attainable trajectory gain. All1081 ASL pickups are maximum-price extreme-
human pickups. Alan misses only2 spawned points; our story misses28, but294
fewer requested points and lower earlier PIV are the larger measured differences.
Source allocation/bounds failures remain failures; no allocator is changed.

### Parameterized relaxed scoring investigation

The owner requested independent bomb/capture controls, then paused bomb
experiments because of their additional variables. Current trials retain No
Miss and No Bomb with captures optional. Both offline tools expose `--allow-bombs 0|1` and
`--require-all-captures 0|1`. The refiner retains strict defaults; the planner's
existing capture option and campaign specs remain explicit. Live clients retain
their default bomb-input rejection. The original seed receipts and NMNB progress
replays are unchanged.

The human bomb trace makes the comparison more specific: ASL's only X window
starts at calculation18349, in the post-Keine stage section, outside a spell.
Alan uses three X windows during card47 at49050/49437/49817. This suggests testing
stage production and deliberate lost-capture farming as distinct opportunities.
Input-clock offsets are never transplanted between seeds. New adaptive tests
inject a declared bomb into our own episode prefix, then replan the entire tail.

Tasks366/367 first tested full-terminal bomb edits without adapting the timed
tail. Extra tries70 edits:66 die and4 finish with unchanged score and no actual
bomb activation. Story tries80:74 die and6 finish unchanged with no activation.
No edit is accepted; incumbents remain2,620,568,090 (0 bombs,14/14) and
4,311,260,760 (0 bombs,37/37). Costs are10.7873s and46.5016s respectively, with
full charged branch work in `*-bomb-terminal-refine-v1/run/result.json`. This
reproduces the timing constraint: inserting a bomb into a successful action tape
does not supply a safe continuation.

Tasks368/369 are charged setup failures: zero capture-reserve factor is rejected
by the existing policy's minimum1 guard before any owner update. No gameplay
result is inferred. Tasks370/371 use supported factor1, zero padding and one
human-form bomb proposal in our own prefixes (Extra index16274; story index44796,
within its card47), followed by fresh Native planning. Extra fails at54430,
card202, after5 actual bombs and7 captures:1,118,686,830 partial score. It costs
407.046s and489768 branch updates, no rollout timeout. This is not a completed
score. Story and tasks382/383 were stopped on the owner's scope change; they
certify no gameplay result. Their charged wall times are731.095s/224.155s/224.062s.
Tasks382/383 used a named combined profile:
focused bomb at the same own-prefix boundary, H1024/publication512, factor2/
padding480 and capture-fire recovery. Tasks384/385 use the original no-bomb
prefixes, the same conservative profile and optional captures. Extra fails at
46526/card202,0 bombs,10 captures and1,309,226,090 partial points:379.239s,
472731 branch+33 proposal updates, no timeout. Task386 returns to the certified
60879-input Extra prefix and bounded cash comparison. Task387 tests96 longer
fire-release edits with the full-terminal story oracle; all96 edited tails die,
so none is accepted. Its retained complete story remains4,311,260,760,0 bombs,
37/37; cost26.8441s and365012 charged branch updates, no timeout. Relaxing capture
coverage alone does not repair the action/RNG timing of those edited tails.
New artifacts stay local.

The completed no-bomb batch retains the earlier bests; it does not beat either
human reference. All four following runs explicitly allow missing captures but
forbid bombs. Costs include rejected futures and initial incumbent verification
where applicable; no trial is selected merely for finishing faster.

| Local run / task | Terminal result / bombs / captures | Wall seconds / charged branch + proposal updates |
| --- | --- | --- |
| `extra-no-bomb-optional-captures-h1024-v1`,384 | Failed card202 at46526;1,309,226,090 partial /0 /10 |379.239 /472731+33 |
| `story-no-bomb-optional-captures-h1024-v1`,385 | Complete4,035,533,800 /0 /37 of37 |702.288 /840881+21 |
| `extra-no-bomb-optional-captures-economic-tail-v1`,386 | Complete2,620,568,090 /0 /14 of14 |117.186 /138928+6 |
| `story-no-bomb-optional-captures-fire-refine-v1`,387 | Retained4,311,260,760 /0 /37 of37;0 of96 edits accepted |26.8441 /365012+0 |

The Extra economic continuation has exactly the incumbent's full72557 input
tape and41-word terminal. Its newly written replay bytes differ and stay local;
the selected historical replay is unchanged. Tasks390–393
freshly execute Extra/story from their original retail entries and naturally
play their exports: both complete, match exact41-word terminals, displayed
scores, zero bombs, all capture outcomes and `ysopwnab`. This is Native execution
agreement, not independent retail equivalence. No Wine playback is launched.

The conservative story loses earlier income: stage3 PIV484030 versus the
incumbent498580, terminal PIV863360 versus918390, and1306 collected points versus
1340. Capture coverage was optional in search, but this profile still chooses
all37 captures. The
relaxed acceptance gate alone supplies no new farming policy; longer verification
and changed pacing/recovery are not measured scoring gains. Next work should
target the earlier source income/production bottleneck with adaptive continuations,
keeping the certified incumbent. This batch and its verification are finished;
no owned solver, queue or test driver remains running.

Focused parameter verification passes3/3 in198.62s, including its incumbent
fixture. It proves actual source bomb consumption and lost capture, complete
no-miss execution, fresh exact41-word agreement, natural replay score/bomb/
capture agreement, and independent rejection when either constraint is restored.
The normal Release Native/DAT suite passes60/60 in1133.30s (task372). A subsequent
report-only correction also rejects a dying owner before its death counter has
incremented. Final focused verification passes3/3 in26.15s (task388), reusing the
completed normal suite's immutable incumbent fixture. It covers that failure,
the actual bomb/capture scene, default input rejection and documentation.

## Native score-first evidence (chronological)

The selected Native progress replays are complete from entry, with exact fresh41-word
terminal agreement, natural replay NMNB/all-capture checks and the selected name
`ysopwnab`. [Validation](VALIDATION.md#selected-native-progress-replays) owns their
identities and commands. Both remain below their explicit human-reference target;
exit5 and `objective_met:false` distinguish valid progress from meeting that goal.
No retail Wine playback is started for them yet. The source-survival priority
follow-up reaches3,903,603,650; the subsequent offline continuation reaches
4,126,102,910. The new retail-seeded full-entry story reaches4,160,457,200;
its follow-up continuation reaches4,311,260,760,37/37, NMNB. Earlier selected
replay bytes remain unchanged.

The baseline common profile is Magic Team, SampledGameplay, score-harvest, held/baseline
recovery,500ms total/60ms proposal, priced collection, upper-stage positioning,
collection/human preferences5, original8-update lead. Extra seed10025 uses
H256/publication128 and pacing with1.5/240 reserve. Story seed40755 uses
H128/publication64 without ordinary pacing, prior-Final-A-clear=true, ending at7.
These are named combined profiles, not component ablations or new holdouts.

| Run | Owner updates / solve seconds | Branch+proposal updates | Rejected deaths / timeouts / overruns |
| --- | --- | --- | --- |
| Extra native baseline v1 |69381 /448.547 |2239860+120 |277 /8 /8 |
| Story native price v1 |119637 /479.403 |1975206+2016 |538 /0 /0 |
| Story source-survival priority v2 |119637 /515.804 |1975206+2016 |538 /0 /0 |
| Story conservative checkpoint continuation |128013 /215.523 |305829+16 |387 /0 /0 |

The Extra baseline finishes with984 points/86136 orbs. The story price baseline finishes with1265 points,
100100 collected stage-orb inventory total, PIV800510 and clock5; all37 captures.
Its stage point counts are61/147/299/326/202/230. The first story fresh-follow
invocation omitted prior-Final-A history and entered6A; it is retained as a
verification configuration failure. The corrected true-history follow agrees
exactly. Reproduction must preserve that history, not only seed and inputs.

The reviewed local research package is
`preparations/10.26-gpt/TH08_Scoring_Research_2026-10-26.zip`, SHA-256
`93ff9119decc6e0132e894a06b078ef5c1c829aa0cc782577e04fbfe77577b40`.
All141 manifest entries verify; originals and the full package remain local.
Its source references, instrumented ledgers and negative controls support:

- Extra v15 generated1011 large points and collected1010. The missing production
  and early PIV schedule matter more than chasing an assumed71 dropped items.
- The historical online story generated1254 large points and collected623;
  collection/price is a different bottleneck from Extra. The new Native result
  already changes that baseline; old static losses are not additional gains.
- `AddBonusProgress` cap, counter and freeze gates invalidate flat8000 valuation.
  Capture-born items and pending payout must not be counted before they exist or
  counted again after cash settlement. Ring-slot failures are real and cannot be
  repaired by changing the game's allocator.
- First-safe return and full nominal-tail reuse can postpone fresh economic
  decisions for nearly an entire horizon. This motivated retaining a certified
  incumbent and comparing fresh economic alternatives, now available through
  the optional bounded cash-comparison control.
  Longer offline budgets and checkpoint continuations are available; their work
  remains charged and final tapes must execute freshly from original entry.
- The new story's five final timeout cards have1642 physical graze events, all
  extreme-human and none extreme-youkai. Check whether the economic human prior
  suppresses the explicit survival-form goal before extending riskier farming.

Package synthetic formula examples are not integrated code or verified gains.
The source-survival priority bug is fixed: outside flow/pacing, the economic
human prior no longer overrides an active source survival-form goal. The
score-harvest regression covers this independently of macro flow. With the
named bias5 profile, the five final timeout cards now have1771 physical graze
events,1695 at extreme-youkai versus0 in the earlier tape. Actual complete
story score improves13,116,090. This combined profile is not a bias ablation;
individual card intervals still include carried items and delayed settlement.

`th08_score_refine` implements the complete-terminal comparison proposed by the
research review. It evaluates the entire remaining true-RNG Native tape before
accepting any edit, preserving NMNB, the exact capture catalogue and final stage.
The first Extra form/fire passes try59/46 candidates and accept0. Form has58
deaths and1 equal-score completion; fire has43 deaths,2 incomplete tails and1
equal-score completion. All costs remain local: form7.97808s/79499 charged
updates, fire12.3233s/168738, each including the initial69381-update incumbent
verification. That2,589,417,330 historical snapshot remains unchanged. This exposes
the next constraint: changing a form/kill can invalidate the old timed tail;
safe adaptive continuation is needed before claiming a scoring benefit.

Follow-up source-economic profiles all complete Native NMNB,14/14, but do not
beat the incumbent. They use the same predeclared seed10025,500ms budget and
whole-entry acceptance, not a component ablation or held-out result:

| Named profile | Score / owner updates / solve seconds | Branch+proposal updates / rejected deaths / timeouts |
| --- | --- | --- |
| Human bias0 between priced collections |2,313,546,260 /65677 /481.653 |2206349+704 /2104 /68 |
| Current reduction-target hit income |2,496,594,790 /68221 /475.148 |2217214+224 /551 /19 |
| No ordinary pacing |2,137,930,300 /47229 /314.593 |1564684+312 /667 /30 |

A private paired adapter/policy tests source current bonus headroom plus nominal
clear-orb value instead of flat8000. It completes2,589,417,330,14/14 in448.751s,
2240019+120 branch/proposal updates,291 rejected future deaths and7 timeouts.
The full69381-input tape is byte-identical to the incumbent (SHA-256
`4997feed5ecfcb523e383ac9db8eedd767b79ca291de5af5d2487106aff2dcbc`):
this valuation correction alone changes no selected input and proves no gain.
The current pacer samples once, then stays in Farm until its capture reserve;
rolling re-evaluation and adaptive continuations remain proposed improvements.
Extra point extends are200/666/9999 in the source, so984→1000 is not an extra-life
threshold to exploit. This differs from the normal-story extend table.
All manifests/tapes/costs remain in
`reports/local/native-score-offline-20261026/`.

The uploaded Extra replay is byte-identical to the selected file. Its `nan%`
slowdown is the original writer's0/0 with no headless draw/FPS samples, not an
online frame-rate result. The checker preserves the metadata bits and reports
unavailable slowdown as JSON null. [Validation](VALIDATION.md#slowdown-metadata-in-native-exports)
owns the source explanation and comparison. No replay bytes are rewritten.

The next owner instruction explicitly prioritizes score over solve speed.
Native continuation now imports a literal input prefix while reconstructing
actual observation history; new explicit controls allow larger horizons/budgets,
shorter nominal reuse and larger certified execution blocks. Defaults and live
Wine behavior remain unchanged. The focused checkpoint regression executes the
original prefix, regenerates its tail, checks fresh exact terminal/natural replay
and rejects an overlong terminal prefix; invalid schedules are rejected early.

Three named H512/publication256 profiles use5000ms per search, lead1/nominal1,
interval128 and7200s total: Extra baseline, a private all-speed continuous-stream
proposal, and full story with ordinary pacing/bias5. All fail NMNB: Extra baseline
card202 at50325, partial1,331,016,900,10/14,162.107s/247020+8 branch/proposal
updates/321 future deaths/1 timeout; swept Extra card202 at50323,
partial1,282,440,870,10/14,167.321s/247186+12/480 deaths/0 timeouts; story
Stage4A card70 at71326, partial1,322,251,780,18/37,
183.982s/308586+7/303 deaths/0 timeouts. Baseline reaches5004.97ms once;
swept/story maxima4071.62/1883.45ms remain below the5s search allowance.
These are named profiles, not gains. The private stream variant uses only its geometry change, without
the earlier failed gauge-warm experiment. Initial tasks245–247 fail before
initialization because the launcher writes the wall argument into the first
control slot; exits20–25ms and manifests remain intact. Corrected tasks248–250
rebuild the argument list from named controls. No failed work is hidden.

The private future-repair experiment uses failed predictions only as proposal
inputs, intervenes before a predicted collision and verifies each complete
horizon again from the unchanged actual origin. It tries at most128 additional
rollouts per search under a30s allowance. From the selected Extra tape's midpoint
spell checkpoint38824, it completes2,480,556,950,14/14, NMNB:68349 owner updates,
257.885s,523376+28 branch/proposal updates,2130 future deaths,0 timeouts,
986 repair attempts/12 adoptions. This restores completion for the named long
profile but does not beat the selected score; it is not promoted.

The story counterpart from the selected tape midpoint59818 completes NMNB at
4,071,061,590 but36/37: ordinary Stage4A card74 times out. It charges134125
owner updates/300.741s,395896+20 branch/proposal updates,748 future deaths,
67 repair attempts/6 adoptions and0 timeouts. The capture gate rejects export;
this is not a new best. Those safety-only horizons do not reject a future
ordinary capture failure, motivating the explicit capture-aware goal below.

The wider H48/beam512 guidance prototype stops with `native_style_proposal_fault`
after5.790s. Further source inspection locates the first rejection in the fixed
24-update learned-feature proposal API; the four23-input transport is a related
limit. A longer ordinary policy horizon is not a longer learned-feature path.
Its manifest/error/wall cost remain; the CLI throws before
writing detailed work counters, which must not be reported as zero. A compatible
wider adapter is now tested; that prototype result remains a failure.

The optional capture-aware rollout goal and wider ordinary-decision adapter are
implemented and pass the normal Release Native/DAT suite59/59 in919.31s. Their
first private story attempts falsely reject the Stage1→2 control
handoff at11426: currentStage is already1 while the prior clear flag persists,
so the new unfinished catalogue is not yet a failed goal. The correction checks
the stage that just ended, shares that boundary logic across owner/rollouts and
adds a selected-story lifecycle regression with the recording clear flag
projected explicitly. The original attempts and focused failures stay local.
The timeout regression uses explicit synthetic invulnerability, rather than
claiming that fixture as a playable NMNB route. Wider ordinary guidance keeps
the learned24-update schema intact. Neither addition has a verified score gain
yet; the selected replay below comes from the earlier frozen prototype.

While that boundary is checked, a generic reserve/fire portfolio uses the frozen
earlier binary. It retains61710 actual inputs before the first failed encounter
of the4,071,061,590 tape, reconstructing the world and observer history from
entry. Reserve2*HP/DPS+480, H512/publication256, lead1/nominal1/interval128,
30s search/7200s owner and capture-fire recovery complete **4,126,102,910,
NMNB,37/37**. Only5 future-repair attempts/1 adoption are needed. Fresh exact
terminal and natural original-writer replay checks pass; no state override or
human input is used in the scored run. This combined offline continuation adds
222,499,260 over the previous selected story, not a component ablation.
Stage points are61/147/299/326/194/258; stage orbs6020/12868/14940/20698/17254/41540,
final PIV866600, clock5. [Validation](VALIDATION.md#selected-native-progress-replays)
owns its exact producer and prefix lineage.

The companion no-pacing tail completes3,880,310,940,37/37 in155.912s,
119549 owner/251357+6 branch/proposal updates, with no future repairs. Extra
reserve1.2/120 with the existing H256/full-tail profile completes2,483,229,780,
14/14 in492.130s,69077 owner updates and8 repair attempts/2 adoptions, below
the selected Extra score. These alternatives are retained, not promoted.

The seed-acquisition checkpoint ends at266/320. The active campaign below
continues the same ledger; concurrency remains at most4.

## Retail-seeded experiment evidence (chronological)

All candidates bind the original Extra46997/story1805 receipts above before
launch. The first full-entry main profiles complete NMNB,14/14 and37/37.
Both pass fresh exact41-word terminal execution and natural original-writer
replay, including score, capture catalogue and `ysopwnab`. Their outputs remain
local under `reports/local/retail-score-offline-20261026/`.

The private future-repair v3 is recompiled against current audited Native
libraries, retaining the shared corrected stage-handoff check. Failed futures
remain proposal material, with at most128 additional full-horizon attempts per
search. H512/publication256, lead1/nominal1/interval128, beam384,30s search and
reserve2/480 are explicit combined profiles, not component ablations. They
complete both episodes. Story improves332,831,920 over its same-seed baseline
and34,354,290 over the earlier selected story. Its new4,160,457,200 replay passes
fresh/natural checks and is selected for publication. Extra does not improve.

| Named complete profile | Score / captures | Owner updates / seconds | Branch+proposal updates | Rejected deaths / timeouts / overruns |
| --- | --- | --- | --- | --- |
| Extra retail baseline |2,384,787,210 /14 |69629 /525.503 |2275692+304 |703 /28 /28 |
| Story retail baseline |3,827,625,280 /37 |118109 /545.516 |1923219+1048 |327 /1 /1 |
| Extra long future repair v3 |2,301,642,690 /14 |58909 /366.169 |552241+27 |1869 /0 /0 |
| Story long future repair v3 |**4,160,457,200 /37** |124549 /543.230 |624148+15 |790 /0 /0 |
| Extra human-focus period2 |2,179,267,180 /14 |60869 /394.031 |487927+23 |1342 /0 /0 |
| Story human-focus period2 |3,812,879,650 /37 |126693 /624.708 |786697+19 |1426 /0 /0 |
| Extra warm/rolling farm |2,181,756,540 /14 |52821 /286.459 |493360+26 |1438 /0 /0 |

Extra/Story long repair charge874/319 repair attempts and8/4 adoptions. The
human-focus profiles charge447/645 attempts and7/7 adoptions; warm/rolling
charges556/9. No profile above rejects a future capture event. These costs
include failed proposals, and complete scores remain below the human targets.
The selected story's stage points are61/146/306/328/192/278; stage orbs
6403/12427/15020/23027/15388/42304; final PIV872860 and clock5.

Source `Player.cpp` separates immediate shot-file selection, the7-update stable
form threshold and the30-update gauge-ramp threshold. A named private proposal
tests alternating focus while already extreme-human and human during ordinary
boss phases; it changes inputs only. The first period2 profiles complete but
do not improve score, so no scoring alpha or public policy change is claimed.
The compiled period7 variant has not run at this checkpoint.

The warm/rolling candidate uses focused fire to reach extreme-youkai before
withholding fire, then rechecks realized farm income every240 updates. It also
completes below baseline. Current evidence motivates an off-axis charging
proposal that preserves pre-charge mature DPS rather than treating deliberately
missed shots as slow capture damage. Full-board targets/all-speed stream
proposals and off-axis variants were unfinished at that publication checkpoint;
their completed outcomes follow below.
All alter proposals, not Native game behavior; no seed/card or human-input
scripts are used. That first publication checkpoint ends at283/320.

The restricted hit-window focus candidate improves the Extra46997 incumbent to
**2,493,554,650, NMNB,14/14**, from2,384,787,210. It uses period2 focus changes
only while actual source HP suppression/current nonboss hit-income targets are
present, with `--hit-income 1`, H256/publication128 and3000ms. This is a named
combined profile, not evidence attributing the gain to focus alone. It has925
points/90541 orbs/PIV752710, and passes exact fresh/natural replay. The run
charges72997 owner updates/585.409s,2391293+168 branch/proposal updates,
605 rejected deaths,0 timeouts,1 rejected future capture failure and52 repair
attempts/5 adoptions. It remains below the selected historical Extra high score.
Precollection alone completes2,438,900,780,14/14; combining it with period2/7
profiles gives2,258,162,220/2,279,504,160, so the gains are not additive.

Read-only policy logging over the full off-axis tape preserves its exact41-word
terminal and explains its weak result: focused charging can deplete HP before
extreme-youkai. Source `Player::AwardGraze` adds100 gauge in youkai form;
`Item::CollectTimeOrb` adds111 with focused pickup, so no-fire grazing can establish
the gauge through actual interactions. Earlier no-fire sampling removes the
16-slow-nearby-bullet gate while retaining mature DPS, the capture reserve and
Native safety. Its H1024 full attempts fail later: Extra at48656/card202,
1,101,464,970 partial/10 captures,269.713s/397592+11 branch/proposal updates;
story at71183 in4A,1,342,161,530 partial/18 captures,408.359s/594588+11.
They charge569/541 repairs with3/2 adoptions,1038/941 rejected deaths and
0 timeouts; story rejects2 future capture failures. Neither exports a replay.

The failed story's first49753 actual inputs cover three captured stages and
raise carried PIV to498580 versus469260 in the earlier selected story. A new
candidate reconstructs those inputs from original entry, then runs the known
conservative generic policy with the same H512/beam384/30s/reserve2/480 profile.
It completes **4,311,260,760, NMNB,37/37**, with fresh exact terminal and natural
replay agreement. This adds150,803,560 over4,160,457,200. The completed attempt
charges136061 owner updates/440.974s,452782+15 branch/proposal updates,
654 rejected deaths,0 timeouts/overruns and169 repair attempts/1 adoption. The
failed ancestor's cost remains charged. No world/RNG/item reset occurs; only
complete full-entry acceptance establishes the new result. Final PIV918390,
clock5; stage points55/147/313/328/187/310 and orbs8459/12800/18455/23610/16596/43756.
[Validation](VALIDATION.md#selected-native-progress-replays) owns the exact lineage.

Full-board targets alone, off-axis charging, relaxed proposal guards and the
first final-survival suffix remain below the story incumbent. Interrupted
tasks295/296 have no result: the queue session exits143 with no surviving owner;
the actual Native exit signal/counters and interruption cause are unavailable.
Both stay charged, with original files retained. Independent process groups
retry them as297/299, yielding the completed scores recorded above and a
3,947,291,450 story,37/37. The normal frozen-binary Release Native/DAT CTest
passes **59/59 in1078.17s**. Deeper Native repair profiles and explicit terminal
Extra refinements were then launched under the four-owner/320-task window;
their outcomes and the later authorized640-task window are recorded below.

The terminal Extra portfolio reconstructs the verified2,493,554,650 prefix at
61756 inputs before the last ordinary HP attack, or66879 before the final source
survival attack. Faster capture, full-board faster capture and final-survival
full-board tails complete2,455,253,060/2,465,152,040/2,489,337,510,14/14.
An adaptive off-axis tail with1.5/240 reserve instead completes
**2,504,560,470, NMNB,14/14**, adding11,005,820 to this episode's best. It charges
72637 owner updates/64.1007s including prefix reconstruction,62027+6 branch/proposal
updates,129 rejected deaths,0 timeouts/overruns and0 future-repair attempts. The
585.409s ancestor remains charged. Fresh exact41-word terminal and natural replay
pass;925 points/90992 orbs/PIV754960. This selected real-seed progress remains
below the historical Native2,589,417,330 score and the human target.

Deeper repair v4 explicitly raises the extra-attempt limit128→384 and tests
lookbacks32/64/128/256/512/1024 under the declared search budget. Extra/story
early-graze retries still fail at the identical48656/71183 boundaries and emit
byte-identical failed tapes to their shallower ancestors. Their extra work is
retained, not a scoring gain. H2048 eventually completes NMNB,37/37 at
3,795,050,440:145381 owner updates/1734.82s,2890244+35 branch/proposal updates,
3095 rejected deaths,7 timeouts/overruns,3 rejected future capture failures and
1629 repairs/13 adoptions. More lookahead restores completion but loses score.
Post-4A and FinalB-only early-graze continuations complete4,073,958,800 and
4,166,960,860,37/37, below the selected4,311,260,760. The acceptance criterion
remains actual complete score, rather than a larger budget or local orb count.

The final Extra reserve1.2/120 tail from the same61756-input incumbent prefix
completes **2,508,068,110, NMNB,14/14**. It uses the stock v3 policy, preserve-fire
recovery and the recorded H512/beam384/30s schedule; no off-axis override.
It charges73237 owner updates/57.5388s,55282+5 branch/proposal updates,
131 rejected deaths,0 timeouts/overruns and0 future repairs. Fresh exact terminal
and natural replay pass. The selected current-episode replay retains925 points,
92024 orbs and PIV760120; historical Native seed10025 remains the higher Extra score.

### Measured point economy and next scoring work

The final source probe records every large-point request, successful allocation
and pickup without changing inputs or RNG. Both complete tapes preserve the
exact original natural25-field rows, and logged pickup counts equal the source
audit925/1340. The first buffered probe loses its final log block because the
checker uses `_exit`; its incomplete totals are rejected. The corrected probe
flushes each event and validates complete CSV records before reporting totals.

| Audited run | Requests / spawned / collected | Actual large-point income | Same-items static price headroom |
| --- | --- | --- | --- |
| Earlier Extra46997,2.508B |976 /930 /925 |748,388,270 |65,896,350 |
| Drop-target Extra46997,2.603B |1042 /1041 /1015 |824,889,570 |55,681,290 |
| Story1805 |1368 /1368 /1340 |1,263,269,100 |192,755,140 |

The earlier2.508B Extra has1 request outside source X bounds and **45 pool-exhaustion rejections
in one burst at source frame56723**. Only5 successfully spawned large points
go uncollected. Its48 low-value pickups and142 non-extreme-human pickups are
overlapping categories. Story has28 uncollected large points,150 low-value and
167 non-extreme-human pickups. Its static price losses by raw stage0/1/2/3/5/7
are2,261,880/8,331,170/15,752,450/68,378,810/46,898,080/51,132,750.
These ceilings hold the actual goods/PIV fixed, not the altered trajectory or
future score; they are opportunities to test, not verified additional gains.

Next, test a generic free-item-slot reserve before an imminent enemy/drop payout,
using current source occupancy, carried shots and actual Native continuation.
The45 rejected Extra points occur before the current61756-input tail checkpoint,
so later tail tweaks cannot recover them. Preserve the game's allocator; change
kill/collection timing. Separately compare a sticky current-actor drop target to
the current weighted centroid, which can aim between separated enemies, and
prepare source human/full-value pickup timing without losing profitable grazing.
Finally, retain complete-terminal score comparison across adaptive continuations:
first-safe acceptance alone does not use a longer compute budget to maximize cash.

All producer sources, CSVs, row hashes and incomplete-probe evidence stay in
`reports/local/retail-score-offline-20261026/point-income-probe-v{6,7}/`.
That checkpoint ends at **320/320**. The owner subsequently authorizes more
offline work under bounded memory. The active window is now640 cumulative tasks,
with the original ledger retained and at most4 concurrent solvers. Further
recorded budget extensions are authorized under the same memory conditions.
Each solver process has a2GiB address-space limit; each owned process tree has
a2GiB summed-RSS guard. Admission requires12GiB host MemAvailable; below8GiB,
owned work is stopped and recorded as a resource abort. Historical measured
solver peaks are about80MiB RSS. Both human-score targets remain unmet.


### Memory-bounded follow-up and new Extra high

The selected Extra now reaches **2,620,568,090, NMNB,14/14**, or86.73% of the
dated3,021,639,920 human reference. It uses the same original retail seed46997.
The full-entry parent reaches2,603,278,620 with1015 points/89095 orbs/PIV745480.
A generic current-actor goal ranks actual damageable drop-bearing enemies above
the player by drop count divided by current HP and horizontal travel cost. Boss,
existing-item, hit-income and graze goals retain priority. The combined profile
also uses the earlier source hit-window focus pulse, beam384 and12s total search;
this is not a component ablation. It charges72133 owner updates/620.136s,
2396093+240 branch/proposal updates,1012 rejected deaths,0 timeouts/overruns and
145 future repairs/5 adoptions. Fresh exact terminal and natural replay pass.

The read-only parent audit preserves the exact stock natural rows. Compared with
the earlier2.508B tape, it requests66 more large points and avoids45 full-pool
rejections:1041 spawn,1015 are collected,26 go uncollected, and1 request lies
outside source X bounds. These are observed combined-profile effects. The prior
pool audit sees1961 normal-rising and105 apex-rising items at the45 failed requests.
A second probe places those requests at56723, before the EndSpell clear at56724;
at that later hook2089 slots are already occupied. Do not attribute the original
point loss to the later clear. Ordinary rising orbs return early only after
shotTimer becomes negative; releasing fire must outlast the source20-update loop.

The new cash comparison retains a certified incumbent, tests at most4 Native
32-update fire/focus alternatives and selects strictly higher horizon cash.
It re-executes each branch with its own true RNG, with all extra work charged.
From60879 literal parent inputs, H512/publication256/lead1/nominal1/interval128,
beam384/30s/reserve1.2/120 completes **2,620,568,090** in72557 updates/114.434s.
It makes271 comparisons/34 adoptions,138928+6 branch/proposal updates,328 rejected
deaths,0 timeouts/overruns and0 future repairs. Points remain1015; orbs rise to
91083 and PIV755420. The620.136s parent remains charged. Fresh exact41-word and
natural replay agree. Original selected exports and provenance are in
[Validation](VALIDATION.md#selected-native-progress-replays).

Public Native controls now expose default-off `--drop-target 1` and
`--economic-proposals 0..4`; [Planner](PLANNER.md#spell-capture-and-score-objective)
owns their contracts. The full-entry parent additionally uses private focus-pulse
and future-repair sources frozen locally; public controls alone do not promise
the identical full-entry tape. The final public build reproduces the2.620B suffix
with the identical full72557-input tape and41-word terminal in131.492s, including
the original60879-input prefix. Whole-terminal score remains the acceptance test.

Negative trials remain charged and do not replace the incumbent:

| Trial | Complete score / outcome | Cost and implication |
| --- | --- | --- |
| Pool reserve64 /256, original warm-tail reuse |2,446,807,820 both,14/14 |147.194s/145.025s;925 points; simple pressure-triggered release does not recover score |
| Pool release with32-update hysteresis, H512/fresh goals |2,482,263,120,14/14 |441.536s; larger budget and sustained release still lose total cash |
| Drop-target H512/fresh-goal Extra |2,442,045,300,14/14 |907.964s; longer lookahead is not automatically more profitable |
| Drop-target full Lunatic story |3,757,933,990,37/37 |474.469s;1387 points but fewer orbs; selected4,311,260,760 remains |
| Cash comparison, H1024 at53599 |NMNB failure at53621 |68.5239s;516 failed futures,0 economic comparisons; initial continuation never certifies |
| Cash comparison, earlier2.508B prefix61756 |2,492,881,310,14/14 |157.521s;271 comparisons/30 adoptions; horizon cash can lose full-run score |
| Stock1.2/120 tail from new2.603B prefix60879 |2,583,293,380,14/14 |53.9381s; lower than both selected parent and cash-comparison tail |
| Longer cash horizon, H1024 from60879 |2,600,269,250,14/14 |216.644s;256 comparisons/32 adoptions |
| Longer64-update cash alternatives |2,612,213,530,14/14 |93.1615s;278 comparisons/22 adoptions |
| Cash comparison, story FinalB continuation |3,970,812,430,37/37 |319.910s;681 comparisons/81 adoptions; FinalB234 points/30398 orbs versus310/43756 |
| Explicit PIV potential, Extra/story |2,618,756,400 /3,966,533,780, all captures |129.395s/370.387s; fixed600*PIV plus displayed cash is a heuristic and does not beat either incumbent |

A separate read-only chain ledger preserves exact full natural rows for both
the2.603B parent and the dated ASL reference. Before source frame21000 it observes
52 independent familiar deaths/24 parent-chain rewards for our tape, versus1/34
for ASL. Seeds and timing differ: this is descriptive evidence, not a controlled
causal comparison. Source `DetachEnemyChain(1)` pays per live chain member, then
adds2*linkedChildCount orbs and localized cancel regions; killing an attached
child separately gives a different small reward and gauge reset. `CountParentChain`
and linkedChildCount differ in real events and must not be silently equated.
The final generic input prototype focuses while a currently damageable parent
chain is above/in the player shooting lane, gated by current gauge, without a
stage/card schedule. Both full-entry trials finish Native NMNB14/14 but lose
score: human-margin2,403,247,570 (974 points/78899 orbs,494.223s), neutral-margin
2,478,813,720 (925 points/91687 orbs,524.270s). They charge2298381+128 and
2419101+312 branch/proposal updates,389/978 rejected deaths,0 timeouts/overruns,
43/29 repairs with3 adoptions each. Neither is selected or promoted to a public
default; no new fresh/natural verification is claimed for these rejected runs.
Exact source/row checks and CSVs stay in local `chain-probe-v1/`; family producer
sources and complete rejected artifacts remain local. This closes tasks355/356.

The failed H1024 case motivates retaining a freshly re-executed known incumbent
suffix at an offline checkpoint before proposing changes. Reusing input data is
allowed; assuming that suffix remains safe after a changed branch is not. Longer
Native farming horizons should also value delayed PIV and unsettled bonuses;
endpoint cash alone is not a complete economic objective. Story needs a strategy
that preserves its earlier orb/PIV gains while improving later capture/pickup cash.
The2.603B owner peaks at136,847,360 summed-RSS bytes, with host MemAvailable
never below27,696,214,016 bytes. No follow-up solver hits a memory abort.
The integrated default-off goals pass the normal Release Native/DAT suite
**59/59 in1191.44s**, plus the maintained-document check. The full suite peaks
at1,002,758,144 summed-RSS bytes; its minimum host MemAvailable is12,034,150,400
bytes, above the8GiB abort threshold. No memory abort occurs. Two earlier
partial suites were deliberately stopped before relinking and remain charged;
neither is reported as a pass. The final public full-entry Extra continuation
reproduces the selected exact input tape and terminal at131.492s.
Local sources, manifests, diagnostics and failed setup attempts remain under
`reports/local/retail-score-offline-20261026/`; the cumulative ledger is retained.

## Established scoring mechanisms

The policy uses current source state and general geometry, without a seed/card
movement script. Card numbers below identify measurements only. Complete
Native rollouts decide safety; proposal forecasts never certify a prefix.
[Planner](PLANNER.md#spell-capture-and-score-objective) owns implementation details.

1. **Point pickup price and form matter more than raw graze totals.**
   `Item::CollectPoint` prices the item's Y at pickup; extreme-human gauge doubles
   that value. Auto-attraction does not guarantee max value. Actual pickup reach
   is the player's pickup half-height plus the item's half-height:32 pixels for
   this shot, versus the old goal only 12 pixels above the price line. The new
   goal uses source pickup reach+2; no-boss positioning stays near that height
   after collection, while human preference applies outside active graze goals.
2. **Ordinary-spell grazing can profit when income exceeds bonus decay.**
   The policy observes effective HP to the next callback, mature damage rate,
   remaining time, extreme-youkai state and realized orb income. It releases new
   fire only with a capture reserve, waits for the gauge, measures an income
   window, and restores fire when unprofitable or close to the deadline.
   Neutral movement recovery preserves the primary policy's branch-owned fire
   intent instead of ending a productive attack accidentally.
3. **Source geometry and reward boundaries changed the diagnosis.**
   Ordinary graze expands the bullet by 20 pixels. The initial macro forecast
   omitted this;413 fixed-input samples had zero macro contacts. The mirrored
   stream regression fails before the fix and passes after it. Audit start
   snapshots now follow preceding deferred-bonus settlement, and distinguish
   ordinary extreme-youkai graze from extra-item-suppressed contacts.

Read-only price instrumentation preserves every original 25-field replay row:

| Actual complete-run component | ASL reference | v10 | v15 |
| --- | ---: | ---: | ---: |
| Terminal score | 3,021,639,920 | 2,308,450,110 | **2,564,311,730** |
| Point pickups | 1081 | 944 | 1010 |
| Full-height pickups | 1081 | 731 | 961 |
| Extreme-human pickups | 1081 | 702 | 977 |
| Actual point-item income | 1,062,330,020 | 625,587,010 | **847,202,820** |
| Collected time orbs | 105531 | 84947 | 83160 |

V15 gains 221,615,810 in point income and 255,861,620 overall despite 1787 fewer
orbs than v10. Those are measured score components in different action-dependent
scenes, not a component ablation. For the same observed items/PIV, full-height
human pricing leaves 29,733,620 of static pickup-price loss in v15, versus
180,729,810 in v10. These counterfactual ceilings are not guaranteed policy gains.

Other source constraints to preserve:

- Score storage is one tenth of displayed points. Spell bonus/item prices are
  already displayed units; do not compare them directly with raw journal scores.
- Extreme-youkai ordinary boss-spell graze can request 3 time orbs for this team;
  suppressed contacts such as the laser path request 1. Counter increments 3/2/1
  are gauge-dependent and do not count physical graze events consistently.
- Releasing Z leaves the running 20-update shot cycle and carried projectiles
  intact. No-shoot gauge decays; rapid form switching resets shooting gauge ramps.
- Deferred capture orb requests are emitted in batches and are not realized
  collections. Carried items cross spell boundaries. Temporal audits are not
  causal attribution to a card. The measured PIV follows base+5/orb; the rare
  extra-PIV branch has not supplied a verified additional scoring insight.
- Ordinary timeouts lose capture. Source timeout-spell handling differs and can
  capture at zero time; that flag does not prove HP immunity. Watching a replay
  does not populate persistent Spell Practice capture history.

## Historical component exploration

Local paths in this section are relative to
`reports/local/score-styles-20261025/`. Every attempt keeps its frozen binary,
source identity, seed/profile, tape, result and verification where applicable.
The task ledger is `reports/local/shadow-runtime-v1/tasks.tsv`:213/320 campaign
tasks before cleanup,215/320 after the two replay/follow revalidation tasks,
at most4 solver jobs at that checkpoint. New reports/tapes/replays stayed local
except the then-selected highest replay. Later explicitly selected progress
files are listed in current provenance; no ablation matrix has been run.

| Experiment / local directory | Complete score or failure | Lesson |
| --- | --- | --- |
| `extra-harvest-v2` | 1,871,533,920,14/14 | Quiet nonspell collection+edge cost; first lower-edge variant fails 10/14 |
| `extra-flow-v6`, `extra-flow-v8` | 1,970,924,450 /1,926,415,230,14/14 | Valid scores, but ordinary pacing never activates and macro contacts were empty; do not credit the broken macro |
| `extra-flow-v9` | Final-card failure 13/14 | Corrected 20 px geometry produces 5878/15195 interval orbs on cards 196/198, versus ASL830/11455 |
| `extra-flow-v9-repair-baseline` | 2,205,160,930,14/14 | Preserve 50760 inputs, generic baseline H64 continuation; original v9 failure remains charged |
| `extra-flow-v10` | 2,308,450,110,14/14 | Gauge-aware income sampling+fire-preserving neutral recovery, complete from entry |
| `extra-flow-v11` | 2,192,904,520,14/14 | Strong 40-point focus prior is weaker than 5; not promoted |
| `extra-flow-v12` | 2,271,435,430,14/14 | Price clearance+collection bias improves point income but loses 71 items; total score lower |
| `extra-flow-v13` | 2,173,050,400,14/14 | Precollection fire release reduces items to 814; not promoted |
| `extra-flow-v14` | 2,354,433,630,14/14 | Generic reserve 1.2*HP/DPS+120 can raise score; defaults remain 1.5/240 |
| **`extra-flow-v15`** | **2,564,311,730,14/14** | Upper-stage goal+economic human preference with price-aware collection; selected replay |
| `extra-pacing-main-v1` | 2,197,464,170,14/14 | Integrated pacing seed 19732; first input difference from v10 at 11048 |
| `extra-pacing-fresh-v1` | 2,336,991,400,14/14 | Seed 10025 drawn before outcomes, fixed integrated pacing profile |
| `extra-point-stage-main-v1` | 2,453,582,120,14/14 | Same new seed with price/staging/form controls and 1.2/120 reserve |
| `extra-point-stage-main-reference-v1` | Seed 19732 fails 13/14 | Main with v15-style 1.5/240 settings; no uniform replay-generation guarantee |
| `extra-point-stage-main-repair-*` | Expert succeeds 2,464,473,300; baseline/zen fail 13/14 | Bounded generic continuation portfolio; offline only |

Earlier `extra-farm-*`, `extra-economy-v4`, blanket-form, owner-goal-refresh and
precollection trials retain failures. Simply extending an attack or raising focus
is not evidence of profit. Source-survival-only `score-flow` fails 13/14 on
seed 19732, newly drawn 37854, and 19732 after the 20 px correction. Those preferences
remain optional, with no favorable-only seed selection.

### Costs of the principal complete and failed results

All figures are Release on the shared EPYC7B12 host,16 logical CPUs/64 GiB, GCC12,
contraction disabled. Owner pauses during Native planning. Search time includes
rejected futures; Native CPU counters exclude file I/O. Wall budgets/timeouts can
change candidate selection even with the same seed. Table costs are per attempt,
not an aggregate claim that ignores preceding work.

| Run | Owner updates / wall seconds | Branch + proposal updates | Rejected future deaths / timeouts / overruns |
| --- | --- | --- | --- |
| v15 highest | 68429 /408.479 | 2216535+312 | 461 /29 /32 |
| v10 | 67165 /407.655 | 2191882+544 | 633 /53 /57 |
| Main seed 10025, price/staging | 73493 /442.000 | 2403152+544 | 842 /55 /62 |
| Main seed 19732, failed 13/14 | 65137 /391.341 | 2126610+449 | 685 /46 /50 |
| v9 original failure | 53684 /340.572 | 1752204+444 | 581 /47 /50 |

V15 additionally charges 70 neutral recovery attempts/32 adoptions,32 retained
windows/256 inputs,8 startup fallback inputs and 38.097 s fresh following. V10 has
96/29 recovery attempts/adoptions and 39.145 s fresh following. Main seed 10025 has
117/46 recovery attempts/adoptions; its complete profile is in Validation.

The main-reference recovery preserves 61789 inputs and the complete carried
world,8 updates before the source last-spell boundary. Expert H128/publication 64,
lead 8/110 ms/60 ms completes at 67845:20.518 s continuation,26.273 s including
reconstruction,96196 branch+24 proposal updates, no timeout/overrun. Baseline H64
and zen H64 fail 13/14, with 15.334 s/13.857 s including reconstruction,
27968+76 /24729+77 branch/proposal work and 8 timeouts/overruns each. The first
changed input is 61792. The original 391.341 s failed run stays charged.

### Historical Wine and offline-ahead evidence — live solving paused

The first live port of the high-score controls is wine-64, original real seed
47691. Pacing, price clearance/staging and5-point economic form preferences use
H256/publication128/lead24,250ms total/60ms proposal, baseline+held recovery and
reserve1.2/120. It fails at54833 on card202:1,421,550,830 partial points,
887 items/74374 orbs. There is no meaningful mismatch or player-position error.
6779 searches charge1750462 branch+589 proposal updates,153 rejected future
deaths,32 rollout/2 proposal timeouts,58 baseline attempts/23 adoptions. No plan
is stale and no prefix is rejected. Publication consumption p99 is8 updates,
minimum publication slack10. The final trap outlasts the verified queue;
18 unqueued updates include14 startup updates. Over926.807s it averages59.163
calculations/s; period p99/max24.117/57.878ms, search p99/max159.475/258.251ms.
The failure remains charged; it is not a completed score or deadline proof.

The preceding actual attempt is wine-63, original real seed 12421,
score-harvest 128 verification/publication, lead 8/110 ms/60 ms. It is hit in the last
card at 41863 calculations:1,222,798,110 partial points,903 items/40017 orbs.
Fresh 41-word following agrees, no meaningful mismatch/position error;1209
numeric-drift updates remain telemetry. It averages 59.733 actual calculations/s
over 700.833 s (period p99 20.087 ms, max 66.781 ms). It charges 678012 branch+383
proposal updates,283 rejected future deaths,28 timeouts,34 overruns and 31
unqueued receipts. A Last Spell hit invalidates this target even before the
ordinary death counter increments. The complete online PB remains wine-35.

Wine-65 tests H128/publication64/lead16/200ms, retaining price/pacing and1.2/120
reserve. Original seed52387 fails at37577/card198:946,361,780 partial points,
720 items/47785 orbs. No meaningful mismatch, player error, stale plan or prefix
rejection. Work is602781 branch+134 proposal updates,66 rejected future deaths,
18 rollout timeouts,37 recovery attempts/19 adoptions,24 unqueued updates.
Over649.547s the mean is57.851 calculations/s; period p99/max30.671/99.877ms,
search p99/max153.235/208.869ms and minimum publication slack7. Shortening the
certificate alone does not resolve this failure. Wine-66 uses a separately
recorded expert/capture recovery profile; its outcome is not yet verified here.

Source probes in `reports/local/extra-live-score-20261026/` preserve the selected
v15 tape's full41-word terminal and all83160 time-orb pickups. Slot provenance
separates1164 collected orbs from nonboss reduction-window shots,663 from
nondamageable actors,2883 from reduced boss shots,2192 from attached-target
shots,5565 from other human shots,52179 from graze and18514 from other sources.
The categories are disjoint; no new income is claimed from these existing orbs.
The independent default-off `--hit-income` goal tries current nonboss reduction
targets. Task219 completes Native seed47691 at2,430,681,420 NMNB but only13/14:
ordinary card199 times out. This is an unsuccessful scoring target, not a new PB.
It charges648.646s,2407386 branch+808 proposal updates,694 rejected future deaths,
109 timeouts/123 overruns and176 recovery attempts/48 adoptions. Fresh41-word
execution agrees. Natural replay matches the score and13/14 outcome, and
`--require-all-captures` correctly rejects it with exit3.

A second read-only ledger measures79 consecutive240-update no-fire windows;
25 have negative cash+current-bonus+Extra orb/graze clear value, before assigning
any unseen future point/PIV income. Several zero-orb windows lose over1.7million
points of this current valuation. Source `Player::OnUpdate` moves gauge toward
zero without shooting and resets its shooting ramp. A named local gauge-ready
prototype tests focused fire during reduction, then firing through gauge warmup
before withholding Z at extreme-youkai. It retains mature damage observations
while warming. Task222 fails card202 at44683,1,382,409,240 partial points with
63468 orbs and10 captures. It charges323.749s,1462497 branch+395 proposal updates,
674 rejected future deaths,45 timeouts/50 overruns and67 recovery attempts/15
adoptions. It is not promoted. A separate local expert/capture continuation
profile retains that original failure and starts again from full Extra entry.

The ledger reaches225/320 after the first two live attempts, source probes,
Native hit-income experiment, Wine-66 launch and fresh/natural checks; later
prototype tasks append to the same ledger. At most4 solver jobs remain allowed.

`extra-ahead-h64` preserves 38000 actual wine-63 inputs and completes offline at
1,923,125,200,14/14. Its H128 companion fails 13/14. The H64 first changed input
is 38006, less than the live 8-update lead; historical pending queue and delivery
timing were not reconstructed. This and the other checkpoint repairs prove
continuation feasibility, not a live handoff or permission to rewind the owner.

## Unresolved problems and proposed next work

The handoff itself authorized no new work; the owner has now resumed exploration.
Current trials require No Miss and No Bomb, allow missing captures, and accept
only higher complete score. Restore all captures explicitly when requested.
Keep independent options for later ablations; the owner has not requested an
ablation matrix now. The following are proposals, not verified gains.

| Current problem / evidence | Next bounded investigation | Acceptance |
| --- | --- | --- |
| Both human-score gates remain unmet | Extra first: actual item production, parent-chain termination, form/pickup price and orb/bonus timing; retain story4.311B while studying FinalB | Higher complete No Miss/No Bomb score under the declared capture goal, exact fresh/natural verification; Wine playback only after the target |
| Endpoint cash can prefer locally profitable inputs with worse full score | Value real pending capture rewards, source bonus cap/decay and remaining actual point opportunities. Fixed600*PIV fails both episodes; do not treat an assumed future item count as guaranteed | Whole-terminal gain with all extra rollout/retry costs charged |
| More Native horizon can fail before reaching a known-safe existing tail | Freshly certify the literal incumbent suffix at a checkpoint, then compare economic alternatives. After edits, re-execute that suffix in the new branch; never assume unchanged RNG or safety | Retain a certified incumbent; incomplete/failed futures remain proposals only |
| Familiar-chain value is nonlinear; broad focus loses score | Read current parent/child/chain state and projectile/form timing. Preserve source hit income and human pickup price; test selective mother targeting or maturation only with declared assumptions | Better actual collected income and complete score; distinguish live chain length from linkedChildCount |
| Selected full-entry parent uses private repair/focus code | Port only measured useful behavior through typed interfaces if needed. Current public optional drop/cash goals are already integrated; five parent repair adoptions are not proof a larger framework is needed | Focused contract plus full-duration Native case and normal CTest; no old-object/new-header ABI mixing |
| Parent2.603B still leaves26 spawned points and trails ASL's1081 points | Trace production and pickup timing on the current tape; static55,681,290 price headroom is not a guaranteed trajectory gain. Avoid returning to superseded925-point assumptions | Actual point income and complete score improve without sacrificing capture/orb economics |
| Story FinalB cash/PIV trials lose76 points and many orbs | Compare source event-aligned capture/collection/form continuations while keeping earlier strong stages and their carried PIV intact | Beat4,311,260,760 from original retail-seed entry, not just a better isolated card |

### Resume here

Use the [operational contract](#operational-resume-contract), the guarded
[current command](VALIDATION.md#resume-extra-scoring-on-main) and the current
local inventory near the top. Code ownership is: current facts in
[online_native.cpp](../src/online_native.cpp), typed score/configuration in
[online.hpp](../include/th08/online.hpp), proposal goals/history in
[online.cpp](../src/online.cpp), Native guidance in
[shadow_proposal.cpp](../src/shadow_proposal.cpp), fork certificate semantics in
[shadow_rollout.cpp](../src/shadow_rollout.cpp), and synchronous experiment/score
comparison in [shadow_style.cpp](../tools/shadow_style.cpp).

Historical v15 local inventory: `extra-flow-v15/manifest.json` binds that older
run; `alpha-v15-src/` freezes its sources; `point-income-audit-v1/`,
`point-income-v12/`, `point-income-v15/`, `extra-phase-economy-v2/` and
`pacing-v10-follow-*` retain earlier probes. They are not the current producer
or active episode. Preserve useful history without using it as the next command.

## Validation and preserved historical work

Current public scoring code passes **59/59 in1191.44s**; see the current local
test inventory above and [Validation](VALIDATION.md#selected-native-progress-replays).
The remaining paragraphs retain earlier component checkpoints, not the current
test count or active campaign limits.

The complete-tape refiner, source-survival priority fix and slowdown metadata
reporting pass Release Native/DAT CTest **57/57 in1020.13s**. The new end-to-end
refinement regression preserves complete source capture coverage, rejects safe
incomplete and timed-out incumbents with charged work, then checks fresh exact
terminal execution and natural original-writer replay. Focused2/2 passes48.19s;
the final documentation check passes. New selected story bytes and previous
selected replays are preserved; other generated experiment artifacts remain local.

The offline checkpoint/budget controls pass a subsequent normal Release
Native/DAT suite **57/57 in970.95s**, including long-H512/publication256/lead1/
nominal1/interval128 continuation, literal-prefix agreement, fresh terminal and
natural replay checks, invalid schedules and terminal-prefix rejection.
Focused checkpoint validation passes1/1 in35.65s.

The live scoring port and optional current reduction target pass the normal
Release Native/DAT suite56/56 in1175.95s and core36/36 in12.75s. The earlier port
suite also passes56/56 in1189.57s. Final recovery-control/source-policy checks
pass4/4 in34.55s, including invalid schedule/option rejection before owner wait.
Logs are under `reports/local/extra-live-score-20261026/`. The selected replay is
unchanged; no new reports, tapes, videos or experiment replays are published.

At `667597f`, Release Native/DAT CTest passes 56/56 in 889.37 s and core 36/36
in 8.47 s. The 56-test local configuration includes two private retained-trace
regressions; the then-normal DAT build had54. There are 36 core CTests without private
data, plus two optional pinned-source checks. Fresh handoff verification is
recorded in [Validation](VALIDATION.md#selected-extra-score-replay).
The October26 cleanup reruns both suites: **56/56 in933.85s**, **36/36 in8.49s**,
plus unchanged highest-replay rows/captures and exact fresh41-word terminal
agreement. Logs stay in `reports/local/handoff-cleanup-20261026/`.
The earlier relink-during-test binary-identity failure stays in local evidence;
subsequent frozen-binary full suites pass. The full retail fidelity gate remains
106/108; fresh Native replay is not an independent retail oracle.

Earlier records are historical, not an active queue. The complete pre-cleanup
[status snapshot at667597f](https://github.com/N0zoM1z0/th08-solver/blob/667597f/docs/STATUS.md)
retains detailed per-component diagnostics, timings, failed experiments and
numerical-profile limitations. The [report index](../reports/native/README.md)
owns the unchanged published machine reports; [Validation](VALIDATION.md) owns
commands and contracts. No report or failed test is deleted in this cleanup.

| Preserved work | Evidence and limits |
| --- | --- |
| Recorded Lunatic 4A→FinalB story | Milestone tag `lunatic-nmnb-4a-finalb-20261024` at `d538bcc`; matching selected replay and audio/video delivery.33/37 captures, missing ordinary cards 6/10 and conditional Last Spells 13/32 |
| First score-directed Native story | Seed 40755,2,596,027,940,37/37,110669 updates,400.02 s. Live score-story seed 40978 fails in 4A card 70; not a new online PB |
| Stage 4B | Paused after `288c6fe`; actual seed 39151 dies at 20648/card 92 with exact 41-word following. Generic long-horizon/recorded-geometry attempts retain failures |
| Complete online Extra | wine-35 proper entry,46501 calculations,1,166,969,620,14/14; matching original-quality recording/replay;56.357 average calculations/s under the old renderer |
| Offline full-route portfolio |24 seed-0 combinations;16 random-seed stories complete through separately charged continuations after all original 2400 s attempts exhaust budgets. Not all 108 route combinations or online coverage |
| Current-memory policies/learning | Frozen 172-trial bank: Extra 202 generic 23/24 versus baseline 14/24; both value models 9/24. Spell proxy only. Models stay optional; no new RL phase |
| Exact runtime acceleration | LZSS/surface-copy checks preserve 108 replay tuples and complete tapes. Stage initialization peaks remain environment-dependent; historical FinalA initialization exceeds 16.67 ms |
| Legacy headless hazard/repair |56/56 Easy/Extra checkpoint recipes and nine stage entries at their named profile, including bounded repairs and retained failures. Earlier per-card adapters do not become the generic scoring kernel |
| Transform-contract proposal | Rejected after losing successful training cases; retained in [Validation](VALIDATION.md#paused-transform-contract-experiment) and [experiment evidence](../reports/experiments/transform-contract-rejected.json) |

The legacy subset engine is separate from the Native production calc chain:

| Outcome | Verified count |
| --- | --- |
| Indexed original spell IDs | 222 |
| Indexed spell-start occurrences | 431 |
| Complete offline spell solutions | 0 |

Controlled DAT ID179 and synthetic relay cases retain their separate labels;
these counts are not Native runtime coverage percentages. Preparation originals
remain unchanged/local-only and absent from published Git history. No menu,
rendering or camera reconstruction is needed to resume checkpoint experiments.
