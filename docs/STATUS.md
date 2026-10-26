# Current handoff status

Updated 2026-10-26, continuing from handoff `b58aad0`.
The Extra target of **2,500,000,000 displayed points, NMNB and 14/14 captures**
is achieved by a frozen Native prototype. The owner now requests **Native-only
score exploration**, followed by Wine retail **replay playback** only after the
export meets NMNB, all captures and the human-reference score target. Online MPC
and live Wine solving are paused. Extra requires14/14 and >3,021,639,920; the
complete Magic Team Lunatic4A→FinalB route requires37/37 and >6,168,365,790.
New replay names are `ysopwnab`. Stage4B remains outside this route. Before a new
scoring episode, capture its original retail Wine seed at the pre-RegisterChain
boundary and freeze it for offline candidates. Wine seed acquisition is allowed;
live solving stays paused. Keep the seed receipt and entry/checkpoint provenance.
The existing Extra seed10025 results are historical Native benchmarks, not fresh
retail acquisitions under this rule. Story seed40755 retains its original
recorded Wine provenance; do not relabel old experiments as new seed receipts.
[Issue11](https://github.com/N0zoM1z0/th08-solver/issues/11) retains the
source investigation and experiment discussion.

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
"Complete" below requires NMNB, all eligible captures, exact fresh 41-word
terminal agreement and natural original-writer replay with matching score/captures.
Fresh replay checks execution agreement, not independent physics or retail equivalence.

| Evidence | Score / captures | Scope |
| --- | ---: | --- |
| **Current Native Extra** | **2,589,417,330 /14 of 14** | Seed10025, complete from entry,500ms offline search; selected progress replay |
| **Current Native Lunatic4A→FinalB** | **4,160,457,200 /37 of 37** | Original retail seed1805, full entry, audited Native future-repair v3; selected replay |
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

## Native score-first progress

The selected Native progress replays are complete from entry, with exact fresh41-word
terminal agreement, natural replay NMNB/all-capture checks and the selected name
`ysopwnab`. [Validation](VALIDATION.md#selected-native-progress-replays) owns their
identities and commands. Both remain below their explicit human-reference target;
exit5 and `objective_met:false` distinguish valid progress from meeting that goal.
No retail Wine playback is started for them yet. The source-survival priority
follow-up reaches3,903,603,650; the subsequent offline continuation reaches
4,126,102,910. The new retail-seeded full-entry story reaches4,160,457,200,
37/37, NMNB. Earlier selected replay bytes remain unchanged.

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
  decisions for nearly an entire horizon. The next Native change should keep a
  certified incumbent and compare a few newly verified economic alternatives.
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
verification. The selected2,589,417,330 incumbent remains intact. This exposes
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

## Retail-seeded score exploration

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
proposals and off-axis variants are now running, with no outcome claimed yet.
All alter proposals, not Native game behavior; no seed/card or human-input
scripts are used. The ledger is283/320 at this publication checkpoint.

## What produced the gain

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

## Exploration retained for the next session

Local paths in this section are relative to
`reports/local/score-styles-20261025/`. Every attempt keeps its frozen binary,
source identity, seed/profile, tape, result and verification where applicable.
The task ledger is `reports/local/shadow-runtime-v1/tasks.tsv`:213/320 campaign
tasks before cleanup,215/320 after the two replay/follow revalidation tasks,
at most 4 solver jobs. New reports, tapes and replays stay local
except the one owner-selected highest replay. No ablation matrix has been run.

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

These are proposed solutions, not implemented/verified gains. Preserve NMNB,
14/14 Extra or37/37 story and actual terminal score as the acceptance criteria. Keep options
independent for later ablations; no new ablation matrix is requested now.

| Problem / evidence | Proposed next experiment | Required acceptance |
| --- | --- | --- |
| Final-card reliability: main seed 19732 fails; expert checkpoint tail succeeds | Freeze the failing frontier and compare a bounded generic neutral/held/move-then-stop recovery portfolio, with explicit observer warming and work accounting. Integrate only if complete episodes improve | Complete from entry on old and predeclared fresh seeds, exact fresh execution and natural replay; retain the original failures |
| Prototype/main and same-seed reruns diverge | Bind source/binary/options, log the first differing candidate/certificate, timeouts, tie state and action. Test whether a deterministic work cap is useful as a separately named profile; do not silently replace time budgets | Explain observed divergence with evidence; preserve current ordering, budgets and failure meaning for baseline |
| Native first-safe selection and stale economic tails | Complete-tape strict-score selection is implemented; literal form/fire windows accept0/105 because the old timed continuation usually dies. Try a generic adaptive continuation or event-aligned proposal while preserving the actual world | Higher complete NMNB/all-capture terminal score; charge every added branch and reject observed capture failures |
| Native scores remain below the human references | Extra: early production/chain termination and PIV timing. Story: supply collection/price and form scheduling. Use reviewed source facts and new Native baselines | >3,021,639,920 Extra and >6,168,365,790 full route, followed by successful retail replay playback |
| Orb/bonus opportunity remains below ASL | Use actual per-spell income and source cap/decay state to choose whether to extend, reposition, then capture. Include point/PIV/clear rewards rather than optimizing raw graze | Higher complete terminal score without capture loss; charge every longer attempt and retry |
| Pickup loss: v15 still has 49 low-height and 33 non-doubled items | Measure transit pickups, gauge convergence and missed drops. Test geometry-aware staging/return paths, preserving actual carried shots and item state | Actual point income and complete score improve, not merely a static price ceiling |
| Suppressed-HP hit income is real; additional policy profit remains unproven | Read-only selected Extra follow traces1164 spawned/collected time orbs from nonboss reduction windows and663 from nondamageable actors, preserving all41 terminal words and83160-orb accounting. Test opt-in current reduction-target aiming, preserving old failures | Incremental collected income and complete score, including collection prices, capture-speed/bonus and added search costs; existing pickups alone are not additional profit |
| Short horizons can enter traps beyond their certificate | Test longer strategic targets/viability proposals with true-RNG Native safety; a longer horizon remains an explicit cost | Complete affected Extra durations and failure cases; no approximate safety certificate or seed/card script |

### Resume here

1. Read the result distinctions above, [Planner](PLANNER.md#spell-capture-and-score-objective),
   [source replay provenance](VALIDATION.md#selected-extra-score-replay) and the
   [scoring commands](VALIDATION.md#resume-extra-scoring-on-main). Main interfaces
   are `ScoreMemory`/`Configuration` in [online.hpp](../include/th08/online.hpp),
   `capture_policy_memory` in [online_native.cpp](../src/online_native.cpp), policy
   history/pacing/goals in [online.cpp](../src/online.cpp), branch guidance in
   [shadow_proposal.cpp](../src/shadow_proposal.cpp), synchronous orchestration in
   [shadow_style.cpp](../tools/shadow_style.cpp), and live orchestration in
   [shadow_online.cpp](../tools/shadow_online.cpp). Live scoring/scheduling controls
   are explicit; never infer the active profile from the style name alone.
2. For new scoring, first validate the owned retail seed receipt and exact entry.
   Continue the captured Extra46997 or story1805 episode above; a new episode
   requires its own retail acquisition before Native planning. Keep regression
   fixtures and historical benchmarks labelled separately. Reconstruct a prefix
   only from that episode's own entry/seed, preserving RNG, bullets, items,
   actors, latch and observer history. A fresh replay is a separate verification.
3. Freeze source/binary/configuration and a bounded complete-duration experiment
   before launch. Use at most 4 solver jobs and the remaining campaign task budget
   (283/320 at this checkpoint); do not reset the ledger to hide earlier attempts.
4. Optimize Native complete trajectories first. Require qualified exported
   replays before retail Wine playback; do not resume live MPC scoring.
   Freeze all binaries for the normal CTest suite; do not
   relink while a binary-identity regression is running. Publish only tested code
   and maintained prose on main, using the repository's English commit prefix.

Local evidence inventory: `extra-flow-v15/manifest.json` binds the highest run;
`alpha-v15-src/` freezes prototype sources; `point-income-audit-v1/`,
`point-income-v12/`, `point-income-v15/` own price probes and unchanged-row hashes;
`extra-phase-economy-v2/` owns corrected ASL/harvest/ahead/v6 interval audits;
`pacing-v10-follow-*.{csv,json,txt}` owns read-only gate diagnoses;
`extra-point-stage-main-*/` owns main results/recoveries. These private artifacts
are useful locally; the tracked replay, maintained conclusions and commit history
remain available without them.

## Validation and preserved historical work

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
regressions; a normal DAT build has 54. There are 36 core CTests without private
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
