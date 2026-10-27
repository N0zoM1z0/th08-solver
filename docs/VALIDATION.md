# Validation, provenance and reproduction

Current scoring mechanisms are in [Planner](PLANNER.md#spell-capture-and-score-objective);
[Status](STATUS.md) owns outcomes, unresolved work and proposed experiments.
This page owns reproducible commands, exact artifact identities and test contracts.
Historical component commands below retain their named build/profile assumptions.

## Native score-first workflow

The owner selected Native exploration for proper Magic Team Extra and complete
Lunatic4A→FinalB. The latest continuation requires No Miss and No Bomb, permits
missing captures and retains independent CLI controls. Bomb experiments are
paused. Require a complete
displayed score strictly above3,021,639,920 or6,168,365,790 respectively before
retail Wine replay validation. Historical NMNB/all-capture results retain their
identities. Live Wine solving is paused. Native budgets are explicit offline
compute limits; they need not meet an online frame deadline.

Before every new scoring episode, launch the owned Wine retail game and capture
the original seed at the observer's **pre-RegisterChain** initialization boundary,
the same boundary as Native `Session`. A seed read after stage initialization
is not interchangeable. Save the initialization receipt, entry stage/difficulty/
shot/story/history, executable/DAT/observer identities and seed-source provenance
under the ignored local episode directory before starting Native search. Reuse
that seed for the episode's candidate comparisons. A new episode gets a new
retail seed receipt; do not choose or reroll Native seeds after inspecting scores.
Historical benchmarks and fixed regression fixtures remain explicitly labelled.
Wine is used for this seed acquisition and, after qualification, retail replay
playback. Live Wine solving/online MPC remain paused.

Use the existing owned-process Wine launcher/observer with
`TH08_SHADOW_UPDATES=1`, `TH08_SHADOW_OBSERVE_REPLAY=0` and no Native command
producer. Declare `TH08_SHADOW_CHECKPOINT=8,4,1,-1,1` for Extra, or
`0,3,1,-1,1` for the story; the checkpoint chooses the entry and never writes
RNG. Set `TH08_SHADOW_PRIOR_FINAL_A_CLEAR=0` or `1` respectively and
`TH08_SHADOW_ADVANCE_DIALOGUE=1`. Use an owned game copy, dedicated Wine prefix
and fresh output directory. After normal menu confirmation, the observer writes
`start.txt` atomically before registration and exits after one observed update.
Validate its V2 fields, pinned retail hash, successful launcher exit and one
228-byte trace row. Preserve failed startup attempts; they are not valid seed
receipts. Record the validated receipt hash in the Native episode configuration.
Menu automation may retry a missed confirmation; a valid captured seed is never
discarded after a score preview. Seed acquisition is not a gameplay certificate.

The first episodes under this rule are in
`reports/local/retail-seed-episodes-20261026/`. Extra `extra-entry-v4` captures
seed46997; `story-4a-finalb-entry-v1` captures seed1805. Both freeze the existing
observer SHA-256 `9647fef617cfef1611296cd4eb611c399db6e20aab439f027f91c07ead5bb24e`
and launcher SHA-256 `2fdeee516adf32dd14b682256969edca63e06537be8f2c95920b239a9565ade1`.
They use the pinned retail/DAT identities documented below. Each directory keeps
the exact driver, manifest, `start.txt`, `seed-receipt.json` and `native-entry.json`.
`native-entry.json` binds the receipt hash before Native planning; its stage is
the raw runtime index, not the follower CLI's human stage label.

| Identity | Extra46997 | Story1805 |
| --- | --- | --- |
| Original `start.txt` SHA-256 |`db65567c75aa3e3cbc2e85ef96914fea18ceae84106ce0919044dd67086f8126` |`80c1d2c1c72d3c8e41b8e861235bda537e0a05d2c416e2789b90ccaf7fa211b3` |
| Seed receipt SHA-256 |`fcecb629fffb3a7973d37da14b73c6c61250db6e38a80e4995d4bee738a93c92` |`88516cae49c9c04a2ebc0857f88eb81b3d0ed7277ff4c4c95a1a8b39d55ae133` |

### Scarlet Team retail episodes

The October27 owner permits all shots and prioritizes Scarlet Team (shot2).
Its source route is1→2→3→4A→5→FinalB. Both new entries use original retail
pre-RegisterChain seeds, captured before any Native scoring candidate:

| Entry | Extra | Complete Lunatic story |
| --- | --- | --- |
| Episode under `reports/local/retail-seed-episodes-20261027/` | `scarlet-extra-entry-v6` | `scarlet-story-4a-finalb-entry-v1` |
| Owner task / fixed seed |665 /9098 |666 /32796 |
| Raw stage / difficulty / shot |8 /4 /2 |0 /3 /2 |
| Story / prior FinalA clear |true /false |true /true |
| `start.txt` SHA-256 |`90face8749068b9bb67b2d9e220d20a8b7df1a6e9a328551a40ede14f76c560c` |`701f1280c82f234c2ac3bbda95367f9832f2979d3391ea86bf63cb4b8017e95d` |
| Seed receipt SHA-256 |`44955955273c6e1ad3f5f2ddc74eef01776243d80f5dc098ef9db27bff247fb0` |`ab5faf14bc8578a873f012b2560803bc277139ba822879b4d4c677fb9e68f638` |
| One228-byte frame SHA-256 |`a0d5f19e27732e59b50827bcf59eac938dc1acb022a2fae9bfd00ccf8953b7bf` |`af143e3c863545b79d9405c514b65ba6f188c42c8bfd9bcb13b1eb431a39850c` |

Both keep the pinned retail/DAT/observer/launcher identities above, dialogue
advance, spell=-1, successful launcher exit0 and clean resource guards. Their
explicit32-bit Wine server hashes
`2a91df0aa96cb5713390a16efa6c694ccee745d3eba5c22a01e31ddf5bb014dd`.
The original acquisition flow uses neither the diagnostic socket alias nor
preload override. Every failed startup and diagnostic remains local and charged;
none supplies a substitute seed. `native-entry.json` binds its receipt hash;
`scoring-episode.json` declares the objective before Native exploration.
The Scarlet guarded launcher also requires a clean acquisition-owner exit and
the exact one-frame identity before admitting a candidate. Seeds remain fixed
for all subsequent profiles; Magic46997/1805 episodes retain their identities.

The owner expands the **entire current phase** to4GiB per-process address space
and4GiB owned-tree RSS before665, including Native solvers and Wine acquisition.
Host12GiB admission/8GiB abort and the four-driver cap stay unchanged. Owners
already launched retain their frozen original limits; old2GiB results are not
relabelled. This is an explicit resource change, not a search-budget increase.
It follows a Wine initialization crash with exactly2GiB mapped at664. The
4GiB receipts record actual tree peaks of136007680/139206656 bytes. Native
search/rollout/proposal budgets and failure meanings remain explicit per case.
Wine remains restricted to seed acquisition and qualified final replay viewing.

### Scarlet Team verified Native baselines

Tasks667/669 use the original Scarlet receipts above, fixed seeds9098/32796,
SampledGameplay, No Miss/No Bomb, captures optional and `ysopwnab`. Both use
the public Release solver with beam384,30s per search, cash4/parent100/human5.
Extra uses H128/publication64/replan32, original stream grid, no gauge charging,
reserve1.2/120 and preserve-fire recovery; story uses H256/publication128/
replan64, upper grid, charging, reserve1.5/240 and capture-fire recovery.

| Complete result | Extra | Lunatic1→2→3→4A→5→FinalB |
| --- | ---: | ---: |
| Displayed score |2,872,562,940 |5,011,063,470 |
| Actual bombs / misses |0 /0 |0 /0 |
| Captures |14/14 |35/37;74/77 time out |
| Owner inputs |71005 |152981 |
| Wall seconds |709.695 |1667.18 |
| Branch + proposal updates |883371+25 |2081032+17 |
| Economic attempts / adoptions |6721 /827 |7114 /1089 |
| Fresh / natural verification tasks |678 /681 |682 /684 |

Each pair checks all41 terminal words, score, exact capture map, zero
misses/bombs and the eight-byte player name. Fresh execution/index agreement
and natural source replay do not establish independent physics or retail
equivalence. The all-shot score gates and6B milestone remain unmet.

The selected files are local under
`reports/local/global-score-20261027/selected/`; published Magic milestones stay
unchanged. `extra-scarlet-2872562940-provenance.json` and
`story-scarlet-5011063470-provenance.json` bind receipt, DAT, binary, controls,
source, tape and natural-row identities, environment and limitations.

| Local replay | Bytes | SHA-256 |
| --- | ---: | --- |
| `extra-scarlet-native-nmnb-2872562940-seed9098.rpy` |29901 |`13e2b8f806406b9bee3201d4865994db94a045e7ebc3ffa1c89b14e60c187faa` |
| `story-scarlet-native-nmnb-4a-finalb-5011063470-seed32796.rpy` |56914 |`d6fcc33c6a75b28c6fc2a345fe8f31fcb3955947cecfee8ce3d332367f4dd0e0` |

Their action hashes are respectively
`6ac85ca19270572efa23b816499000c14d4e4403ffe5143f30a8ee29fe000c72` and
`a82a0e5ece6c823f1565951ec5ac3860c6522c00ddd105ab22afcd5b5c6ce985`.
Environment: Linux6.1.0-54-cloud-amd64, AMD EPYC7B12, GCC12.2.0, C++17 Release,
`-ffp-contract=off`, no fast-math; each guarded owner has4GiB process/tree limits
and host12/8GiB admission/abort thresholds. All I/O remains charged to wall time.

The complete Extra continuation694 improves Scarlet to **2,888,408,190,NMNB14/14**.
It keeps19000 literal inputs from689:676's direct same-shot human road pose
through5400 followed by the private Probe-focus revision2 through Keine. The
remaining public tail uses H256/publication64/replan32/cash4/human5, original
grid, ordinary charging0 and reserve1.2/120. It is a combined profile at the
same fixed9098 entry, without reference world/RNG transplantation.

The68005-input route costs1156.77s wall,1255003+45 branch/proposal updates and
4654 economic attempts/576 adoptions. Fresh709/natural710 pass exact41-word
terminal, score,14/14 outcome map, zero-miss/bomb and name checks. Its local
selected replay `extra-scarlet-native-nmnb-2888408190-seed9098.rpy` is28604 bytes,
SHA-256 `fa430b2aa91f80e72e7910f63da12d934d5cad91cb4ef9fa22775ad6a3d4b95a`;
the complete action tape hashes
`dc5c68470762843b4c430289ad8d46941b672dd08094fa392f9b19757a089561`.
`extra-scarlet-2888408190-provenance.json` binds the full lineage and environment
above. Earlier local/published selected bytes retain their identities. The
global Extra gate remains unmet.

The next Extra profile716 changes only that post19000 verification horizon
from256 to128, retaining publication64/replan32 and the same seed/prefix/other
controls. It completes **2,939,109,600,NMNB14/14**,71965 inputs,560.362s wall,
659170+21 branch/proposal updates,5043 economic attempts/565 adoptions.
Fresh718/natural719 pass the same complete agreement checks. Its local selected
`extra-scarlet-native-nmnb-2939109600-seed9098.rpy` is30727 bytes, SHA-256
`f104c41a7cd0ffacd4eb23c63c21ed1116ff5a1310a3d6ca64a301a96eb52c3b`;
actions hash `220b05bb0c19657b4224cda9ea2d7557e89fc04311be84dc2fef84e317e82872`.
The preceding prefix/guide production costs remain separately charged.

Story722 completes **5,317,717,330,NMNB35/37**,155037 inputs;10/74 time out.
Its prefix lineage is baseline669 through8205, relative first-stage spell poses
698 through15785, Levea's4096 bomb-free Stage2 input proposals at705, public
Native adaptation through28000, and public H256/publication128/replan64/cash4/
human5/upper-flow1/charge1. The3600s wall budget was declared before711.
Host-memory-aborted711 certifies no result; its committed141312-input tape is
only a proposal. New722 reexecutes the exact committed bytes from original
entry32796 with the same frozen binary, then validates its complete Native tail.
Fresh729/natural731 pass all41 terminal words, score, exact35/37 outcomes,
zero-miss/bomb and name. These remain execution checks, without a retail claim.

Original711 costs1321.190s before abort; clean722 costs145.944s process wall,
of which145.789s is reported planning/simulation wall including prefix execution.
Both owners and all earlier source/guide/transfer trials remain charged. The
retry's147391+1 branch/proposal updates and646 economic attempts/80 adoptions
are not totals for the aborted search, whose branch counters were not saved.
The local selected
`story-scarlet-native-nmnb-4a-finalb-5317717330-seed32796.rpy` is59101 bytes,
SHA-256 `adcf5fcf8d14d1aebff616ff9e5cd770cb4224f29bcd1b6ceffcdc186677a123`;
actions hash `2d6ba5e401cbc5020e9642fda7ee1918fd64466029db2acd6e3386ec89e473b0`.

Separate private36-spell profile723 completes5,262,569,250,NMNB32/37 after the
same kind of original-entry recovery from aborted712's142336 inputs. Only
10/29/47/77/171 time out. Fresh727/natural728 pass all agreement checks.
Its local selected
`story-scarlet-native-nmnb-4a-finalb-5262569250-seed32796.rpy` is58030 bytes,
SHA-256
`75cc6fee62e3f4733c186d9b528dca54acc5d7cb252d84d6a016da6ff557711f`;
actions hash `84be0dfc805a8ecf038944fba4aea0db5f66e78bec6e98ae0eae43996041ec93`.
All selected provenance files bind the producer/manifest, original receipt,
DAT, prefix, binary, natural rows, controls and environment above. Published
Magic milestone bytes remain unchanged;6B and the global gates remain unmet.

The owner subsequently requested publication of all17 selected verified Scarlet
and Magic improvements, followed by retention of new overall project records.
Six later verified records bring this continuation's selected uploads to23.
Their exact bytes are now grouped by shot and route in
[the Native evidence index](../reports/native/README.md); the compact
[progress table](../reports/native/replays/progress.tsv) binds tasks, actual
misses/bombs, capture outcomes, seed receipts, replay/action hashes and source.
Historical272 payloads are relocated unchanged under the paths recorded in
[the relocation table](../reports/native/archive/relocations.tsv). Old recorded
paths in frozen reports remain historical provenance. Earlier local-only labels
below predate this publication request; raw reports, tapes and videos stay local.

Latest overall records are Scarlet Extra **3,087,585,240,14/14** (786,
fresh832/natural833) and complete story **5,821,038,980,35/37**,10/77 timeouts
(930, fresh946/natural947). Both are NMNB, use the original9098/32796 receipts,
and pass all41 terminal words plus natural score/outcomes/name checks. Exact
bytes/hashes and the intermediate3,073,007,650/5,594,687,960 records are in the
progress table. Extra keeps19000 prior inputs and public H128/cash4/movement4;
The preceding5.784B story retains the actual first51253 inputs of H256 task794 through47,
then uses H128/publication64/replan32/cash4/movement4. Its61373-byte replay
has SHA-256 `37ea4e2f9b6ca020a470ab777f700e09aa5fe4d6acffeae3e714dbca4c67d488`,
actions `407e157a43c9baff3d43b5c2cd2748465244b260182c56a2738604c9be17dcc4`.
That5.784B parent remains unchanged. The new5.821B record retains its exact
first142675 inputs through171 entry, then uses H128/cash4/reserve1.2+120.
Its61631-byte replay SHA-256 is
`9b3b6091efbe5414f4eaf27621a8df01cfebd0186dd2a181585b51e59f418c46`, actions
`d00b24d6327d9c1ddbf0a9f85e4cb883f61637eab2e264075a595a46c963cc88`.
The literal prefix preserves unsettled capture rewards and carried world/RNG;
no component scores or states are combined arithmetically. These are
whole Native continuations, not retail equivalence or isolated speed evidence.
The [experiment journal](experiments/2026-10-27-vps.md) owns complete costs,
matched-profile comparisons, resource guards, negative results and future work.
Normal831 passes60/60 after resource-aborted803; no gameplay binary is relinked
during either test owner. The all-shot replay gates and current6.5B milestone
remain unmet. [Per-phase evidence](experiments/2026-10-27-phase-composition.md)
records the partial experiments and full-route combinations separately.

The current Scarlet bests extend those lineages without changing the public
runtime physics or the original9098/32796 receipts:

| Local selected replay | Bytes | SHA-256 |
| --- | ---: | --- |
| `extra-scarlet-native-nmnb-2969055340-seed9098.rpy` |28059 |`d8dc7835480d11d47cbf5e8e5aafbfe51b966a3f5f778e689ebb6bfc4b80d2d7` |
| `extra-scarlet-native-nmnb-2979717840-seed9098.rpy` |27959 |`1b4994b28fd6899ae76dc7114cfc53add97cc765143f83197d10fca192555897` |
| `story-scarlet-native-nmnb-4a-finalb-5383092450-seed32796.rpy` |59253 |`91801eb38523d453dcc5ba90add7cd867ceebb38309672cef2eac680476eaaf3` |

Extra734 uses720's first5400 inputs (a freshly executed shorter3200-input human
proposal followed by Native pose guidance/recovery), Probe2 at726 through19000,
then public H128/publication64/replan32/cash4. It completes2,969,055,340,NMNB14/14;
fresh741/natural742 pass. Its66925-input action tape hashes
`e84d0cd530a02883c160c4aea6e0c46c5c582cbc48aeea1e679dec25a92a4ee5`.
The failed full human input transfer717 and resource-aborted721 certify no
prefix and remain charged; new owners always reexecute from original entry.

Extra744 keeps734's first60958 inputs, then uses the private Extra205-only
movement cash comparison described in Status. It completes **2,979,717,840,
NMNB14/14**; fresh747/natural749 pass all41 words, exact score/outcomes,
zero misses/bombs and name. The66925-input action tape hashes
`d7d168ff516a6c9a4ec389818a425b44ac698e2916d20a01806449b1a340f91a`.
The tail charges105.505s,108352+1 branch/proposal updates,493 fire/form attempts/
65 adoptions and620 movement attempts/46 adoptions. This cost is additional to
734's499.38s and earlier prefix/guide/abort/build/selection costs; it is not a
105-second whole-route solve. The private comparison changes proposals only,
keeps the original shared economic/total budgets and does not claim independent
physics equivalence. The public opt-in control is now `--economic-movement 0..4`;
real-data754 reproduces the entire744 tape and terminal exactly. Focused756 and
normal767 (60/60) pass. Comment-only rebuild780 preserves the tested binary SHA-256
`23c0ad8d01f9cf09b055b71911f5ad2859a31ffe9ebefcba37e2b2a0a99f9a4f`.
Resource-aborted normal760 remains charged and does not count as a test pass.

Story737 keeps724's35209 first-two-stage inputs, uses the same-shot Stage3 road
pose guide/adaptation733 through45000, then public H256/publication128/replan64/
cash4/human5/upper-flow1/charge1. It completes **5,383,092,450,NMNB35/37**;
10/74 time out. Fresh748/natural750 pass all complete agreement checks.
Its154837 inputs cost1214.64s,1446574+15 branch/proposal updates and5163 economic
attempts/690 adoptions, plus all earlier prefix, aborted713 and recovery724
costs. Actions hash
`a9023eb9790aa13069701504170e618a39b0be8fc7dc246c001a752b368c66da`.
All selected files/provenance remain under the local directory above. Earlier
selected bytes remain unchanged. New verified selected improvements are:

| Local selected replay | Bytes | SHA-256 |
| --- | ---: | --- |
| `extra-scarlet-native-nmnb-2980273700-seed9098.rpy` |27311 |`73d06b933b7ea7bf583c88cb88e385796fa0885ee25300b8e0f3d2c619c1e629` |
| `extra-scarlet-native-nmnb-3072999650-seed9098.rpy` |28983 |`d4ce7c74d70c0e987dc6f307e1a6904146212bfd0a35e0600fc8e50dd8bbdb95` |
| `story-scarlet-native-nmnb-4a-finalb-5544203730-seed32796.rpy` |59400 |`82dda92a2e8309eac5510b01213cc891557d17e3641690254bf2d533d96b18bf` |

Tasks761/769/768 produce these three complete NMNB results; their fresh checks
765/772/773 and natural checks766/774/775 pass all41 terminal words, capture
outcomes, natural completion, exact score, zero misses/bombs and `ysopwnab`.
Their action SHA-256 values are respectively
`ded4571c2a09a05b396e06c5095df97330b468b2c3d325c6b94fdb36eeb80389`,
`624e6bd7ca381e46184f47e45180862f24b1f344c71b436a087848d39ec6edb8` and
`30186d0edb01bb9622124f6e0f15499965f0bb2ab1238427ef6fc49e94cf36d3`.
769 follows763's second-road proposal and freshly reexecutes the committed33792
inputs of resource-aborted764;768 freshly reexecutes126976 inputs of aborted759.
Original aborts certify nothing and remain charged. Status owns full lineage,
profiles and separate original/recovery costs. Scarlet3.073B now leads Native
Extra; Magic5.551B still leads story. The6B and all-shot gates remain unmet.

New scoring exports default to `ysopwnab`, the owner's selected eight ASCII
bytes. The original `ReplayData::playerName[8]` is followed by reserved and
practice-mode bytes; longer strings are rejected instead of overwriting mode.
`--replay-name` accepts1..8 identifier bytes. The checker reports the eight bytes
as `player_name_hex` (`79736f70776e6162` for this name), avoiding arbitrary binary
or legacy-encoded text in JSON. Historical selected replay bytes remain unchanged.

Use `--require-all-captures 1 --verify-captures 1 --replay-name ysopwnab`
and `--target-score DISPLAYED` for the active campaign.
The report separates game completion, capture coverage, exported replay and
`objective_met`. Exit3 means capture failure; exit5 means a complete qualifying
NMNB/capture run below the explicit score target. Such below-target replays may
be exported for Native progress, but do not pass the retail-playback gate.

### Offline compute budgets and checkpoint continuation

The owner now prioritizes complete score before optimizing solve speed. Native
`th08_shadow_style` records these independent controls; defaults are preserved:

| Control | Default | Accepted Native range |
| --- | ---: | --- |
| `--verify-horizon` |64 | Powers of two64..2048 |
| `--publish-horizon` |64 | Powers of two64..2048, at most verification |
| `--search-budget-ms` |110 |1..60000 per search |
| `--lead-updates` |8 |1..32, below publication |
| `--nominal-warm-updates` |0 |0 means full old tail; otherwise lead..verification |
| `--replan-updates` |8 |1..publication |
| `--policy-horizon` |24 |2..200, with lead+horizon-1 at most verification |
| `--policy-beam` |192 |1..8192 |
| `--verify-captures` |0 |0/1;1 requires the all-capture run goal |
| `--drop-target` |0 |0/1, current source enemy target; no budget/physics change |
| `--economic-proposals` |0 |0..4 extra full-Native horizon fire/focus comparisons, sharing total search budget |
| `--economic-movement` |0 |0..4 active-spell movement comparisons; independent of fire/focus comparisons |
| `--checkpoint-updates` |0 |0 disables;64..400000 executed inputs between prefix saves |
| Positional `WALL_SECONDS` |Explicit |1..7200 per owner attempt |

For example, H512/publication256,5000ms, lead1/nominal1 and interval128 spends
more computation on freshly guided future actions. All proposed actions still
execute through the owned Native world. A certified block may be consumed while
the owner is synchronous; no-certificate retry spacing remains at most8 updates.
The normal proposal budget remains60ms. Record complete score/outcome and work
before deciding whether the larger profile helps.

`--economic-movement N` compares up toN differing cardinal pulses in stable
up/down/left/right order, only while a spell is active and dialogue is absent.
Each pulse keeps the certified lead and the incumbent's fire/form bits, changes
only direction for16 Native updates, then continues with ordinary guidance in
its own Native world. Fire/form alternatives run first; both kinds share the
existing2000ms economic admission window within the total search budget. An
admitted Native future can use the remaining total budget, as with the existing
fire/form comparison;2000ms is not a hard per-rollout deadline. The new control
does not require `--economic-proposals` to be enabled. Zero keeps the old path.
Only certified complete-horizon/terminal futures compete; strictly higher raw
cash adopts, while stable ties retain the incumbent. This horizon ranking is
not a guarantee of capture or higher complete-route score. Reports record the
requested count/revision, separate `motion_economic_attempts`/`adoptions`, and
charge every extra attempt to the existing branch counters and timing.

Append `--prefix-actions PATH --prefix-updates N` to continue after an explicit
input checkpoint. The original entry configuration, seed and sampled input
profile must match its provenance. The tool executes and observes all firstN
inputs, then plans from the same owned world without resetting RNG, carried
shots/items, actors or lifecycle state. Prefix reconstruction is included in
owner updates and Native work. Both options are required together; short files,
invalid NMNB actions and an actual terminal beforeN are rejected. The output
contains the full prefix plus regenerated continuation, with requested/executed
prefix counts for auditing. All generated tapes/replays/reports stay local until
a selected progress replay is authorized for publication.

With `--checkpoint-updates N`, the owner appends only actually consumed inputs
to `checkpoint.inputs.actions`, then flushes and replaces `checkpoint.json`
everyN inputs and at an ordinary exit. Branch predictions never write this
tape. The default writes neither file. Units are owner input updates, including
prefix reconstruction, rather than natural replay row indices. No simulation
state, RNG or search memory is serialized. Reports include the interval,
committed update/byte counts and checkpoint I/O nanoseconds; the wall budget
charges this I/O, while Native simulation timing excludes it.

After a process kill, read the last metadata and copy exactly the first
`saved_bytes` of the tape to a new file: trailing bytes may be uncommitted.
Pass that file with `--prefix-actions` and its `saved_updates` with
`--prefix-updates`, at the same original entry and fixed seed. The new owner
must freshly execute the entire literal prefix and validate its own futures.
The metadata states `gameplay_certificate:false`; persistence does not certify
safety, a capture or a complete result, and does not rehabilitate a failed or
resource-aborted owner. Flush/rename supports process-termination recovery;
there is no fsync or machine-crash durability claim.

Focused692 passes the complete real-DAT test in144.62s: final saved/final tape
equality, default-off absence, and an exact1024-input budget-limited owner that
exports no replay. Controlled SIGTERM695 preserves19584 inputs/80439 bytes,
with no result or replay. Fresh696 reexecutes that committed prefix at the
original Scarlet9098 entry and returns budget with19584 updates,0 deaths and
No Bomb. These are partial-duration recovery checks, not scoring certificates.
Normal693 passes **60/60 in1207.78s**. Complete affected-data Extra694 and its
fresh709/natural710 checks pass; the saved tape exactly equals all68005 final
inputs/274915 bytes. Its83,431,757ns checkpoint I/O remains charged within
1156.77s wall. All new owners use the authorized4GiB process/tree bounds and
unchanged host12/8GiB guards; the normal suite peaks at1004314624 bytes tree RSS.

These checkpoint/compute controls pass the normal Release Native/DAT suite57/57
in970.95s. The focused checkpoint regression passes1/1 in35.65s and verifies
literal-prefix retention plus a regenerated continuation with H512/publication256,
lead1/nominal1/interval128/2000ms, fresh exact terminal execution and natural
original-writer replay. Invalid schedules and an overlong terminal prefix fail.

The capture-aware/wider-guidance follow-up passes the normal Release Native/DAT
suite **59/59 in919.31s**, with frozen test binaries. The checkpoint regression
now also uses policyH48/beam256 and capture verification. A source ordinary-spell
timeout fixture demonstrates that safety-only survival certifies while the
capture goal rejects the same Native trajectory, preserving all41 fields until
the stop and leaving the parent world/audit unchanged. Its explicit synthetic
invulnerability isolates capture semantics and is not NMNB gameplay evidence.
A selected-story replay regression covers Stage1→2, including a read-only
projection of the recording owner's carried clear flag; it checks the previous
catalogue without falsely failing the unfinished new one. The H48 adapter test
checks bounded single-action proposals and a complete Native guided horizon.

### Complete-tape refinement

`th08_score_refine` requires our own complete Native `plan.actions` plus its
original entry configuration. Defaults require NMNB/all-captures. An incomplete,
dead or interrupted incumbent is rejected before owner execution.
The source audit includes gated Last Spells. Example for the saved Extra baseline:

```sh
build-extra-live/th08_score_refine game_data_donottrack/th08.dat \
  reports/local/extra-refine 8 4 1 10025 1 0 \
  reports/local/native-score-offline-20261026/extra-native-baseline-v1/run/plan.actions \
  --family form --trials 96 --window 240 --interval 1800 \
  --timeout-ms 30000 --wall-seconds 1200 --target-score 3021639920
```

Positional flags after the seed are story and prior-Final-A history. Use
`0 3 1 40755 1 1` for the full story configuration. `--family fire` explores
fire-on/released-fire windows instead. Every trial completes the actual remaining
Native trajectory; only strict final-score improvements retaining NMNB, the
same capture catalogue and final route replace the incumbent. Exit2 rejects an
invalid incumbent, recording its status and cost in `result.json`; exit5 exports
a valid progress replay below the requested target. The chosen name remains
`ysopwnab`. Fresh tape following and natural `.rpy` checks remain required.

### Bomb and capture constraints

The offline planner `th08_shadow_style` and complete-tape refiner expose two
independent controls:

| Goal | Planner controls | Refiner controls |
| --- | --- | --- |
| Original NMNB/all captures | `--allow-bombs 0 --require-all-captures 1 --verify-captures 1` | `--allow-bombs 0 --require-all-captures 1` |
| Current No Miss, No Bomb, captures optional | `--allow-bombs 0 --require-all-captures 0 --verify-captures 0` | `--allow-bombs 0 --require-all-captures 0` |
| Available opt-in, currently paused: bombs permitted, captures optional | `--allow-bombs 1 --require-all-captures 0 --verify-captures 0` | `--allow-bombs 1 --require-all-captures 0` |

Bomb permission defaults off. Refiner capture coverage defaults on; the planner's
existing capture controls are explicit in campaign specifications. No Miss is
mandatory. The planner tries one-update human/focused bomb prefixes after ordinary
recovery fails, with the immutable lead retained. Native source decides inventory,
lockouts, form, cancellation, capture invalidation and invulnerability. Bomb
permission does not guarantee a useful bomb or a higher complete score.

For the capture-optional Native planner, `--farm-to-timeout 1 --attack-pacing 1`
allows a measured profitable farm to continue past the capture reserve. It
requires `--require-all-captures 0 --verify-captures 0`; the active campaign also
passes `--allow-bombs 0`. Every240 updates the income sample refreshes, and zero
income restores fire. This is a default-off proposal preference, not a promise
of timeout or scoring profit. Reports record `farm_to_timeout` and its revision;
the source audit owns actual timeout/capture outcomes. Historical benchmarks and
their original controls keep their identities.

`--farm-charge-fire 1 --attack-pacing 1` is a separate default-off Native
experiment. It fires only during the initial Sample gauge preparation, then
releases fire for the ordinary realized-income sample. It preserves the capture
reserve and bounded wait. The report records `farm_charge_fire` and
`farm_charge_fire_revision`; complete score and actual capture outcomes decide
whether the preparation helped. [Planner](PLANNER.md#stream-proposals-and-attack-pacing)
owns the fire-intent priority and sample boundary.

Native `--upper-flow 1` requires `--attack-pacing 1` or `--survival-flow 1`.
It expands stream proposal targets from Y208..400 to Y16..400, defaults off,
and records `upper_flow`, `flow_grid_revision` and `flow_grid_min_y`. No early
damage-Probe approach is added. Native still owns boss bodies, collision and
realized income. [Planner](PLANNER.md#stream-proposals-and-attack-pacing) owns
the active-cell order, target cadence and unchanged default behavior.

The default-off `--drop-follow 1 --drop-target 1` experiment proposes a position
below current drop-bearing stage actors after their reduction window ends.
Reports record `drop_follow` and its revision. It retains the same NMNB/capture
controls above and changes no source damage, collision or reward rule. Inspect
complete terminal score and per-stage item production/price together; a higher
kill count alone cannot select a replay. [Planner](PLANNER.md#price-aware-positioning)
owns coordinates, ranking, priority and snapshot lifetime.

Native `--parent-chain-bias 100 --drop-target 1` names the first adaptive parent
experiment. Zero restores the historical proposal. Positive values exclude
attached-child lanes and prefer focused shots at a root with a current chain,
temporarily superseding its human/hit-income goal; actual item trips still win.
Revision2 waits until fewer than24 root-reduction updates remain and yields
form to any full pending pickup batch, including when bullet density delays
the collection movement gate. The snapshot copies only the signed update count.
Reports include `parent_chain_focus_bias` and `parent_chain_revision`. The value
is a soft preference cost, not a forced form or a safety certificate. Keep the
original seed, literal prefix and all Native rollout costs; evaluate complete
score, exact capture outcomes and source point production. The root cue is
snapshot-only and predicts no guaranteed child reward.

Refiner `--family bomb --allow-bombs 1` compares human/focused X windows against
the complete incumbent terminal. Use `--window 1` for a one-update pulse and
`--first-update N` to begin trials at a declared owner-input index (default64).
Every edit still needs a complete no-miss tail, the original final route and
strictly higher actual terminal score. Restoring either constraint rejects tapes
that violate it; no partial result is certified. `result.json` records both
controls, actual `bombs_used`, capture counts and trial costs. `score-audit.json`
records every encountered/missing spell and its outcome, including failed captures.

Fresh verification of a bomb tape needs follower `--allow-bombs 1`; its default
input parser and process transport continue to reject X. The original-writer
replay checker already accepts human bomb replays. Omit `--require-all-captures`
for the relaxed replay check, and compare no-miss state, score, actual bomb count,
capture outcomes and name against the producer. The focused
`headless_bomb_scoring` CTest covers this complete scene and both strict rejections.

### Slowdown metadata in Native exports

The original writer computes slowdown from the supervisor's accumulated lag
numerator/denominator. `Supervisor::CalculateFps(shouldDraw)` updates these
counters through the drawing/FPS path. Headless `Session::step` deliberately
executes only the calculation chain, so both counters remain0; the writer's
0/0 produces IEEE754 NaN. This is unavailable rendering-performance metadata,
not a measurement of Native solver time or retail playback speed. The slowdown
fields are not read by the gameplay replay executor.

Silent Selene's [uploaded Native Extra](https://www.silentselene.net/replays/th08/89742)
therefore displays `nan%`; its downloaded bytes have the exact selected replay
SHA-256 below. The [human comparison](https://www.silentselene.net/replays/th08/54125)
displays0.3102%. `th08_replay_check` exposes finite values as `slowdown_percent`,
nonfinite values as JSON `null`, and preserves `slowdown_ieee754_bits` for
diagnosis. Selected replay bytes are preserved; no measured0% is invented.
The downloaded human replay decodes to0.31015873%, matching the site's rounded
display, and completes naturally in Native. Its SHA-256 is
`78e559c1deb8fca62941b1ee4ade69ecc9ca8bf6ec2acd99882e6794268a7e92`;
the external replay remains local.

## October27 verified Native progress

The requested Native thresholds are achieved: **Extra3,010,040,230,NMNB14/14**
and **complete1→2→3→4A→5→FinalB5,025,195,380,NMNB37/37**. Both milestone exports
capture every eligible spell, despite the capture-optional campaign goal.
Original retail receipts46997/1805 and the exact `ysopwnab` name are preserved.
Fresh original-entry execution matches all41 terminal words; natural original-
writer replay agrees on score, zero misses/bombs, exact captures and name.
This is Native execution agreement. Both separate human-reference retail gates
remain unmet, so no Wine playback is launched.

The owner selected these two exact files for publication on October27. Copies
under `reports/native/replays/` retain the verified bytes; the original local
`reports/local/target-3b5b-20261027/selected/provenance.json` and producer
directories remain the full execution evidence:

| Published replay | Bytes | SHA-256 |
| --- | ---: | --- |
| [Extra3.010B](../reports/native/replays/magic/extra/extra-native-nmnb-3010040230-seed46997.rpy) |30331 |`d5d95ec115afee3ef2c301b378af9385e77d9b2e89edae572641146e24b8aad5` |
| [Complete story5.025B](../reports/native/replays/magic/lunatic/story-native-nmnb-4a-finalb-5025195380-seed1805.rpy) |55241 |`1d53cd6a624a3e0821f11951ec9c1fd24752c76894b76908eea8451a5ead7dbc` |

Extra is entirely produced by the public implementation. First,
`target-extra-firststage-human100-v1/` starts from the proper retail-seeded entry
and executes5400 inputs with human bias100/drop-follow. Next,
`target-extra-public-early-keine-v1/` retains those5400, restores human5 and uses
reserve1/0 through17000, capturing all three Keine spells. Finally,
`target-extra-public-early-full-tail-v1/` retains17000 and restores1.2/120 for the
complete tail. All use H128/publication64/replan32, lead1/warm1/beam384/30s search,
attack/survival focus10, priced/upper/hit-income/drop goals and cash4; timeout
farming, bombs and parent bias remain off. No private parent prefix is inherited.
Its full69957-input tape SHA-256 is
`a4f81471ae49061b1e850acfaf6a97184e063d39f62f46d9662e2e2590c5a35b`.

Story producer `target-story-captured77-cash-h256-p64-v2/` retains106321 inputs
of the verified all-capture4.708B route (see its earlier provenance below), then
uses parent100 revision2/H256/publication64/replan32/cash4, reserve1.5/240,
capture-fire recovery, lead1/warm1/beam384/30s and ordinary human/farm focus5.
The full146229-input tape SHA-256 is
`0e783f185b5821232f9ddfa5911109d7fbfe11d566e3a7c06c45c014e4c3954a`.
Each producer's `verification/{verified.json,identities.json}` binds the exact
source/binary/receipt/prefix/DAT/argument and fresh/natural outputs. Original
selected historical files retain their bytes. Normal Release Native/DAT CTest
passes60/60 in1133.27s for the implementation; final documentation also passes.

The preceding verified bests are Extra2,979,939,680,NMNB14/14 and complete
story4,827,562,920,NMNB36/37. Only card77 times out in that story; the earlier
all-capture route reaches4,708,495,930,37/37. Those earlier3B/5B attempts had not
yet reached the requested milestones. Seed receipts46997/1805, full-entry history and `ysopwnab` are
unchanged. No Wine playback is launched.

### Global-goal continuation: 5.551B story

Task620 improves the complete Magic Team route to **5,550,599,270,NMNB36/37**;
only47 times out. It retains the5.458B tape's first52561 inputs, executes the
Stage4A human-pose guide at618 through60000, then restores the public
H256/publication128/replan64/cash4/human5/upper-flow1 tail. Capture pacing is
normal; no bombs are allowed. Source references are immutable XY/fire/form
hints from Alan's bomb-free Stage4A gameplay1..6500, not source RNG, bullets,
HP or actor state. The private guide owns a temporary current-snapshot baseline
proposal every8 inputs (H24/beam384/anchor0.08/focus±10); independent recovery
and complete Native futures remain the safety authority. Its exact source,
reference/header hashes and current-ABI link inputs remain frozen locally.

The actual148021-input route charges1019.43s wall,1105467+23 branch/proposal
updates,4171 economic attempts/556 adoptions. Fresh634 and natural638 pass
exact41-word terminal, score,36/37 outcome map, zero-miss/bomb and eight-byte
name agreement. This is execution agreement, not independent physics or retail
equivalence. The seed1805 receipt, DAT, Release environment and resource
bounds remain those below. The6B/all-shot gates remain unmet.

Local selected replay
`reports/local/global-score-20261027/selected/story-native-nmnb-4a-finalb-5550599270-seed1805.rpy`
is58519 bytes, SHA-256
`7ef6066e5e4ec84e0a6e50f31c7f222935bd76b2c8e3c6f52c4b9234959d4eb2`.
Its complete action tape hashes
`8e0d2125b06263eb094326a4dea1f285340c94bc3fa37b54bff55254eec7f552`.
The selected provenance JSON preserves the producer, prefix, receipt, source,
binary, DAT, natural-row identities and verification results. Published milestone
replays keep their original bytes. Public code remains `79882c6`, tested by
normal59160/60 and complete real-data592; the pose adapters remain private.

### Global-goal continuation: 5.458B story

At that checkpoint, selected Native bests are **Extra3,013,913,970,NMNB14/14** and
**complete Magic Team4A→FinalB5,457,960,010,NMNB35/37**. The separate
all-capture story is **5,107,553,670,NMNB37/37**. These use the original retail
episodes Extra46997/story1805, proper non-practice entries, No Bomb and
`ysopwnab`. Story47/74 time out in the highest-score route. The owner-selected
published3.010B/5.025B milestone bytes remain intact; these newer exports stay
under ignored `reports/local/global-score-20261027/selected/`.

| Selected local replay | Bytes | SHA-256 |
| --- | ---: | --- |
| `extra-native-nmnb-3013913970-seed46997.rpy` |30326 |`0015f7c7b132f4febd114d9ff0737c4852726c306422fd011f1708bdccf72b2b` |
| `story-native-nmnb-4a-finalb-5457960010-seed1805.rpy` |56800 |`387269153db836f15728d8699694da138f6a73093e3d629a8ee3596854ee8994` |
| `story-native-nmnb-4a-finalb-5107553670-seed1805.rpy` |53961 |`2c192a7c6289f89310eacb0a8d6cb263ca450e2de1fb093528d6001b853a7cd7` |

Task597 keeps63908 literal inputs of the published Extra lineage and enables
only `--upper-flow 1` for the terminal205 tail: H128/publication64/replan32,
cash4, reserve1.2/120, focus10, preserve-fire recovery. Its complete69957-input
tape hashes `62598246791f1ab6ca1aef0d80dc666d9105f8816e04f1168b39e4ad15c217e2`.
Wall time is74.4912s,64363+2 branch/proposal updates,498 economic attempts/54
adoptions. Fresh603 and natural606 pass all terminal, score,14/14 outcomes,
zero-miss/bomb and eight-byte name checks.

Task595 retains49286 literal inputs from the Stage3 human-road/upper47 scene.
Its lineage is task503's14529 first-stage inputs, task567's first32721 inputs,
3500 bomb-free Alan Stage3 gameplay inputs executed at our own seed in578,
Native adaptation through580, and585's complete upper Sample47 timeout. Failed
4096-input transfer576 certifies no prefix;578 freshly executes the shorter
proposal from original entry. Task592's public `--upper-flow 1` reproduces the
complete585 tape and41-word terminal exactly. The585 standalone binary/source
correction is retained explicitly in `source-provenance-correction.json` and
`frozen-bin/online_native_at_link.cpp`, alongside the untouched original manifest.
No reference RNG or world state is transplanted; no old object is relinked
against the changed policy header.

After that prefix,595 uses public H256/publication128/replan64/cash4/human5,
upper-flow1, charge-fire0, normal capture pacing and reserve1.5/240. Its
146853-input tape hashes
`d59f144c7943dff00f98b134d5e4defde7cf30a65a032d03eef170105097cfea`.
It charges978.846s wall,1248015+23 branch/proposal updates,4611 economic
attempts/621 adoptions,2241 rejected future deaths, and zero search timeouts,
budget overruns or resource aborts. Fresh600/natural602 agree exactly, including
the35/37 capture map. It is a complete combined profile; partial gains are not
added to terminal scores.

All-capture task575 uses the successful Stage2 human-road4096-input proposal
and Native adaptation through26000, followed by H256/publication256/replan128,
cash4/human5, original grid and reserve1.5/240. It completes143109 inputs in
689.318s,750113+13 branch/proposal updates,2752 economic attempts/404 adoptions.
Fresh588/natural589 pass. Its tape hashes
`c00240006d7450faf4da2db01738e4d313364c1e625746e1bce0fbaf17a7baca`.

Selected provenance JSONs retain the full controls, exact producer/prefix,
binary, receipt, DAT and natural-row hashes. Environment remains Release
GCC12.2.0, C++17, `-ffp-contract=off`, no fast-math, AMD EPYC7B12 and
Linux6.1.0-54-cloud-amd64 x86_64. Reported Native timing excludes file I/O;
the complete wall cost is also charged. Each owner obeys the same2GiB limits
and host memory guards. Fresh execution and source natural replay establish
execution agreement, not an independent physics oracle or retail equivalence.
Neither the6B milestone nor either all-shot human reference is reached; no
retail replay playback is qualified. Focused590 passes2/2; normal Release
Native/DAT591 passes60/60 in1092.11s;592 covers the complete real-data47 scene.

### Global-goal continuation: 5.163B story

Task568 improves the complete Magic Team4A→FinalB route to **5,162,703,270,
0 misses,0 bombs,35/37 captures**. Only32/77 time out; all37 eligible cards are
encountered. At that point the37/37 best is5,025,195,380. Original retail seed1805,
entry receipt SHA-256
`88516cae49c9c04a2ebc0857f88eb81b3d0ed7277ff4c4c95a1a8b39d55ae133`
and `ysopwnab` remain unchanged. Both6B and the7,053,332,460 all-shot reference
are still above this score; no retail playback is qualified.

This lineage uses public code throughout. Task503 supplies the first14529
inputs: human100, charge-fire1, H128/publication64/replan32, no cash comparison.
Task563 retains those inputs and uses human100/charge-fire0/H256/publication256/
replan128/cash4 through26000. Task568 retains that literal26000-input tape,
restores human5 and uses H256/publication128/replan64/cash4 for the complete tail.
The economic edit window stays32 updates. Common controls include parent100
revision2, drop-follow, reserve1.5/240, capture-fire recovery, lead1/warm1,
beam384 and30s total search. The record does not inherit the human-button
transfer experiments or private boss/age/window prototypes.

Producer `global-story-public-earlycash-h5-p128-r64-full-v1/` is under the
existing local campaign directory. It records142709 complete owner inputs,
1329.203s wall time,1506421+21 branch/proposal updates,2395 rejected future
deaths,5452 economic attempts/770 adoptions and zero timeouts/overruns/resource
aborts. Measurements use Release GCC12.2.0, C++17, `-ffp-contract=off`, AMD
EPYC7B12 and Linux6.1.0-54-cloud-amd64 x86_64. The report separates Native,
guidance and file-I/O time; wall time is the charged owner cost.

Tasks577/579 verify exact fresh41-word terminal agreement, complete natural
original-writer replay, score, no misses/bombs, the exact35/37 outcome map and
name. This remains execution agreement, rather than an independent physics
oracle or retail equivalence. Public source is unchanged from the tested
`de87af9` controls; normal Release Native/DAT task504 passes60/60 in1099.82s.

The exact selected file stays local at
`reports/local/global-score-20261027/selected/story-native-nmnb-4a-finalb-5162703270-seed1805.rpy`:

| Local artifact | Bytes / SHA-256 |
| --- | --- |
| Replay |54682 / `70c3ddb6e933d9040cf1d82fbe5e598950e30119b2e143345ea3e2e6902846dc` |
| Complete action tape |`6f3e6ec003e4bb155844d36128a557ef55b3c8e2dd3ec6f5c74bc5889c6285a3` |

The selected provenance JSON and producer's `verification/` retain all binary,
DAT, prefix, receipt and natural-row hashes. Earlier published bytes remain intact.

The earlier `reports/local/target-3b5b-20261027/selected/` directory also
contains these exact exports:

| Local replay | Bytes | SHA-256 |
| --- | ---: | --- |
| `extra-native-nmnb-2979939680-seed46997.rpy` |29446 |`5f279c472bea6e148ce151a292aca59735ef6d478a899e39606ec857544bfe6c` |
| `extra-native-nmnb-2978088160-seed46997.rpy` |29453 |`2b060437ede1cd9f81da33d97331faa0703ab6dc99744751f381807207f24f68` |
| `story-native-nmnb-4a-finalb-4827562920-seed1805.rpy` |54042 |`cd86d2785d0f9c08ada25d3c774ff1d8af6213eb8eff531b787f8e413c13cde3` |
| `story-native-nmnb-4a-finalb-4708495930-seed1805.rpy` |52909 |`ff591f56d44dc2853bb9e4989948f72df0d3151b65b8e74f8f2c1410a8056c70` |

`target-extra-early-income-follow-tail-v1/` retains16187 inputs (card194 end+200)
of the measured193-farm2.891B tape, then uses H128/publication64/replan32,
focus10/reserve1.2/120/drop-follow/cash4. `target-extra-last205-flow-v1/` retains
63200 of its inputs and enables survival flow for the final205 continuation.
The full69277-input tapes hash to
`51051ff465852e48e77ed23499bdbb4cbba104193afc63204b2ea474cf462afa` and
`41abb2efdf0ebe998b18da3d1bc960f3c79a13cdeed8b6e3f68ca590c5f91e58`.
The inherited early prefix keeps its earlier private pulse/repair provenance;
this does not claim a new public full-entry parent.

`target-story-parent-full-v1/` retains14409 inputs of the4.469B tape and uses
the first parent100 proposal. `target-story-parent-finalb-cash-h256-v2/`
retains its first105673 inputs, including the ordinary Last Spell77 timeout,
then uses revision2 parent100/H256/publication128/replan64/cash4. The new gates
preserve root reduction income and pending-item human form. Its full144885-input
tape hashes to `727d41b17ec8839e39402e233dd836664f290b482a06c9f9e5605e9526d920a1`.
The all-capture `target-story-parent-full-v2/` instead retains14409 original
inputs and applies revision2/H128/publication64/replan32 without cash comparisons;
its full145197-input tape hashes to
`f4533b8ac8a50dd07a89c839a6002f7e7b48d690aa6e740da6ccc9989d969dfb`.
Both use reserve1.5/240 and capture-fire recovery. Each selected export has fresh
original-entry exact41-word terminal and natural original-writer
score/NMNB/exact spell outcomes/name agreement. These remain execution checks,
not an independent physics or retail oracle. Revision2 passes normal Release
Native/DAT CTest60/60 in1133.27s; the obsolete revision1 test owner is explicitly
interrupted and charged, not reported as passed. New artifacts remain local.

Earlier source-guided continuations reach Extra2,891,728,530,NMNB14/14
and complete story4,469,021,890,NMNB37/37. Neither reaches the
owner's new3B/5B milestone or its historical human reference. Seed receipts,
entry/history and `ysopwnab` are unchanged; no Wine replay is launched.

These selected local copies are under
`reports/local/target-3b5b-20261027/selected/`, with exact producer/verification
identities in `provenance.json`:

| Local replay | Bytes | SHA-256 |
| --- | ---: | --- |
| `extra-native-nmnb-2891728530-seed46997.rpy` |30719 |`c2d4d016e324b72d768f2ffca6db5d45e6cce16cd5ec2bbe9331e86caae7067b` |
| `extra-native-nmnb-2825597670-seed46997.rpy` |29008 |`4365ce22a3749d76e49a47eb4a18319d712452a10a4d19bd1c16fe0296331dde` |
| `story-native-nmnb-4a-finalb-4469021890-seed1805.rpy` |52502 |`6d58e47e4aa2c5a80071a686470d8f542c235f58256097302e28610557423b19` |

Producers `target-{extra,story}-drop-follow-v1/` are under the existing local
campaign directory. Extra retains14026 literal inputs of the2.650B tape and
uses H128/publication64/replan32, beam384/30s search, lead1/warm1, focus/survival
bias10, reserve1.2/120 and four economic alternatives. Story retains14409 inputs
of the4.444B tape, with the same horizon/schedule/beam/budget, bias5, reserve1.5/
240 and capture-fire recovery. Both enable drop-follow, retain the priced/upper/
hit-income goals and disable timeout farming/bombs. Their full69677/144837-input
tapes hash to `b566ce5274be7809d12bc93e2837505b0c9afc5f7394b744f42de59bde5579ac`
and `0e2b94297b23e6d8da160267692e0328777dcd97e0e8889a4568d3fc4b0894da`.
Fresh original-entry exact41-word terminal and natural original-writer
score/NMNB/exact capture outcomes/name all agree. These are Native execution
checks, not an independent physics or retail oracle. New bytes remain local.

`target-extra-card193-income-v1/` retains9400 original2.650B inputs and enables
timeout farming only until11900. `target-extra-card193-capture-tail-v1/` retains
that explicit safe prefix, then restores ordinary capture pacing with the same
H128/focus10/reserve1.2/120 profile and drop-follow disabled. It captures193/194
late and completes2.891B with14/14. Its full71253-input tape SHA-256 is
`5e8afbc5091100d801022f71d50b691ed545567c02d97e9c749241b1f0ae2783`.
Both setup and complete-tail costs remain charged; fresh/natural checks pass.
Normal Release Native/DAT CTest passes60/60 in1214.34s for these public controls.

The preceding No Miss/No Bomb campaign allows missing captures. Extra improves
to2,650,283,240,14/14; the complete1→2→3→4A→5→FinalB story improves to
4,444,083,080,36/37, with only ordinary card58 timing out. Its all-capture
alternative reaches4,403,879,050,37/37. All keep the original
retail seed receipts (46997/1805), entry/history and eight-byte `ysopwnab` name.
Both remain below their human-reference targets; no Wine replay is launched.

New artifacts remain local under
`reports/local/score-continue-20261027/selected/`, with `provenance.json` and the
original producer/verification directories under the existing local campaign:

| Local replay | Bytes | SHA-256 |
| --- | ---: | --- |
| `extra-native-nmnb-2650283240-seed46997.rpy` |29062 |`335711a86dc773d74e8a85e836fede70adcb0c92b7341c32bf93632154f261dd` |
| `story-native-nmnb-4a-finalb-4444083080-seed1805.rpy` |51805 |`0a6f1cc3b36cd0dcfcd748d16b0815866c3042a686dca9783b90d8000b910386` |
| `story-native-nmnb-4a-finalb-4403879050-seed1805.rpy` |51392 |`00a2e80784e1c5fd67928e4b676bfd4786b9ffb0b5832480a90bb93e9c44c0ad` |
| Earlier `extra-native-nmnb-2624965020-seed46997.rpy` |28892 |`9791826a7e91dcd9e9cf9f652b11a86e127345034352704242ea10988201825c` |

The earlier H256 Extra producer `oct27-extra-strong-focus-tail-v3/` retains60879 inputs from the
verified2.620B tape, then uses H256/publication128/interval64, bias10, reserve1.2/
120 and four bounded cash comparisons. Its complete72597-input tape hashes to
`75bed4fc7161615078b4be763b9248b439ad3c2201b83964b840703348901f5e`.
The final `oct27-extra-strong-focus-h128-v4/` uses the same original prefix and
bias/reserve/comparison controls, H128/publication64/interval32. Its complete
72461-input tape hashes to
`0f4f88e449c6453b4e665e0781803c7849d3b36fe7e69d4c6e1f26ff4aa70bd4`.

Story producer `oct27-story-58-prefix-safe-tail-h128-v3/` reconstructs59476 inputs
from `oct27-story-selective58-timeout-v2/`, which itself retains56276 inputs of
the old4.311B incumbent. The failed later parent remains charged and unused.
The continuation disables timeout farming, with H128/publication64/interval32,
reserve1.5/240 and capture-fire recovery. Its complete143157-input tape hashes to
`c2fe3b2def69a87c23891e7227818ed19ed7772090be3decefe77ac3777c38f6`.
The all-capture `oct27-story-incumbent58-h128-control-v4/` instead retains58764
inputs of the original captured58 incumbent (its source end plus200), with the
same H128 tail configuration. Its complete142845-input tape hashes to
`7c288d5fec1b1c187a47688cdc872eb5825ceb1b988ad6ce646b4de5cb594a43`.
It is a separately charged whole-route counterfactual, not a free score estimate
or a general claim about the value of one spell.

`verification/{verified.json,identities.json}` in all four producer directories
records fresh original-entry exact41-word terminal agreement and natural
original-writer score, no-miss/no-bomb, exact spell outcomes and name agreement.
The local guarded `reports/local/score-continue-20261027/verify-no-bomb.cjs`
compares capture outcomes instead of imposing all captures. Checks are Native
execution agreement, not an independent retail oracle. Earlier tracked selected
replays retain their exact bytes; these new progress files are not published.

## Selected Native progress replays

The owner selected the October26 Native progress files for publication. They
retain the original writer's exact bytes, player name `ysopwnab`, proper
non-practice entry and SampledGameplay. The original two baseline files below
use no prefix repair. All selected progress files have
exact fresh41-word terminal agreement and natural original-writer replay with
zero death/bomb-input rows, matching score and all eligible captures.

| Field | Extra | Lunatic4A→FinalB |
| --- | --- | --- |
| Replay | [Extra2,589,417,330](../reports/native/replays/magic/extra/extra-native-nmnb-2589417330-seed10025.rpy) | [Story3,890,487,560](../reports/native/replays/magic/lunatic/story-native-nmnb-4a-finalb-3890487560-seed40755.rpy) |
| Displayed / stored score |2,589,417,330 /258941733 |3,890,487,560 /389048756 |
| Captures |14/14 |37/37 |
| Initial stage / difficulty / shot / seed |8 /4 /1 /10025 |0 /3 /1 /40755 |
| Prior FinalA clear |false |true |
| Owner updates / solve seconds |69381 /448.547 |119637 /479.403 |
| Replay bytes |26995 |35865 |
| Replay SHA-256 |`5fb88138ce1a301b9695f1b3ebff2039fb36eb046242bbff7c7a590c6de76898` |`6b9bb1a5158946194c336240b2f4209cdf407e298e62cfc1386e0790fb3e9ce4` |
| Natural calculation rows |69377 |119724 |
| Natural25-field row SHA-256 |`eb25c17721fe088bca05e9bfe6a4d2ceb93684173538f92aa2caa4a481223d56` |`903b45fb0945a89ddde66c198ddd4058c738b9412669db0b9f2e5a9fe1fe98bd` |

The common profile uses score-harvest, held/baseline recovery,500ms total/60ms
proposal, priced collection/upper-stage positioning and collection/human bias5.
Extra uses H256/publication128 and pacing1.5*HP/DPS+240. Story uses
H128/publication64 without ordinary pacing. Both retain the original lead8;
these are offline compute profiles. The producers are main parent `d434e7e`
plus this Native replay-name/objective change. Frozen solver SHA-256:
`f03447bad8b1699d483be000c628bdbedffce094995537207583235257250ada`;
source patch SHA-256:
`c5760b747bc4d33b15c42ec78c4669b80817ea216de8571b5449c534ae1ad4e4`.
Fresh follower SHA-256:
`8c591082ee7d9623c133e26af4af91f80034ae52a5af273226dedf96cda4789b`;
checker SHA-256:
`326e24173f55613a8ac782f93c222124b2557d112d2bb12d5f943fa96a7bea92`.
The DAT and1.00d export target are the same as the v15 section below.

Complete local manifests, tapes, reports and rows remain under
`reports/local/native-score-offline-20261026/{extra-native-baseline-v1,story-native-price-v1}/`.
The two baseline files and selected follow-ups below are published. They remain
below the human score target, so retail Wine playback waits. Reproduce natural verification:

```sh
mkdir -p reports/local/native-progress-check
build-extra-live/th08_replay_check game_data_donottrack/th08.dat \
  reports/native/replays/magic/extra/extra-native-nmnb-2589417330-seed10025.rpy \
  reports/local/native-progress-check/extra.rows \
  --score-audit reports/local/native-progress-check/extra-audit.json \
  --require-all-captures
build-extra-live/th08_replay_check game_data_donottrack/th08.dat \
  reports/native/replays/magic/lunatic/story-native-nmnb-4a-finalb-3890487560-seed40755.rpy \
  reports/local/native-progress-check/story.rows \
  --score-audit reports/local/native-progress-check/story-audit.json \
  --require-all-captures
```

For fresh story tape following, add `--prior-final-a-clear 1` as well as
`--stage 1 --story 1 --difficulty 3 --shot 1 --seed 40755` and the sampled input
profile. Omitting this history follows6A instead of the producer's6B. The first
incorrect verification is retained locally; the corrected configuration agrees.

This Native-first progress passes the normal Release Native/DAT suite56/56
in970.10s, core36/36 in8.37s and focused writer/name/capture checks2/2 in39.85s.

The selected source-survival follow-up is
[Story3,903,603,650](../reports/native/replays/magic/lunatic/story-native-nmnb-4a-finalb-3903603650-seed40755.rpy),
36213 bytes, SHA-256
`1d4d1338a6af34049ee1a9d0ad51d65f9d61d6b672b3f24894d8387574d2186e`.
It retains the same complete story entry, seed40755, prior-Final-A history,
SampledGameplay,500ms/H128/publication64 and collection/human bias5, with the
source survival-form priority fix and explicit `--survival-bias 5`. It completes
NMNB,37/37 in119637 owner updates/515.804s; fresh41-word terminal agreement and
natural replay verification pass. Natural119724 rows SHA-256:
`99aeb5ede5ca99e6301ba992b5bad2b8dfd8820087706953d035d4afa0aff35d`.
Frozen solver SHA-256:
`d6c145a9fe077d7dfe90b88185cc80e37d19a31452bc2af73a56a1982c19265c`;
parent `cb6f47a`, source patch SHA-256:
`be41529b785ef3581182a201db01e736433d3a018637b4024bd548238186968b`.
Fresh follower SHA-256:
`f7e112e93a739fd692044c116faca426cf3600f808703b7c35e3439a77cdd59f`;
checker SHA-256:
`8aa8344addb7e5a542895473a36bf76ddcf81e77c732bce4aa30e30ecd609fc7`.
The original-writer name remains `ysopwnab`. Complete manifests and checks are
under `reports/local/native-score-offline-20261026/story-survival-prior-v2/`.
The earlier selected story replay remains unchanged.

The selected offline continuation is
[Story4,126,102,910](../reports/native/replays/magic/lunatic/story-native-nmnb-4a-finalb-4126102910-seed40755.rpy),
42372 bytes, SHA-256
`00d9207984e33c17d9090ba22e7a9b0dc4872540aaf662e625403ce2c35ef930`.
It is NMNB,37/37, with the same original entry/seed40755/prior-Final-A history
and `ysopwnab`. The full128013-input tape includes61710 literally reconstructed
inputs from the failed4,071,061,590/36-of37 prototype tape; that tape itself
reconstructs59818 inputs from the selected3,903,603,650 run. No world/RNG/item
reset occurs at either boundary. The new tail uses reserve2/480, source ordinary
pacing, survival bias5, H512/publication256, lead1/nominal1/interval128,
30s search/7200s owner, and capture-fire recovery in the frozen generic
future-repair prototype. The complete attempt charges215.523s/305829+16
branch/proposal updates/387 future deaths/0 timeouts/5 repair attempts/1 adoption;
the original failures remain separately charged. These are combined profiles.
Full fresh41-word terminal agreement and natural original-writer replay pass.
Natural128100 rows SHA-256:
`f6dc6766095d643c2ccb4547cd743142ad0d81753d3595ef2bc6abcbc48a2258`.
Frozen producer SHA-256:
`f141348215249d9367f35c3fb03333853326921653e0965bc3b7e8402d430aac`;
private producer source SHA-256:
`55e42bbb3eaaa705c2706261860b01277ee007e5be268a79f0ce5a7646151692`.
Follower SHA-256:
`0343f2edbd2e66a6ae6fde5b591042690712a32f9b06258c9a2516acb1eb64d5`;
checker SHA-256:
`039b3134d2edb0da655d3f6089542d2caff9007830ceb5d7e255bf51802d3195`.
The producer is the frozen local future-repair prototype, not an assertion that
current-main searches regenerate identical actions. Complete identities, prefix
lineage and verification receipts stay under
`reports/local/native-score-offline-20261026/conservative-tail-score-v1/story-conservative-capture-v1/`
and its referenced prototype manifests. Earlier selected replay bytes stay unchanged.

The complete-tape refiner, survival-priority regression and unavailable-slowdown
JSON reporting pass the full Release Native/DAT suite57/57 in1020.13s. The new
refinement CTest covers full-duration capture/score retention, fresh exact
terminal replay, natural `.rpy`, incomplete-incumbent rejection and timeout work
charging. The initial focused pair passes2/2 in48.19s; the final documentation
check passes after the selected story and measured negative trials are recorded.

The selected retail-seeded full-entry story is
[Story4,160,457,200](../reports/native/replays/magic/lunatic/story-native-nmnb-4a-finalb-4160457200-seed1805.rpy),
41117 bytes, SHA-256
`76d8860c334abffac0fdf8eb42b34ee34e86771f837f8383d0fd35e0660b0768`.
It is Magic Team Lunatic4A→FinalB, NMNB,37/37, with `ysopwnab`, raw stage0,
difficulty3, shot1, story=true, prior-Final-A=true and SampledGameplay. Seed1805
is bound to the original Wine acquisition receipt documented above before any
Native candidate. This run starts from original entry with no imported prefix.

The producer is the frozen private future-repair v3, rebuilt against main
`5a5f2af`'s audited Native libraries and corrected owner handoff. It uses
H512/publication256, lead1/nominal1/interval128, policyH24/beam384,30s per
search/7200s owner, source pacing and reserve2/480, priced/upper collection,
collection/human/survival biases5 and capture-fire recovery. Every accepted
proposal still completes its Native horizon or the actual terminal; failed
partial paths are only proposal material. The run charges124549 owner updates,
543.230s,624148+15 branch/proposal updates,790 rejected future deaths,
0 timeouts/overruns and319 repair attempts/4 adoptions. This is a named combined
profile, not a component ablation. The main CLI without that private search
does not promise to regenerate its tape.

Frozen producer/source SHA-256:
`71b1b3e972017e3165ec29ea3ea41521baaea318219432b8a398476ae5821caf` /
`fe4f63d18f860e628baada77e242dd0702b2164bbbca96f8f67028ce25f75d8c`.
Full input-tape SHA-256:
`d055afa43af7511a47cd837f964d6839830c21dfb134baf10652c467c00cb872`.
Fresh follower/checker hashes remain
`0343f2edbd2e66a6ae6fde5b591042690712a32f9b06258c9a2516acb1eb64d5` /
`039b3134d2edb0da655d3f6089542d2caff9007830ceb5d7e255bf51802d3195`.
Fresh41-word terminal agreement passes; natural124636 rows have SHA-256
`7f6a2825695608aeaf4c9e58f39d7d1f562599cef8bbf353eecacb963d6fdd2b`.
Natural score416045720 stored units,37 captures, zero death/bomb rows and the
selected name all agree. Complete manifests, frozen tools/sources, input tape
and verification receipts stay local in
`reports/local/retail-score-offline-20261026/story-retail-long-repair-v3/`.
The earlier selected replay bytes remain unchanged. Retail playback waits for
the owner's human-score target; the seed-acquisition run is separate evidence.

The selected next story continuation is
[Story4,311,260,760](../reports/native/replays/magic/lunatic/story-native-nmnb-4a-finalb-4311260760-seed1805.rpy),
47437 bytes, SHA-256
`a20f0dac5c00ea61d6eee673e554bf5ac5cabda2fe8c5c73d48178e2255d8c04`.
It retains the original retail seed1805, Magic Team Lunatic entry/history,
SampledGameplay and `ysopwnab`, with NMNB and37/37. It reconstructs49753 actual
inputs from the early no-fire graze trial, then uses the conservative v3
H512/publication256/beam384/30s/reserve2/480/capture-fire profile. Those inputs
precede the failed ancestor's4A encounter; the entire new run is executed from
original entry without world/RNG/item resets. The ancestor fails later at71183
and stays charged. Its early-graze policy SHA-256 is
`52f360f88531418f12a3d8228a4898eed652d15d84f240f4173bff6721b52e6f`;
its full failed tape SHA-256 is
`4d8db5ff61f221000fb47889816f2b9781a45978ae8385731e6e50837480fcff`.

The completed continuation charges136061 owner updates/440.974s,
452782+15 branch/proposal updates,654 rejected deaths,0 timeouts/overruns and
169 repairs/1 adoption, in addition to the ancestor's408.359s. Producer/source
and verification-tool hashes are the same v3 identities immediately above.
Full selected-tape SHA-256:
`25d844d137a70be1346015d49620e8a70645090b3642d18aa590b93e5bdc330e`.
Fresh exact41-word terminal agreement passes. Natural136148 rows SHA-256:
`a1e39f043f91f223afcf83997a05eabd45f6356de67386959fc37620b0d1f4e7`.
Score431126076 stored units,37 captures, zero death/bomb rows and the selected
name all agree. Complete local producer/check receipts are in
`reports/local/retail-score-offline-20261026/story-retail-early-prefix-safe-tail-v3/`.

The selected real-seed Extra progress is
[Extra2,504,560,470](../reports/native/replays/magic/extra/extra-native-nmnb-2504560470-seed46997.rpy),
28886 bytes, SHA-256
`e2a98fdde8ae9071ffb282890ddb2bb6d01f858e759cebbaa153fafe4d74ecab`.
It uses the original retail seed46997, raw stage8/difficulty4/shot1/story=true,
prior-Final-A=false, SampledGameplay and `ysopwnab`, with NMNB and14/14. The
first61756 literal inputs come from the verified source suppressed-hit-period2
incumbent, then the off-axis policy regenerates the last HP/survival tail.
Both execute freshly from the same original entry. The current-episode score
crosses2.5B; historical seed10025 still has the higher2,589,417,330 Extra record.

The tail uses H512/publication256, lead1/nominal1/interval128, policyH24/beam384,
30s search/7200s owner, reserve1.5/240, capture-fire recovery and hit-income1.
Its completed attempt charges72637 owner updates/64.1007s including literal
prefix reconstruction,62027+6 branch/proposal updates,129 rejected deaths,
0 timeouts/overruns and0 future repairs. The incumbent's585.409s remains charged.
Producer SHA-256:
`30011df351374b760ee81247e932c5e5aef5264141a5d8f7b99071432676eb4a`;
CLI source is v3 above, paired off-axis policy SHA-256:
`fb5e60414a2e3953fabc1183a589d0c717f81b777040f3a96cf8418715f8f1f4`.
Full tape SHA-256:
`44b98efb7cddd71f0488013ec1bba30cb027245bba5ac968f37d596a20d42523`.
Fresh41-word agreement and natural72633 rows pass; natural row SHA-256:
`0065922919b0af0c70e450bb88faab15aaf03ae4f7d064cb69e6520df61a7e21`.
Score250456047 stored units,14 captures, zero death/bomb rows and `ysopwnab`
agree. Local provenance/checks remain in
`reports/local/retail-score-offline-20261026/extra-retail-last-hp-offaxis-v4/`.
Both newly selected files preserve the original writer's exact metadata bytes.
The frozen public runtime's full Release Native/DAT suite passes59/59 in1078.17s;
the maintained-document check is rerun after these selected results are added.

The earlier selected Extra continuation is
[Extra2,508,068,110](../reports/native/replays/magic/extra/extra-native-nmnb-2508068110-seed46997.rpy),
29318 bytes, SHA-256
`e826b1c49b513937098e768ce7d60d544649de6d0d28cd7c6d62275358c95455`.
It retains the same original retail Extra entry/seed46997 and the verified
61756-input prefix from2,493,554,650. The regenerated suffix uses the stock v3
policy with reserve1.2/120, preserve-fire recovery, hit-income1 and the same
H512/publication256/lead1/nominal1/interval128/beam384/30s schedule. It completes
NMNB,14/14 in73237 updates/57.5388s including prefix reconstruction,
55282+5 branch/proposal updates,131 rejected deaths and0 timeouts/overruns/repairs.
The585.409s ancestor remains charged. Producer/CLI-source hashes match v3 above.
Full tape SHA-256:
`7619204268bdbf80150aa180bdf54e0147ec9233c352c0acbcb36d85853c8844`.
Fresh exact41-word agreement and natural73233 rows pass; natural row SHA-256:
`ed6b58e464a55a519e665245e29db8c5070c516d0a450965a22b7663cf5174a3`.
Score250806811 stored units,14 captures, zero death/bomb rows and `ysopwnab`
agree. Complete local provenance is in
`reports/local/retail-score-offline-20261026/extra-retail-terminal-orb-income-v5/`.

The final read-only item-economy probe uses the current source ItemManager with
request/allocation/pickup logging. It changes no gameplay control flow, input or
RNG. Tasks319/320 preserve exactly the Extra natural rows immediately above and
the Story4,311,260,760 natural rows above; complete logged pickups equal925/1340
source-audit points. Per-event `fflush` is required because `th08_replay_check`
deliberately uses `_exit`. The first buffered attempt's exact gameplay rows pass,
but its truncated CSV is explicitly incomplete and excluded from whole-run
economy conclusions. Corrected identities and checks remain local in
`reports/local/retail-score-offline-20261026/point-income-probe-v7/{extra,story}/`;
[Status](STATUS.md#measured-point-economy-and-next-scoring-work) owns measured
totals, limits and the proposed scoring experiments. New replays/reports/tapes
outside the explicitly selected files remain ignored.

The selected current Extra files are the full-entry
[2,603,278,620 parent](../reports/native/replays/magic/extra/extra-native-nmnb-2603278620-seed46997.rpy)
and its
[2,620,568,090 continuation](../reports/native/replays/magic/extra/extra-native-nmnb-2620568090-seed46997.rpy).
Both retain original retail seed46997, proper Extra8/4/1/story=true/priorFinalA=false
and player `ysopwnab`. Fresh41-word terminal and natural NMNB14/14 agreement pass.

| Identity | Full-entry parent | Cash-comparison continuation |
| --- | --- | --- |
| Task / owner updates / seconds |324 /72133 /620.136 |338 /72557 /114.434 |
| Replay bytes |28613 |28968 |
| Replay SHA-256 |`d6e7b34da1a08ea00889af106dabfb7fb97910cdef54288b3ab29323ad7eeb7c` |`332f5fe46d8f7419bc3e58cf40032d8625060472332f04aa71da637f503d616c` |
| Full input tape SHA-256 |`602b3859599184c4dcc2de31fc863a3b7b4191d663d6abfbdffb26bafd580546` |`47bf615c33c300dd29ce22cd28c4ca04b17662d9add6d4152b4c7d6a90ee84c9` |
| Natural rows / SHA-256 |72129 /`9be971dbfb2cb00ef8d6040e02d632d7ea76a3304bc90df9bb9ecedba9640571` |72553 /`cb4de67f40a68533950af55f5b33502397be8e47d7e3483e5a652d363b91d368` |
| Producer binary SHA-256 |`dbf54e5da1001533f6cdd8eb4029952d5db6834c95294b8c7291587fecd822de` |`0efbbae585285e9b0f2ae3d8a7c35ce006b5c3228b5e33915cf93bb7753746b4` |

The parent uses score-harvest, held/baseline preserve-fire recovery, current
source drop-count/HP/reach target, source hit-window focus-pulse2, hit-income1,
priced/upper collection, human/collection/survival bias5, H256/publication128,
lead8/nominal0/interval8, beam384,12s total search and reserve1.5/240. Its CLI
is the recorded future-repair v3 source, SHA-256
`fe4f63d18f860e628baada77e242dd0702b2164bbbca96f8f67028ce25f75d8c`. Its frozen adapter/kernel/proposal sources are
`a9a505e2267e5c1ac10a96e90a87140e6ae525e94095e42ebf7f0146b5df5000`,
`8203243bbf77019d3bdcebc57262b602d2beea7befb9d52765cf63415e1508f1` and
`53e884e46a339f88687d480e92226d90580f0dc7bda1e83beefed6acaf1b8155`, respectively.
This is a named combined profile, not a component ablation.

The continuation executes60879 literal parent inputs from original entry and
warms its observers; it resets no world, bullets, items, actors or RNG. Then the
stock policy uses H512/publication256/lead1/nominal1/interval128, beam384,30s total
search, reserve1.2/120 and preserve-fire recovery. Up to4 Native32-update fire/focus
comparisons retain stable ties, preserve the fixed lead/movement and require full
horizon certificates. It records271 attempts/34 adoptions,138928+6 branch/proposal
updates,328 rejected future deaths,0 timeouts/overruns and0 future repairs. Its
CLI source SHA-256 is `7acc4e6c9629ca028977508ffdb6e30c13266a12c35adf27eae5d41969364747`. Parent work stays charged.

Parent follower/checker SHA-256:
`0343f2edbd2e66a6ae6fde5b591042690712a32f9b06258c9a2516acb1eb64d5` /
`039b3134d2edb0da655d3f6089542d2caff9007830ceb5d7e255bf51802d3195`.
Continuation follower/checker SHA-256:
`0343f2edbd2e66a6ae6fde5b591042690712a32f9b06258c9a2516acb1eb64d5` /
`039b3134d2edb0da655d3f6089542d2caff9007830ceb5d7e255bf51802d3195`.
The instrumentation over the parent preserves its stock natural rows and logs
1042 requests/1041 spawned/1015 collected large points, with1 X-bound rejection
and0 full-pool rejections. Its exact read-only source/binary/row identities are
local in `extra-drop-target-h256-v1/pool-audit-v9/`.

Complete frozen sources, compile/link commands, seed-bound manifests, literal
tapes, resource records and verification outputs stay in
`reports/local/retail-score-offline-20261026/{extra-drop-target-h256-v1,extra-newbest-cash-compare-v1}/`.
The final public Native producer reproduces the identical72557-input tape and
41-word terminal in131.492s, using `--economic-proposals 4` and the same original
60879-input prefix. Its binary SHA-256 is
`a92af259282880aba49d5a2550da4f0aeaa44c09bb84cd84b55210dd3ac545f4`; CLI source SHA-256 is
`71adf15044d54b143845629c155f1db22087bcb0123a57e2faf83e4cad4fe8ba`. The selected replay retains the first verified
prototype export's exact bytes; public-run exports remain local. The preceding
public build also passes fresh/natural verification with the same inputs.

Public Native now exposes default-off `--drop-target` and
`--economic-proposals`; the selected parent additionally uses the separately
frozen private focus-pulse/repair producer. Neither replay meets the human-score
gate yet, so Wine playback remains pending. The Native slowdown metadata remains
the original unmeasured writer value.

The integrated default-off goals pass the normal Release Native/DAT suite
**59/59 in1191.44s**, plus the maintained-document check. The full suite peaks
at1,002,758,144 summed-RSS bytes; its minimum host MemAvailable is12,034,150,400
bytes, above the8GiB abort threshold. No memory abort occurs. Two earlier
partial suites were deliberately stopped before relinking and remain charged;
neither is reported as a pass. The final public full-entry Extra continuation
reproduces the selected exact input tape and terminal at131.492s.

## Selected Extra score replay

The owner-selected [historical v15 Extra replay](../reports/native/replays/magic/extra/extra-native-nmnb-2564311730-seed19732.rpy)
is an unchanged original-writer export of the frozen v15 Native prototype:

| Field | Value |
| --- | --- |
| Displayed / stored score | 2,564,311,730 /256431173 |
| Outcome | Complete NMNB, all 14 eligible spells captured |
| Entry | Raw stage 8, difficulty 4, shot 1, seed 19732, story=true, priorFinalA=false, SampledGameplay |
| Owner updates | 68429, from complete Extra entry without prefix repair |
| Replay | 26949 bytes; SHA-256 `2bf469a6cbe59732f53b2a7b2ef208d3b450fc272e4c9c172eff244502b0c2d4` |
| Natural playback | 68425 calculation rows, replay input clock 67925, no death/bomb-input rows, matching terminal score and 14/14 capture outcomes |
| Complete 25-field row SHA-256 | `f172b6a60cc440696fd2f4d9015c2ae4a4848b766a4ddb6a8745da0bb47d834a` |
| Recorded full action tape SHA-256 | `c66646b84430d4e3b881bca7a6667772f5b32c9bd3261b5f0b1eebfb2f0bfd53` |
| Frozen solver binary SHA-256 | `740826437ca21e952e6ff29d55496a766ee089721d162570669aa3e89dfa68a6` |
| Frozen/fresh replay checker SHA-256 | `ee9fd522cb1795d7750227ffe5206eb66d691dcafd4ddd060486864d7501aba3` |
| DAT SHA-256 | `9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb` |
| Export compatibility target | Retail 1.00d, size 840704, checksum 2724749753, version 0100d |

The producer manifest records main parent `576c5f1` **plus local prototype edits**;
that parent alone is not the solver identity. The complete local manifest is
`reports/local/score-styles-20261025/extra-flow-v15/manifest.json`, task 207;
prototype source snapshots are in `alpha-v15-src/`. Policy source SHA-256:
`a3b5cf8d300287b8a4762995c12a9a989e7126f51b7a5b21fa50e593ef5ec691`;
Native adapter SHA-256:
`f00d548075dffe1dce60ba95bbdcc8465b71306dcf97a62e525d1e2ef0e1f743`.

The explicit profile is 256-update verification,128 publication,8-update lead,
250 ms total search,60 ms proposal, held recovery and neutral baseline movement
recovery preserving primary fire intent. Collection uses source pickup reach+2,
no-boss upper-stage positioning and 5-point collection/economic human preferences;
active graze goals take priority. Capture reserve is 1.5*observedHP/DPS+240.
Source gauge warmup/income sampling and the corrected 20-pixel ordinary graze
expansion are enabled. The original 408.479 s solve, rejected work, fresh verification
and weaker/failed attempts are charged separately in Status.

The October 26 handoff repeats natural replay, every spell outcome/bonus and fresh
41-word terminal agreement on the exact recorded action tape. Calculation rows,
Session API calls, replay clock and original owner updates are different units;
they are not expected to share one count. Fresh execution/natural replay verify
Native execution and export agreement. This selected run has **no actual online
Wine scoring run or retail playback validation**. It remains a frozen prototype
result; current-main searches can choose a different action-dependent future.

Handoff checks pass Release Native/DAT CTest **56/56 (933.85 s)** and core
**36/36 (8.49 s)**. Fresh tape execution costs29.622s in this verification;
the original38.097s follow remains part of the original experiment's accounting.
Documentation path/heading checks and14 local chapter-anchor checks pass.
The fresh checks and publication receipt stay in
`reports/local/handoff-cleanup-20261026/`; only the selected `.rpy` is newly published.

Reproduce the tracked replay from a Release headless build and owned DAT:

```sh
mkdir -p reports/local/extra-replay-check
build-headless/th08_replay_check game_data_donottrack/th08.dat \
  reports/native/replays/magic/extra/extra-native-nmnb-2564311730-seed19732.rpy \
  reports/local/extra-replay-check/extra.rows \
  --score-audit reports/local/extra-replay-check/audit.json \
  --require-all-captures > reports/local/extra-replay-check/result.json
sha256sum reports/local/extra-replay-check/extra.rows
```

Require exit 0, natural completion, zero death/bomb rows, raw terminal 256431173,
14/14 captures and the exact row hash above. To watch in an owned retail game,
copy the unchanged file into an unused `replay/th8_XX.rpy` slot. Retail viewing is
a separate check, not a property established by the export target header.

## Resume Extra scoring on main

The current [operational contract](STATUS.md#operational-resume-contract) owns
resource limits and the cumulative1024-task window, including the October27
recorded64-task extension before use. The owner has resumed with
No Miss/No Bomb and optional captures. Use the existing retail-seeded episode and
a fresh output directory, rather than copying a historical Native-selected seed
or launching online MPC.

The local guarded launcher at
`reports/local/retail-score-offline-20261026/run-batch.cjs` reads the proper
entry from its hash-bound seed receipt. It freezes binary/source/controls and
charges one task per owner. `memory-guard.cjs` enforces process address space,
summed owned RSS and host-memory admission/abort bounds. Count all other active
owned drivers before launch: the script's one-to-four-case input is not a global
concurrency lock. Only its own process groups may be terminated.

For a later code change, build Release with `--parallel 2`, stop all relinking
before verification, and run the full suite through the local guarded helper
with a fresh report basename:

```sh
setsid --wait node reports/local/retail-score-offline-20261026/run-normal-ctest.cjs \
  normal-release-next-001
```

The parameterized implementation passes60/60, plus final focused reporter checks;
documentation-only cleanup requires the documentation CTest. These helper/receipt/tape paths belong to this working
copy; a clone contains the selected replays and maintained provenance, not the
ignored local producers and action tapes.

The following records the public2.620B suffix profile using its preserved
full input tape. It is a reproduction example, not a reason to rerun a settled
case. Clone the spec to a unique name and record why any new controls/prefix
change when designing a new experiment. These Node scripts are local campaign
orchestration; production solver/build/tests remain C++17/CMake.

```sh
cat > reports/local/retail-score-offline-20261026/next-spec.json <<'JSON'
[
  {
    "name": "extra-next-suffix-001",
    "kind": "extra",
    "binary": "build-extra-live/th08_shadow_style",
    "source_file": "tools/shadow_style.cpp",
    "source_files": ["include/th08/online.hpp", "src/online.cpp", "src/online_native.cpp"],
    "reason": "Reproduce the verified Extra suffix from its own original-entry prefix",
    "controls": {
        "--allow-bombs": 0,
        "--require-all-captures": 0,
        "--verify-captures": 0,
        "--verify-horizon": 512,
      "--publish-horizon": 256,
      "--lead-updates": 1,
      "--nominal-warm-updates": 1,
      "--replan-updates": 128,
      "--search-budget-ms": 30000,
      "--policy-beam": 384,
      "--hit-income": 1,
      "--capture-factor": 1.2,
      "--capture-padding": 120,
      "--recovery-fire": "preserve",
      "--economic-proposals": 4,
      "--prefix-actions": "/home/yann/yann/touhou/th08-solver/reports/local/retail-score-offline-20261026/extra-main-cash-confirm-v2/run/plan.actions",
      "--prefix-updates": 60879
    }
  }
]
JSON
setsid --wait node reports/local/retail-score-offline-20261026/run-batch.cjs \
  reports/local/retail-score-offline-20261026/next-spec.json
```

The driver adds the active NMNB/capture/name/score-target and declared collection/
form defaults,400000 maximum updates and7200s owner wall budget; inspect its
manifest's actual args. The selected full-entry parent additionally uses private
focus-pulse/repair sources. A new public full-entry `--drop-target 1` experiment
is a different profile, not a claim to regenerate that parent. For story use
`kind: "story"` and its own seed1805/prefix; never attach Extra inputs or omit
prior-Final-A history. The last FinalB cash/PIV trials are already negative.

After completion, inspect each `exit.json` (including memory abort reason),
`run/result.json` and capture audit. The launcher may exit0 while a child failed.
A Native exit5 is below-target valid progress only if the complete result meets
its declared bomb/capture constraints. The older guarded checker below targets
strict NMNB/all-capture exports:

```sh
setsid --wait node reports/local/retail-score-offline-20261026/verify-runs.cjs \
  extra-next-suffix-001
```

That strict checker requires fresh verification output and checks original-entry fresh exact
41-word terminal agreement plus natural replay NMNB, score, all captures and the
eight-byte name. Optional-capture outputs instead compare their exact capture
count/outcomes; do not run the strict helper unchanged or interpret a missing
capture as a verifier failure under the relaxed goal. The current local
`reports/local/target-3b5b-20261027/run-guarded.cjs` binds entry receipts,
source/binary/argument identities and memory limits for parameterized trials.
The current source/parent/cash profiles and explicit pending owners are listed
in Status; historical specs keep their original goal identities. Preserve exact
selected original-writer bytes. A lower/partial/
resource-aborted run remains charged and local. Do not edit NaN slowdown metadata
into a fabricated measured percentage. Current frozen success/negative profiles
and prototype/source locations are in [Status](STATUS.md#operational-resume-contract).

### Historical Native and online commands

These commands retain earlier benchmark/recovery profiles. They are not the
active retail-seeded campaign or an instruction to resume Wine solving.

Build with the private DAT configuration described under
[local build](#local-build-and-core-checks) and
[Native reproduction](#native-headless-reproduction-and-acceleration-checks).
The following is the historical integrated seed10025
profile that previously completed at 2,453,582,120, NMNB/14/14:

```sh
build-headless/th08_shadow_style game_data_donottrack/th08.dat \
  reports/local/extra-main-next 8 4 1 10025 score-harvest 100000 1 900 \
  --verify-horizon 256 --publish-horizon 128 --held-recovery 1 \
  --attack-pacing 1 --baseline-recovery 1 --search-budget-ms 250 \
  --priced-collection 1 --upper-stage 1 --collection-bias 5 --human-bias 5 \
  --capture-factor 1.2 --capture-padding 120
```

For the integrated v15-style seed 19732 profile, change the seed and use
`--capture-factor 1.5 --capture-padding 240`. Its last main run **failed 13/14**;
this command does not promise the selected prototype's2,564,311,730. Freeze
source/binary/options and report every attempt. Wall-time budgets, timeouts and
proposal ordering can change the trajectory. Prefer a recorded bounded failure
investigation over repeatedly searching until only a favorable tape is reported.

After a complete solve, verify full inputs and the original-writer replay:

```sh
build-headless/th08_shadow_follow --dat game_data_donottrack/th08.dat \
  --actions reports/local/extra-main-next/plan.actions \
  --output reports/local/extra-main-next/fresh-follow.json \
  --stage extra --difficulty 4 --shot 1 --seed 10025 --story 1 \
  --input-profile sampled-gameplay --frames 100000
build-headless/th08_replay_check game_data_donottrack/th08.dat \
  reports/local/extra-main-next/native.rpy reports/local/extra-main-next/natural.rows \
  --score-audit reports/local/extra-main-next/natural-audit.json \
  --require-all-captures > reports/local/extra-main-next/natural.json
```

Require matching all 41 terminal words, score and every capture outcome/bonus.
A failed or incomplete solve cannot certify a successful full run; its partial
score stays labeled as such. The synchronous Native owner waits during planning,
so its throughput is not actual online FPS. The live `th08_shadow_online` CLI
now exposes pacing, reserve, price/staging/form, baseline recovery and total
search budget. For example, after launching an isolated Wine observer/owner:

```sh
build-issue1/th08_shadow_online game_data_donottrack/th08.dat RUN_DIRECTORY \
  reports/local/extra-live/comparison.json mpc 2400 behavioral h256-held \
  score-harvest --attack-pacing 1 --baseline-recovery 1 --search-budget-ms 250 \
  --lead-updates 24 --publish-horizon 128 --priced-collection 1 --upper-stage 1 \
  --collection-bias 5 --human-bias 5 --capture-factor 1.2 --capture-padding 120
```

Create the output parent first. `RUN_DIRECTORY` must contain the real observer's
versioned start, journal and mapped command file. This is the failed wine-64
profile, not a complete online result. Lead is an immutable1..32-input prefix;
publication must exceed lead and fit both the complete certificate and128-input
wire capacity. Invalid/duplicate controls are rejected before waiting for an
owner. Search telemetry is `comparison.json.searches.tsv`; preserve it with the
report, original RNG start, frozen binary/source/options and actual inputs.
The owner continues while branches run. Completion, NMNB and capture/export
checks are required separately from favorable average FPS or publication slack.
With `--baseline-recovery 1`, `--recovery-style expert|zen` selects the alternate
movement policy, and `--recovery-fire capture` enables new fire in that recovery
instead of preserving the primary pacer's fire intent. Defaults remain
baseline/preserve. These controls keep the original total budget and forced lead;
their extra Native work is charged. Wine-66 is a separately recorded trial of
expert/capture, not an established replacement for the complete online PB.

Those older commands used proposal H24/beam192 and an8-update interval. Current
Native horizon/beam/lead/publication overrides are listed above; do not infer a
fixed current budget from an old command. These Native-selected seeds and live
Wine examples remain historical, with online MPC paused.

## Selected online recordings and replay provenance

These are preserved online/media results with their own profiles. They do not
become the producer of the selected2.564-billion Native replay above.

The owner-requested [recorded Stage 4A replay](../reports/native/replays/online/shadow-recording-v2--stage4a-d3-s1-seed34235.rpy)
corresponds to the actual split-capture Lunatic Marisa/Alice practice, real seed34235.
The original Native replay writer consumes a fresh execution of all 43597 actual
sampled input/control receipts; each receipt agrees with the Wine journal before
export. Natural replay then completes all 43597 calculation rows, zero deaths/bomb
inputs, raw end score27231727 and the pinned 1.00d executable target. Its replay
input clock is 43528, a different unit from calculation receipts. This is an
export of verified actual inputs, not a different searched route; its producer
metadata is Native and its playback clock is not the video's wall clock.

Replay SHA-256: `0e670b832b595bb153429407d17c03d280450dcfe19dbcca002145de2029a92d`.
Original matched video SHA-256: `7f5f4d2938840971b7eeb23c05d56d4ba86c25096ab16a1f95a2e0b9ed4b31a0`.
The original video is 770.566 seconds, H.264 640x480 with actual AAC stereo game
audio; videos remain local. A later compressed delivery retains this case identity.
The compact 640x480 fast-CRF26/AAC128k delivery is 191516144 bytes over 770.584
seconds, with SHA-256 `003be7ce892e8e1f366eb89cc9454d78175198eae585d25b43c0eeeeb4d971c8`.
Its complete decode passes; capture frame timing is retained apart from four
duplicate-timestamp drops required for the encoded delivery.
Copy the replay into an unused `replay/th8_XX.rpy` slot in an owned game copy.
The [standalone Final B replay](../reports/native/replays/online/shadow-recording-v3--finalb-d3-s1-seed29085.rpy)
corresponds to real seed29085, Lunatic shot1, the `fbb7758` client and V2 dialogue
advancement. Fresh following and export agree on all 30333 actual receipts,
zero miss/bomb, zero drift/player error and raw live score93642342. The 8845-byte
replay naturally completes NMNB with the pinned 1.00d target. Natural playback
reports 30330 rows, replay input clock29829 and end score127392342: the original
final-practice/playback clear-bonus offset is 33750000, as documented below.
This is not a row-for-row terminal or score equality claim; the file preserves
the verified actual inputs through the original writer. Actual retail playback
of this selected file remains unchecked.

Final B replay SHA-256: `e565f51c06e210e3242d1e136133184023863ca170b7b83d2810dbad5993011c`.
Matched capture-quality MP4 SHA-256: `9f94a8e74b56836211bab62a3a06d763577e33512a43004557b936b41d71fff8`.
The MP4 is 523.983 seconds/672570952 bytes, with its capture video stream copied
and audio aligned by trimming 10816 samples at 48 kHz. Its complete decode passes.
The 20-second spell158 Kaguya preview starts at movie second198, remains 640x480,
and is 6748573 bytes, SHA-256 `b8eb00563589ff2fceec2d6377e4acaa8b95d14ba94d62bcb5bd90081eacc4ac`.
The [full 4A-to-Final-B replay](../reports/native/replays/online/shadow-recording-v3--story-4a-finalb-d3-s1-seed40755.rpy)
corresponds to the actual complete Lunatic shot1 story, real seed40755, identical
`fbb7758` client, prior-Final-A-clear and dialogue advancement recorded in V2.
Fresh following and export agree on all 121533 actual receipts, 121528 calculations
and five handoffs, zero miss/bomb/player-position error. Numeric drift remains
telemetry. The 34489-byte replay naturally completes NMNB through raw stages
0/1/2/3/5/7, 121654 playback rows and raw end score218996507, matching the live
terminal. Its per-stage input clocks are 14049/15458/14962/24775/21583/29811;
these are separate from receipt/calculation counts and video wall time. This is
the original writer's export of verified actual inputs, without another search.
Selected Stage2 also completes actual normal-mode retail replay timing with
all 15216 callback packets identical across the four renderer configurations and
no death/bomb rows. A held-out normal-mode Final B playback completes with 29501
callbacks, stage-complete set, no death/bomb rows and matching score 218996507.
These are selected-stage playback checks. Full retail playback remains unchecked.

Story replay SHA-256: `202622b4e52e3cedc959f5f41d3e745b8f9d51faad33cf09571834f884758c2a`.
Matched original MP4 SHA-256: `ac7fe1f8abdeb0ecee88d5b7e7b2e26cfa76ae0ee20b56076263dbd9b77fbe03`.
It is 2129.716 seconds/2608210712 bytes, with capture video copied and audio
aligned by trimming 21242 samples at48kHz. Complete media decode passes. The
compact full-story delivery retains 640x480 and capture cadence, fast CRF26 and
AAC128k from aligned original PCM: 2129.717 seconds/683523714 bytes, SHA-256
`615c78f3c0cdfe2c9c7f8536c573355941a9c3e018c9c94f37bac8de4fbfee9c`.
Its full video/audio decode passes, with eight duplicate-timestamp drops and no
interpolation. The stopped first-clear
seed24474 attempt is not published as a completed route.

`th08_shadow_export DAT RECORDING_DIRECTORY OUTPUT.rpy` reads the versioned start
record and requires the owner's Complete marker plus an exact-length journal.
It freshly executes every actual input with SampledGameplay, compares each
receipt under the declared behavioral contract and requires a complete NMNB
terminal (Final B for the second-clear story). Only then does a child invoke the
original Native replay writer. It rejects incomplete/stopped/dead recordings and
existing output paths. Export and fresh natural replay are execution/index checks;
they do not establish independent physics equivalence or actual retail playback.

The owner's un-compacted Extra delivery is the complete actual `wine-35` movie:
`reports/local/shadow-runtime-v1/wine-35/extra-baseline-online.mp4`,836.633 seconds,
1128157155 bytes,640x480 H.264 and48kHz stereo AAC. No second video encode is used.
Its SHA-256 is `eec2c0a6e9ca064a78e7db45f83341e6d050844b5167029e7bc9275460d312c5`.
Both its video and the original Matroska capture have canonical Annex-B stream
SHA-256 `4f7faee6c923b16d974196ce692b13f11479b5d5c3829d4795d620f738d4fd8f`.
Audio alignment trims13995 original48kHz samples. Complete video/audio decode
passes. The matching local `complete-extra.rpy` has SHA-256
`dea524c41bf88b9355ff7d6cd1a978515e1a702ec47ae5d8e47597589104b058`;
the movie and replay remain ignored, with the earlier live/following evidence in Status.

## Source capture audit and Native scoring contracts

The October25 read-only source-lifecycle audit repeats both preserved natural
replays. All121654 story rows and46497 Extra rows remain byte-identical to the
original captures: row SHA-256 respectively
`17815533a8e343a54de382f108e88a89cebf184e434e57b877f0c9fbec33330e` and
`452fc7cc120a6da7c81532eb98f19c47b185bd2fd7e505d82a69016fffd81074`.
Story's missing Last Spells are one-based cards13/32; ordinary cards6/10 end with
timer-transition failure. The other33 captures include Kaguya's timed Last Spells.
Extra captures14/14, including card205. Bonus award values come from `EndSpell`,
not from an OCR estimate or a positive score delta that might include item pickups.
Stored/displayed terminal scores are218996507/2189965070 and116696962/1166969620.
These observations describe the matching Native playback and its published state
projection; replay mode deliberately does not update persistent capture history.

Score auditing preserves the existing replay-check stdout/row interface:

```sh
build-issue1/th08_replay_check game_data_donottrack/th08.dat \
  reports/native/replays/online/shadow-recording-v3--story-4a-finalb-d3-s1-seed40755.rpy \
  reports/local/story.rows --score-audit reports/local/story-score.json \
  --require-all-captures
```

Exit3 means natural playback succeeded but complete capture coverage failed;
exit2 means playback/reporting failed. The report uses source card catalogues for
visited stages, retains skipped Last Spells and reports each lifecycle outcome
and bonus. `observations` counts replay calculation rows in this mode, not batched
Session calls. Start/end timestamps count GameManager calculations. The two
focused real-data regressions cover the old story's negative capture case and a
complete Stage1 scoring4/4 case, fresh execution and natural replay.

The bounded Native comparison tool is:

```sh
build-issue1/th08_shadow_style game_data_donottrack/th08.dat \
  reports/local/score-trial 0 3 1 23035 score 70000 0 180 \
  --collect-items 1 --aim-boss 1
```

Arguments after the output directory are raw stage, difficulty, shot, seed,
proposal preference, maximum owner updates, story flag and wall seconds. The
directory must be fresh. Preferences include baseline/zen/expert/sniper/graze and the score family
(`score`, `score-survival`, `score-harvest`, `score-precollect`, `score-flow`).
The score base enables current-item collection and literal boss aiming.
Optional `--collect-items 0|1` and `--aim-boss 0|1` independently override those
two controls. The result records both resolved booleans and the score-goal
revision, so later ablation runs need no source edits. No new ablation matrix is
required for the current score-improvement campaign.
`score-survival` enables the separate `--survival-focus 0|1` control, recording
form-goal revision2. `--survival-bias VALUE` records the nonnegative prior strength
(default0.01). `score-harvest` adds quiet nonspell boss collection and an existing
edge prior; `score-precollect` additionally prepares current unconditional enemy
drops before firing. `--boss-collect 0|1` and `--precollect 0|1` independently
override these collection controls; both require item collection enabled. Policy
contracts cover closing the boss gate, pre-kill fire release in both decisions
and branch proposals, restoring fire at the collection line, and clearing a
finished collection target. `--verify-horizon 64|128|256`, `--publish-horizon 64|128` and
`--held-recovery 0|1` make the search/publication/recovery choices explicit; all
resolved values, failed branches and retained-window counts are reported. The
read-only score audit counts physical graze awards separately from displayed
graze. Its frozen negative replay regression checks their source bounds and the
unchanged complete25-field row hash.
Per-spell audits now record source-boundary score, collected-orb, point-count and
point-value snapshots, plus physical/ordinary/suppressed graze counts. The start
hook follows preceding-reward settlement; a focused fast-transition regression
guards against charging that bonus to the new spell. Fields remain additive in
audit schema1 and no instrumentation changes gameplay/RNG.
`score-flow`/`--survival-flow 0|1` exposes the separate source-survival stream
proposal, with its resolved strength and flow-goal revision recorded. The policy
contract checks that ordinary attacks keep the harvest result and that an active
survival proposal travels toward an observed ordinary stream with focus.
Mirroring a distant stream must change its proposed endpoint; this regression
fails when the macro omits the source's20-pixel graze expansion. Native safety
verification remains unchanged, and the corrected complete-duration trial's
13/14 failure remains recorded in Status.
Default Native budgets are H64/lead8/110ms/60ms; overrides are explicit.
`--attack-pacing` has a separate contract for excluding source damage reduction
from mature DPS, waiting for extreme-youkai state, profitable/unprofitable orb
samples, bounded unproductive warmup, capture deadlines and fire intent in both
decisions and retained proposals. `--attack-focus-bias`, `--baseline-recovery`
and `--search-budget-ms` are independent reported Native experiment controls.
The complete local prototype and every weaker/failed trial remain in Status;
fresh terminal/replay agreement is not an independent physics oracle.
The price/staging contract checks the source pickup reach and whole proposed
pickup position, collection human preference, empty-field stage positioning,
boss-registration exit, and survival-graze priority over general human preference.
An initial stationary-first-action assertion was rejected: beam proposals can
move before returning to the target. The retained contract checks economic
endpoint/phase behavior without prescribing that incidental first movement.
The optional `--capture-factor`/`--capture-padding`, price height, stage anchor
and two human preferences are independently reported. Complete-duration local
cases retain both higher scores and weaker outcomes; read-only price probes
require byte-identical original25-field replay rows before interpreting values.
The tool records all rejected work,
timeouts, owner/branch Native CPU excluding file I/O, movement/form metrics and
terminal41-word projection. Periodic Z advances dialogue. Story mode declares
prior FinalA history except Extra. Output is a tape, source score audit, complete
result and original-writer replay for a successful terminal. A synchronous owner
pauses during planning; this tool never supplies online FPS evidence.
Fresh `th08_shadow_follow` accepts `--input-profile sampled-gameplay` and
`--prior-final-a-clear 1` for that exact execution contract. The live client also
accepts a final optional proposal-preference argument after its existing horizon
profile and records its name/revision. Baseline remains unchanged by default.

Core CTest cases: 36

Normal core tests require no game assets or reconstruction. Optional source comparisons
add two CTests (38 total). Native builds add process, semantic, hazard-probe and
unsupported-instruction and online-policy contracts; private DAT enables the
complete-scene, online-stage/replay, shadow-follow and planner-prefix CTests (54 total,
including the score-audit and complete style-scene regressions).
An optional `TH08_SHADOW_FAILURE_TRACE` CMake FILEPATH enables one additional
retained-journal regression (55 total) against the private seed42386 Stage4B trace.
`TH08_SHADOW_PULSE_TRACE` adds the private seed56891 move-then-stop regression
(56 total with both traces). It reconstructs all21120 actual receipts, requires
a fatal held direction and a surviving256-update16-step pulse, compares all256
rows with a literal Native tape and checks the actual lead/parent remain unchanged.

The common-start shadow follower compares independently initialized Native
processes under one configuration and the actual input tape. It performs no native
future steps or rewinds, waits for each receipt, and does not enforce a live clock.
The 41-word projection includes current scalar state, RNG/feedback, and hashes of
the declared actor/bullet/laser views; it is not complete world serialization.

```sh
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --difficulty 0 --spell-id 179 --seed 0 --strategy spell-portfolio --frames 2000 --actions reports/local/id179-shadow.actions --output reports/local/id179-shadow-control.json
./build-headless/th08_shadow_follow --dat game_data_donottrack/th08.dat --stage 6b --difficulty 0 --spell 179 --seed 0 --actions reports/local/id179-shadow.actions --output reports/local/id179-shadow-follow.json
ctest --test-dir build-headless -R 'shadow|documentation' --output-on-failure
```

The real-data regression compares all 1292 updates, rejects an explicitly different
follower seed at update zero, and compares 32000 historical story inputs through
the natural first handoff (31999 calculations). The partial story reports `budget`,
not complete. Reports bind DAT/binary/actions hashes. This validates Native process
synchronization only; replayed input is not a new solver or live Wine result.
The same real-data test also verifies the complete true-RNG native branch,
rejects a fatal stationary candidate, checks parent world/RNG/filesystem isolation
and executes every accepted action through the original Session. Held-tail verification is compared with literal Native execution for the same
complete64-input action tape, including a released shoot bit and parent isolation.
The optional retained4B test reconstructs all34256 original receipts: the same
32-input prefix dies at62 with a Policy tail and survives64 when held. It checks
the full prefix and parent state, with its diagnostic timeout recorded separately
from live search budgets. The core wire
contract also checks bounded publication of a longer successful certificate,
including unpublished-tail validation and rejection of a fatal longer future.
It also crosses an earlier publication boundary using the remaining successful
certificate after a fresh extension fails, preserving queued inputs and original
rollout counters. Fatal or malformed retained certificates cannot extend a queue. It
tests late-prefix agreement, stale receipt rejection and preservation of
an existing plan after a failed proposal. Neither self-comparison is an
independent physics oracle. A guided child also executes the complete 1292-update
scene from an eight-input fixed prefix: every remaining input comes from a
branch-owned callback observing the advanced world. Its packets must match the
fixed-action reference, and callback counters/state must remain unchanged in the
parent. This guards callback update order and fork isolation, not Policy quality.
The core scheduling regression retains the already-verified queue while the
owner advances after an unsuccessful nominal extension. That same plan stays
ineligible for reuse after death or timeout; a new certificate origin enables
reuse again. Advancing the current receipt alone must not restore eligibility.

The optional local Wine bridge requires a 32-bit MinGW compiler and the pinned
retail 1.00d EXE (`330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`,
840704 bytes). Keep a private copy of the game/config/score and a dedicated Wine
prefix. It attaches only to the new process created by its launcher. Its three
in-memory hooks are declared; no gameplay RNG value is written. Core/native builds
do not require Wine or MinGW unless the observer option is enabled.

```sh
cmake -S . -B build-headless -DTH08_HEADLESS=ON -DTH08_WINE_SHADOW=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-headless --parallel 2
export WINEPREFIX="$PWD/game_exe/owned-shadow-prefix" WINEARCH=win32 WINEDEBUG=-all
export TH08_SHADOW_CHECKPOINT='5,3,0,107,0' TH08_SHADOW_UPDATES=6000
run="$PWD/reports/local/wine-shadow-example"
game="$PWD/game_exe/owned-shadow-game"
mkdir "$run"
launcher=$(winepath -w "$PWD/build-headless/wine-shadow/launch.exe")
observer=$(winepath -w "$PWD/build-headless/wine-shadow/observer.dll")
executable=$(winepath -w "$game/th08.exe")
destination=$(winepath -w "$run")
(cd "$game" && wine "$launcher" "$executable" "$observer" "$destination") &
./build-headless/th08_shadow_online game_data_donottrack/th08.dat "$run" "$run/result.json" mpc 300
```

Start a normal game in the owned window. The optional checkpoint is applied only
before original gameplay registration: zero-based stage, difficulty, shot, spell
or -1, story 0/1. Unset it to preserve the actual menu-selected configuration.
The observer records the real seed before registration, then publishes only
actual gameplay/control updates. Use a new output directory each time; existing
journals/command files are deliberately rejected. The mapped command file requires
local coherent x86 Linux/Wine memory; remote SSH transport is not implemented.

The client accepts mode `follow` for receipt-only Native comparison, including an
already finished observation journal; mode `mpc` adds branch-owned futures.
The wall-seconds argument caps total command wall seconds, including menu/start
waiting. Exit 0 is the actual case terminal; 3 is projection mismatch, 4 is death,
5 is a budget/stop boundary and 1 is runtime/transport failure. Actual sampled
inputs export beside the output as `.actions`. Reports bind start/trace/actions,
DAT and Native binary hashes, configuration, budgets, proposal/native attempts,
charged interrupted work, stale results, input fallbacks and actual owner clocks.
The trace's calculation time brackets the original hooked calculation function,
including input-hook calls within it. Observer time covers projection/cache
preparation after that call, stopping before trace encoding, `WriteFile`, atomic
publication and outcome handling. Inter-receipt periods include work between those
boundaries, such as rendering, limiting, scheduling and journal writes; they do
not attribute a slow frame to one component. Total elapsed time starts at the
first hooked calculation's entry and ends at the last projection timestamp.
`TH08_SHADOW_RENDER_TIMING=1` adds an independent `render-times.bin` journal
without changing the existing228-byte receipt format. Its16-byte little-endian
header contains uint32 magic0x31544d52, version1, sample size64 and reserved0.
Each sample has eight uint64 words: receipt sequence, raw stage, elapsed ns,
Present-end period ns, BeginScene-to-EndScene ns, original Present-call ns, scene
count and flags (1 no draw,2 failed Present,4 repeated receipt). The first period
is unavailable. Draw includes submission/synchronization; Present includes backend
waits. Remaining period time includes original pacing, other callbacks, scheduling
and preceding profiling writes. These are measured boundaries, not GPU-only costs.
The core wire contract also checks the disabled legacy refresh setting and the
exact128-receipt refresh boundary, including spacing after a failed attempt.

`TH08_SHADOW_OBSERVE_REPLAY=1` requires render timing and rejects history/dialogue
flags and non-replay gameplay. It forwards original input calls and lets the
original replay loader restore recorded RNG/inputs. Its `replay-timing.txt` marker
and absence of `start.txt` distinguish it from a common-start live run. Normal
retail replay can restart the calculation chain during dialogue; its sequence
counts observed callbacks, not certified live calculations. Discontinuities are
retained in `replay-clock-events.tsv`. Ordinary live cursor checks remain strict.
A selected-stage replay timing run has no Native planner or capture process; record
its playback mode, software-renderer threads, environment and terminal separately.
The selected Stage2 comparison uses Wine8, llvmpipe with two renderer threads,
private game audio, no planner/capture, and the same story replay hash above.
Only the per-process `vblank_mode` setting changes from unset to0. Mean
presentations/s improves56.167→59.343; mean Present time7.900→2.342ms.
All15216 packets agree exactly, including input and all41 fields. The
[Mesa option documentation](https://dri.freedesktop.org/wiki/ConfigurationOptions/)
defines0 as disabling swap-interval synchronization. This remains an opt-in
measured environment choice; local slow windows and scheduling tails remain.
A further matched control changes only renderer threads: LP1 reaches41.045/s,
whereas LP4 reaches59.997/s with mean Present0.043ms. Both match all15216 LP2
packets exactly. LP4 has a59.811/s slowest complete5-second window and18.729ms
period p99. It uses additional rendering workers while solver concurrency stays
fixed. [Mesa documents LP_NUM_THREADS as the renderer thread count](https://docs.mesa3d.org/envvars.html#lp-num-threads).
A held-out normal-mode Final B check uses the unchanged LP4/vblank0 profile,
original pacing and 640x480, private game audio and no planner/capture. Its 29500
Present samples average 59.95552/s; the slowest complete 5-second window is 58.00598/s
and period p99 is 19.81930ms. There are no repeated receipts, absent draws or failed
Present calls. The 29501 callback packets end at stage-complete, score 218996507 and
zero death/bomb violations. This is held-out rendering and replay evidence, rather
than an additional solver clear or every-frame60Hz guarantee.
The buffered solver profile separately declares512 verification,128 publication,
lead32,480ms total and128-receipt fresh-planning intervals. Keep this latency and
compute budget distinct from the original110ms/lead8 controls.

Optional trailing arguments `[behavioral|exact] [h32|h64|h64-held|h128-held|h256-held|h256-pulse|h512-buffered|h128-recorded|h64-recorded]` select comparison and
planning profiles (defaults `behavioral h32`).
The recorded mode uses128 verification/publication, lead8 and110ms total, with
Native conditional hazard recording and independent full-path verification.
`recorded_*` fields separate geometry/search work and timeouts; its recording
updates are also charged in `proposal_*` totals. The real-data contract checks
literal replay, immutable lead, parent RNG/filesystem isolation and the retained
4B failure; the ordinary current-memory Policy remains unchanged.
The `h64-recorded` portfolio preserves H64 nominal guidance, then verifies128
updates for a recorded repair and preserves its longer literal continuation.
Publication remains64. JSON separates base/maximum verification, publication,
recording horizon and profile revision. The API also supports independently
verified shorter windows; the regression rejects overstating their length.
Live comparison records numeric bullet/laser digest drift, player differences up
to 1/64 pixel and inactive spell/route metadata; meaningful gameplay fields remain
checked. It does not modify either simulation or collision handling. Exact mode
retains bitwise projection diagnosis. Keep launcher/DLL/config identities with the
local run receipt as well. New logs,
tapes and diagnoses stay under ignored `reports/local/`.
The opt-in `h64` bundle, named held-recovery variants and nominal-prefix extension are specified in
[Planner](PLANNER.md#true-rng-shadow-runtime). Do not combine its results with
default-profile runs or attribute differences solely to horizon. Its reports
separate branch simulation, guidance, proposal work, conservative timeout charges
and actual unqueued-update streaks; these are costs, not a 60 FPS guarantee.

Core checks reject late generations, torn publications, altered immutable leads,
unsupported bomb bits and edits to the owner's private accepted copy. The complete
Native scene also compares future-lead proposal inputs with the same Policy only
after actual execution reaches that lead, guarding history, latch and private tie
RNG isolation. Rendered Wine spell completions and retained story mismatches are
separate manual real-data evidence in [Status](STATUS.md). Same-update stopped
diagnostics use raw float bits: the client writes `.bullets.tsv` beside its report;
the observer retains `retail-bullets.tsv` from the post-calc phase, before drawing.
Neither a partial story nor a complete spell establishes a continuous 60 FPS run.
The same real-data regression executes a complete first story stage in both
Controller and SampledGameplay profiles, checks the control-only handoff, then
requires the new stage's first sample/latch to be respectively zero or the
explicit supplied input. A core cursor regression covers the intervening retail
loading callbacks so that the control receipt cannot disappear again.
Existing public CI runs sanitizer OFF/ON without DAT/source;
it does not establish real-DAT scenario success. Keep new verification proportional to
the changed behavior; no elaborate sanitizer infrastructure is required.

`lzss_decode` compares the headless field reader against an independent one-bit
scalar oracle. It checks persistent nonzero dictionaries, generated streams and
byte truncations, sequential overlapping/ring-wrap copies, end markers and exact
output guards. The output-capacity obligation and zero-padded EOF remain the
native caller contract. A separate real-DAT audit checks the actual exported
Encode/Decode entry points against unchanged renamed legacy code: all 317 entries
and the file table have identical bytes, declared lengths and complete final
dictionaries after each decode. This is codec equivalence, not a gameplay oracle.

`headless_surface_copy` calls the production D3DX entry point through owned CPU
surfaces with padded pitches and change-notification counters. It checks every
16-bit pixel of the two eligible formats against direct identity and the forced
original conversion path, plus 32-bit alpha, untouched padding/source, self-copy
overlap, rectangles/clamping, color keys, scaling, X formats, invalid filters and
zero dimensions. The fast path is headless-only and requires separate owned
surfaces, equal positive full dimensions, no rectangles/key and exact filter 3;
all other cases keep their original behavior.

`model_artifacts` checks the complete saved-table file set, SHA-256, byte lengths
and data-row counts against [the model manifest](../models/artifacts.tsv).
The native online-policy contract additionally loads all three historical packs
through the production parser. Artifact identity and successful parsing do not
establish a disjoint evaluation split or learned-policy quality; [model provenance](../models/README.md)
records the inherited unknowns.

The branch-value core contract checks normalized logistic probabilities, extreme
logits, exact model round trips, immutable output snapshots and strict rejection
of truncated models, invalid scales/features, duplicate labels and failed or
incomplete native windows. The independent separable CLI fixture must fit and
reproduce byte-identical weights; overlapping train/tune seeds and native faults
must fail before creating a model. These synthetic checks validate the trainer
and format, not gameplay quality.
The online-policy contract verifies that candidate export, a constant model and
zero model weight preserve ordinary actions/depths and proposal counts. Complete
H24 samples own 23 newly submitted inputs (the old latch owns movement one), keep
shooting and contain only finite features. Partial/failed searches export none.
`headless_value_collection_real_data` compares exact nominal tapes, observation/
actor projections, terminal, score, RNG and calculation counts with/without
independent Native branch sampling on both 107 and Extra 202. An initial false
startup terminal is guarded by the required 640 nominal updates and positive
branch count. Teacher futures stay outside policy features.
The maintained collector writes `branch-inputs.tsv` with each exact submitted
sequence and prefix hash; its real-DAT contract also requires one sidecar row per
branch. The first frozen collection predates that sidecar and retains its exact
producer binary/source and parent tapes instead; the model card preserves this
provenance distinction.

The online-policy lifetime regression preserves source dispatch order: an active
gate precedes masked-record skipping, WAIT clears after program dispatch, and a
DESPAWN still collides in its activation callback. An independently stepped
three-position corridor checks the inclusive cutoff, longest safe partial depth,
unavoidable latched first hit, unknown-clock fallback and disabled metadata/tie
transparency. Native known-seed pilots and fresh action execution are separate
checks of the enabled proposal profile; neither this narrow program hypothesis
nor agreement between two Native executions is an independent physics oracle.

The complete online-stage CLI regression also executes a generated idle prefix
without Policy decisions. Native `player_state==2` is an immediate NMNB failure
during the deathbomb window, before the `deaths` counter increments. Exit 4 and
`outcome:death` therefore remain valid with `deaths:0`; the additive
`final_player_state` field makes that boundary explicit. Consumers must retain
this failure rather than require a positive life/death counter.

The shared spatial selector's `plan_contract` regression compares partial rank
selection against the frozen former full-sort algorithm in 420 deterministic
cases. It checks every retained float field, action, violation count, parent,
output position and bin statistic across ties, signed zero, infinite scores,
negative coordinates, focus bins and different widths/cell sizes. Both fresh and
reused flat-table workspaces must match, including growth, shrinking and empty
input between searches. Hash collisions still require full bin equality. Invalid
cells and non-finite coordinates still fail. Performance checks additionally require
identical full action bytes and behavioral reports before comparing timings.
The spawn-grid contract checks clipped edges, row separation, signed zero and a
large/small-term sum whose rounding changes under reassociation. A second contract
warms search storage, takes an immediate latched-collision return, then compares
the next full decision with an observer-only control, including selection counts
and exact action/depth. These guard per-decision grid and scratch-storage reuse.
The movement decomposition contract compares the frozen original move with
prepared displacement plus position addition/clamping in 204800 cases: all 256
low input-bit combinations (with unrelated high bits set), twenty speed/multiplier
profiles, ten positions and four rounding modes. It requires identical position
float bits and floating exception flags, covering signed zero, subnormals,
negative multipliers and bounds. Complete action/behavior comparisons additionally
guard integration of per-search displacement reuse into candidate enumeration.
The online AABB predicate is compared with the frozen former short-circuit
expression in 403116 finite-geometry cases across four rounding modes. Fixtures
cover all sixteen rejection combinations, signed-zero permutations, subnormals,
touching/degenerate bounds and deterministic finite bit samples. Boolean results,
standard floating exception flags, preexisting flags, errno and rounding mode must
agree. This checks the proposal predicate; native collision physics stays separate.
The half-pixel key contract compares the optimized path with the original
`lround`/mask expression at every fast-range rounding boundary and its adjacent
floats, plus one million finite bit samples (26165824 comparisons). Four rounding
modes additionally check exact values, exception flags, errno and unchanged mode,
including signed zero, subnormals and large/special-value library fallback.
Optional component clocks are checked against an unprofiled policy through
consecutive observations, a lethal root, recovery and duplicate-decision rejection.
Actions, depth, proposal counts and fallback work must agree; rejected duplicate
calls clear the caller-owned timing sample. The real-data contract also compares
profiled/unprofiled budget tapes and checks a complete prefix with zero decisions
and zero live-native time. `--profile-components 1` emits separate, explicitly
instrumented scopes as described in [Planner](PLANNER.md).

## Current-memory CPU controller

Build `th08_online` with the native Release configuration described in the README.
The owner permits current process memory, including current transform and laser
lifetime state. This baseline is `native-memory-kinematic-v1`, not an image-only
agent. It makes no future Session updates and does not rewind the live game.

```sh
./build-headless/th08_online --dat game_data_donottrack/th08.dat --out reports/local/online-stage2 --stage 2 --difficulty 3 --seed 0 --shot 0 --horizon 32 --beam 192 --reach-filter 1 --frames 60000 --seconds 900 --log-every 1
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/online-stage2-verify --stage 2 --difficulty 3 --seed 0 --shot 0 --verify reports/local/online-stage2/online.actions
./build-headless/th08_replay_check game_data_donottrack/th08.dat reports/local/online-stage2-verify/th8_verify.rpy reports/local/online-stage2.rows
ctest --test-dir build-headless -R headless_online --output-on-failure
```

For the opt-in physical clock/worker harness, a short run explicitly remains a
budget outcome. It takes approximately six seconds for 360 API ticks:

```sh
./build-headless/th08_online_live --dat game_data_donottrack/th08.dat --out reports/local/live-smoke --stage 1 --story 1 --final-route a --difficulty 3 --seed 0 --shot 1 --horizon 24 --beam 192 --diversity-cell 16 --spawn-guard 16 --reach-filter 1 --hard-phantoms 1 --motion-envelope 1 --value-model models/branch-value-v1/weights.tsv --value-weight 0.25 --history-queue 4 --scheduler-tolerance-us 0 --frames 360 --seconds 10
ctest --test-dir build-headless -R 'online_live|headless_value_collection' --output-on-failure
```

Live core checks cover absolute tick arithmetic, cutoff/origin/phase rejection,
bounded consecutive observations, worker faults, discarded-work accounting and
expired-history agreement with a serial Policy. The one-frame Native CLI check
uses a declared generous owner tolerance solely to verify serialization/input
accounting, not performance. It requires JSON-escaped literal TSV model metadata
and rejects zero queue capacity before Native initialization. Physical timing and
complete routes require separately frozen experiments with their actual tolerance.

Natural replay `rows` counts recorded gameplay calculations; `stages[].frames`
is the last observed `ReplayManager::frameCounter`, a replay input clock. They
are different units. `ReplayTraceRecord` runs before input-enabled/clear gates
that can prevent clock advancement. The complete Stage 2 regression, for example,
has 32993 rows and last input-clock value 32924. Validate total row coverage against
the tape/calculation count, plus natural terminal, score, ordered stage path and
zero death/bomb rows. Do not require the last input-clock value to equal total rows
or patch it with a guessed constant offset. Fresh action agreement, natural replay
and independent retail comparison remain separate evidence levels.

The optional branch-value experiment uses separate offline collection and C++
training tools. Use new output paths; neither collector episodes nor saved model
snapshots are overwritten. The collector's `--stage` is the native index 0..8
(1/2/3/4A/4B/5/6A/6B/Extra), unlike the online CLI's named stage convention:

```sh
./build-headless/th08_value_collect --dat game_data_donottrack/th08.dat --out reports/local/branch-labels --stage 5 --difficulty 3 --spell 107 --seed 42 --shot 0 --frames 20000 --stride 128 --checkpoints 32 --seconds 1800
./build-headless/th08_value_train reports/local/train.tsv reports/local/tune.tsv reports/local/new-value-weights.tsv > reports/local/value-training.json
./build-headless/th08_online --dat game_data_donottrack/th08.dat --out reports/local/value-stage5 --stage 5 --difficulty 3 --seed 42 --shot 0 --horizon 24 --beam 192 --reach-filter 1 --hard-phantoms 1 --motion-envelope 1 --value-model models/branch-value-v1/weights.tsv --value-weight 0.25 --frames 100000 --seconds 1800
```

The first command supplies one parent episode, not disjoint train/tune datasets.
Construct whole-parent/seed splits before collection, preserve both header lines
when merging TSV rows, and freeze model/weight before independent evaluation.
Every sampled prefix adds separately reported native updates and wall cost.
The saved first model reproduces byte-for-byte when the maintained trainer fits
the audited original train/tune files locally. This checks reproduction, not
generalization. Its [model card](../models/branch-value-v1/README.md) owns label,
split and input-profile assumptions.

Stage names use the repository convention `1/2/3/4a/4b/5/6a/6b/extra`, rather than
the preparation prototype's zero-based numeric CLI. `--story 1 --final-route a|b`
uses the existing native dialogue/unlock-history checkpoint contract. For a
continuation, pass `--prefix` with its original checkpoint; frame budgets include
that prefix, and observer history is warmed without replaying past search ties.
Only one-update commits and the kinematic forecast are supported. Outcomes have
distinct exits: complete 0, setup/output error 2, budget 3, death 4, runtime error 5.
Output directories must be empty; failed tapes remain evidence, not successes.

Optional `--planner-lag 1|2` uses the deterministic delay model described in
[Planner](PLANNER.md). `--executor-check`, `--emergency-horizon` and
`--emergency-beam` declare its check/rescue budgets; warm/held/tail combinations
are rejected. Lag zero remains the default. Policy regressions cover the one-input
latch at adoption, early/punctual adoption, newly visible hazards, pending-result
cancellation, actual input/position/movement/phase changes, skipped decisions,
an unavoidable first update and both shorter/longer rescue depths. Full Stage 2
keeps `c4acd20a…`; complete lag-zero 6B tapes also retain their parent identities.
Lag-one/two window repetitions must match actions and deterministic event counts,
while excluding diagnostic times/overruns from equality. New complete runs still
require the same fresh action and natural replay gates. Separate executor/planner
statistics in a synchronous driver do not certify unpaused deadlines.

The optional lazy-index regression compares eager and delayed construction through
normal reuse, a new incoming hazard and a phase invalidation. Actions, depths,
selection/search counts and executor events must agree while at least one update
builds fewer indices. The hazard must also force a previously omitted suffix to
be constructed before replacement search. Component clocks must remain disjoint.
Three alternating spell-166 pairs per model and two complete 6B candidates retain
the eager lag-two tapes and all event counts; the complete tapes pass fresh action
execution and natural replay. This changes index scheduling, not the forecast or
the native collision predicate.

The opt-in hard-phantom contract compares a disabled fired bullet with the same
ordinary real bullet, with and without reach pruning. The no-model default must
still match an absent ghost. The prior must preserve ordinary search behavior,
leave disabled spawning bullets unchanged, retain conservative root failure and
respect a ghost-to-real velocity discontinuity. A model-loaded control checks
that hard promotion replaces the phantom-only soft obstacle. Native paired-bank
outcomes, default-mode action agreement and fresh replay checks are separate gates;
these synthetic comparisons alone establish neither survival nor model quality.
The pilot's prespecified practice tapes pass fresh native action execution, but
the current verifier exports a replay only when `spell < 0`. Practice receipts
leave `rpy` empty. The attempted natural replay check is retained as a failed
harness assumption, so this pilot has native-proxy evidence without a natural or
retail replay claim. Such tapes require an appropriate export/validation path
before promotion to certified expert routes.

The optional current-state motion envelope is checked against all modeled switch
times with repeated float additions in all four rounding modes. It keeps the
ordinary forecast when no alternative is supplied, rejects non-finite inputs and
requires explicit hard-phantom mode. A focused policy fixture exposes a restored
flight direction with reach-filter agreement. `headless_online_motion_real_data`
executes the complete 107 checkpoint that previously died after that mode change,
then freshly verifies its complete action tape, terminal, score, RNG and update
counts. This is a native-practice regression, without a natural/retail claim or
proof that the conditional motion hypothesis covers every native transform.

Optional `--risk-dir` requires all four files: `risk_table_merged.tsv`,
`risk_backoff_seg.tsv`, `risk_backoff_global.tsv`, `nowarn_merged.tsv`.
`--risk-phantoms 1` additionally uses collision-disabled bullets as soft risk
unless the fired-bullet hard prior replaces that path.
The maintained line parser fixes the preparation's cross-row backoff parsing, so
those results need new reproduction rather than assigning old outcome/timing
claims to the corrected model. New model hashes and training lineage belong in
the local experiment protocol. No learned model is required by the complete-stage
CTest. Its unit companion also rejects truncated/duplicate risk rows, checks
latched movement and prefix velocity history, and compares pruned/unpruned search
with interleaved independent policy instances.
The transform-history regression observes a stopped relative turn followed by its
final constant-velocity flight. Estimating continuing acceleration across that
mask change incorrectly declares the first latched update lethal; resetting the
rate history retains a complete escape, with reach-filter agreement. An unchanged
active vector transform still accelerates from rest and keeps its genuinely
unavoidable latched hit. No zero-speed or seed-specific strategy is substituted.

The October 23 exact-kernel gate compares 200000 prepared laser rotations against
the frozen original formula, including signed zeros, subnormals, large/axis angles
and all four rounding modes, with batch exception/errno agreement. Bounded spatial
selection adds 2100 dense/fallback comparisons to the frozen full-sort oracle,
preserving all retained float bits, source ties, parent/action indices and counts.
The complete Stage 2 CTest retains action SHA `c4acd20a…`; normal 40-test native/DAT
coverage passes. Optional `TH08_ONLINE_ARCH=native` and `x86-64-v3` also pass local
policy contracts; this is numerical evidence for those builds, not a flag guarantee.
The proposal-set regression separately compares first-key ownership with the
frozen original `lround`/mask formula in all four rounding modes. It covers cyclic
boundaries, signed zero, subnormals, focus separation, undersized/invalid hints,
migration of populated bins, independent instances and repeated layer resets.
Native complete-stage and prefix pairs also require zero unexpected domain
expansions; their original action, selection-count and terminal gates stay active.

`th08_spell_bank TRIALS SEEDS CASES PROTOCOL BASELINE CANDIDATE` evaluates a paired
spell-practice bank against declared case/seed files, not only available outputs.
SEEDS contains one native seed per line. CASES has the TSV header
`stage difficulty shot spell` (tab-separated; stage uses the report's zero-based
entry index). TRIALS has `profile stage difficulty shot spell seed outcome frames
actions_sha256`, also tab-separated. Every declared pair must exist exactly once;
unexpected cases/seeds/profiles, duplicate or truncated rows and runtime/setup
outcomes reject the summary. Budget outcomes remain non-completions and are counted
separately. The report binds all four input files by SHA, gives Wilson 95% intervals
and the exact two-sided McNemar discordance test, and makes no equivalence claim.
The standalone contract includes a completely missing case, missing pair, duplicates,
invalid identities/outcomes and known interval/binomial-tail values. Spell practice
remains a proxy; it does not establish complete-story probability.

The independent Release campaign freezes shot 0, Lunatic, seed 0, non-practice
6B story entry, H64/beam192/cell16/guard16, reach filter and soft phantom option.
It tests both no-model and the unchanged four-file risk model. Actual spell-166
prefixes have 42681/45032 updates; each measured continuation has 2400 decisions.
Warming does not replay tie RNG, so compare each continuation with its own frozen
control rather than demanding the original full run's suffix. Three alternating
orders separate laser caching, bounded selection and each CPU target; component
profiling is separate. Portable and native complete runs compare all actions and
non-timing behavioral fields against full controls. Six fresh action executions
and natural replays agree on score/RNG/calculations and 380292 rows, with zero
death/bomb rows. Raw rows are removed only after retaining hashes/byte counts.
This is execution agreement, not independent physics, retail or unpaused timing
validation. [Status](STATUS.md) owns measured results and remaining work.

Optional `--spawn-guard 16` adds actor-origin soft risk, `--diversity-cell 16`
retains spatial/focus beam alternatives, and `--queued-shots 1` uses positive
bounded shot prefixes from current main program cursors. All default to zero/off;
record each setting and compare complete outcomes rather than combining the best
seed from different profiles. Queued coverage is partial and counts unsupported
preview records; zero records is not an empty-future certificate. The policy
contract also checks early avoidance of a queued birth, the unavoidable latched
update, soft actor risk without a learned table and deterministic spatial retention.
`--queued-lasers 1` independently enables the existing timed direct-ECL warning
decoder; reports count warnings and unsupported dynamic/variable geometry. Its
regression checks escape before a rotated beam appears, the already-latched first
update and rejection of a reversed lethal interval. These forecasts are partial;
actual native execution, fresh actions and natural replay remain the outcome gates.
`--held-fallback 1` adds eighteen checked constant-input proposals after an
incomplete beam. Its synthetic two-beam regression retains a complete rightward
escape that the default beam prunes at depth eight, checks reach-filter agreement
and counts the extra forecast work. A lethal first latched update still returns
depth zero. Complete-world outcomes are evaluated separately from this proposal
contract.

The optional terminal-tail regression keeps H8 branching/depth while previewing
a birth at update nine through eight extra held-input checks. It verifies early
escape, reach-filter agreement, stopping at a complete first suffix, separate
query counts, a lethal latched update and rejection of H+T above 200. The narrow
laser-arc regression retains a clear first update that the previous cosine/dot
tolerance rejects, with 10001 independent static source-predicate samples across
the angular interval. Companion checks cover wide arcs, full-circle reach and
the opposite ray at near-zero reach. These geometry contracts are separate from
full-world outcomes.

The optional `--spawn-clock 1` regression requires an escape before a supported
animation completes, checks reach pruning and rejects a purported spawn clock on
a fired bullet. A late first snapshot with one animation call left must account
for both fractional and full displacement, including an unavoidable latched hit.
Unsupported timing retains the named legacy hypothesis. The pure ANM companion
checks all three terminal opcodes, next-call/null-cursor order and refusals for
unknown, masked, malformed, blocked, negative and fractional clocks. These timing
contracts do not certify velocity changes at activation or later native state.

The optional `--warm-fallback 1` regression uses two crossing linear bullets and
a one-node beam. Rechecking the previous plan suffix restores a full 24-update
proposal after ordinary replanning discards it at depth 13. The same check covers
reach pruning, skipped decisions, changed latched focus, a new forecast collision,
depth ties, an already-latched death and the exact additional-query counts. These
are synthetic proposal contracts; complete native outcomes remain separate gates.

`--direct-turn-envelope 1` tests a current-memory angular uncertainty model.
Its policy contract includes opposite-turn escapes from signed extrapolation,
sixteen orientations around the full circle, reachable-envelope agreement and
pre-birth cue timing through the input latch. Raw-batch tests cover difficulty,
both forward branch paths, truncation, variable writes and backward-loop rejection.
Reports add `turn_hint_records`, `unsupported_turn_hints` and
`turn_envelope_frames`; these count a partial heuristic, not certified future
geometry. Observe both complete outcomes and controller latency.

The queued pooled-laser regression starts from a harmless birth update and
requires movement before the first long active hitbox. A one-update-late snapshot
cannot recover that escape through the latched input. It also checks reach-filter
agreement and rejects a purported spawn whose timer has already advanced. Source
preview contracts remain independently exercised by the native planner tests.

The relative-turn policy regression warms a stationary velocity history, then
installs an owned source turn. It requires an early escape, counts only the
bounded kernel updates, checks reach-filter agreement and the first input latch,
and rejects aimed/inactive state masquerading as an independent relative turn.
The existing bullet-motion source differential remains a separate kernel check.

For an always-shoot student's offline teacher queries, add `--shoot 1
--quiet-retries 0` to `th08_plan` with the original checkpoint, immutable
`--prefix` and explicit total frame/time budgets. Inspect every generated suffix
input's shoot bit, the byte-identical prefix, `options.quiet_retries:0` and
`quiet_windows:0`; a zero quiet-window counter alone does not cover checkpoint
or short retry delays. The prefix CTest completes a real spell from a supplied
quiet input, retains that input exactly, checks every continuation input and
freshly verifies the result. The default planner retains adaptive quiet retries.
Matching output actions does not remove the offline teacher's privileged future
rollouts or establish online completion.

`online.json` binds executable, DAT, exact action/prefix bytes and each risk file;
the local protocol additionally binds the source archive and environment. Decision
timing includes observation copying, policy and input decoration, excluding native
execution and file serialization. Prefix reconstruction contributes to native and
whole-loop costs; setup is separate. Buffered `decisions.tsv` records predecision
state and the submitted input when requested. Reports retain mean, p99, p99.9,
maximum and misses against the configured deadline. `forward-measured` waits for
late decisions and explicitly reports `deadline_enforced:false`. A passing stage
or a low mean is not proof of a real 60 FPS controller.

Fresh verification must match exact terminal, score, RNG, action hash and
calculation/control events. Natural replay must finish with the same score and
observed calculation rows, zero death rows and zero bomb-input rows. These checks
establish native execution agreement, not an independent physics/retail oracle.
Source, model, process receipts, timings and all failures normally stay local;
publish sanitized conclusions and maintained code/prose. The owner's explicit
request for the two successful complete 4A story captures is a narrow exception:
their [publication audit](../reports/native/archive/online_story_4a_validation.json) retains
sanitized provenance, timings and native verification receipts. It does not add
retail playback or enforced-deadline evidence at its publication date. The later
manual retail audit in [Status](STATUS.md) covers these exact two original files;
the frozen publication JSON is not refreshed with new experiment artifacts.

Final-stage practice recording and natural playback currently differ at the
clear-bonus branch. Native practice sets `isPracticeMode`; playback clears it and
uses a separate replay-practice predicate. Static inspection of the unchanged
retail 1.00d executable (SHA256
`330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`)
agrees with the native GUI branch: final-stage non-practice clear adds
`2500000*lives + 500000*bombs` before difficulty scaling. A separately checked
supplied Stage 6A tape predicts and reproduces the same 33750000-point Lunatic
offset seen in the Stage 6B control; it starts at the clear-bonus update. Position
also diverges later in the clear tail, where playback freezes input while live
control continues. This explains the observed mode difference without changing
the score gate, and does not certify a complete retail replay or all 25 fields.
Keep the failed equality results. Any non-practice final-stage checkpoint must be
named and evaluated separately rather than relabelling a failed practice result.

## Native fork planner and replay gate

This section reproduces the preserved earlier offline portfolio and its report
producers. Its named profiles, build identities and timing results are historical;
the current Extra scoring entry points are at the top of this page.

The C++ replay suite requires the accepted corpus, manifest and schema from upstream
`02ff72f29e7b4928f21c5a2fb0f57ffa0cec5415`. Pass their locations explicitly; raw replay
captures and game data remain local. The default full gate pins the 108-entry manifest
and supported v4 schema, verifies replay hashes, creates a new output directory, and
requires natural successful captures with every expected row/fingerprint.

```sh
./build-headless/th08_replay_suite --checker build-headless/th08_replay_check --dat game_data_donottrack/th08.dat --manifest reference/config/replay-fixtures.json --schema reference/scripts/replay-external-schema.json --corpus reference/tests/fixtures/replays/th08-1.00d/accepted --output reports/local/fidelity --jobs 8
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/spell92 --stage 4b --spell 92 --difficulty 3 --seed 0 --rollout 64 --diversity-cell 16 --shoot 1 --checkpoint 60 --keep 12 --frames 10000 --seconds 180
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/spell92-verify --stage 4b --spell 92 --difficulty 3 --seed 0 --verify reports/local/spell92/plan.actions
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/story --stage 1 --story 1 --difficulty 3 --seed 0 --rollout 64 --beam 384 --diversity-cell 16 --checkpoint 120 --keep 64 --shoot 1 --frames 250000 --seconds 2400
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/story-resumed --stage 1 --story 1 --difficulty 3 --seed 0 --prefix reports/local/story/plan.actions --rollout 64 --diversity-cell 16 --checkpoint 120 --keep 64 --frames 350000 --seconds 1800
./build-headless/th08_plan_matrix --seed 0 --output reports/local/matrix.json
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/final-b --stage 1 --story 1 --difficulty 1 --shot 1 --seed 0 --final-route b --rollout 64 --beam 384 --diversity-cell 16 --checkpoint 120 --keep 64 --frames 350000 --seconds 3000
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/extra --stage extra --story 1 --difficulty 4 --shot 11 --seed 0 --frames 350000 --seconds 3000 --diversity-cell 16
./build-headless/th08_hazard_probe --dat game_data_donottrack/th08.dat --stage 4b --spell 92 --difficulty 3 --seed 0 --event-update 393 --leads 393,360,320,280,240,200,160 --output reports/local/spell92-probe.json
./build-headless/th08_planner_report --root reports/local/campaign --fidelity reports/local/fidelity/summary.json --known-binary-sha256 01cdab0637be646f45d3e0db12734837a979aa5a8e218fc8dabc7e427f0171e8 --replay-dir reports/native/replays --output reports/native/archive/planner_current_summary.json
```

`reference/` above is a placeholder for supplied pinned corpus files, not a runtime
dependency. `--case ID` selects an explicitly labeled subset but still validates the
complete corpus. Explicit non-108 fixtures exist for synthetic protocol tests and do
not count as a full gate. `--rows FILE --schema FILE` independently fingerprints an
existing capture. One Session step can perform multiple replay calculations; capture
rows and API call counts are checked separately.

The current measured corpus result is **106/108**, not the handoff's recorded 108/108.
`lunatic-1-b` ends early at 36200 rows; `lunatic-11-a` has all 116281 rows but a different
fingerprint. Both reproduce with O0/O3 and a fresh pinned full native Debug build;
restoring the draw chain does not fix them. Their retail-divergent row and cause remain
unknown. Do not relax the gate or rewrite reference fingerprints. Matching projection
fields do not establish every internal state or every input sequence.

Planner reports include native attempts/completed updates, failed rollouts, checkpoint
retry costs, options and capacity. New reports hash DAT and executable once before
forking, outside native timing. The first recorded comparison campaigns predate this
metadata addition; the retained spatial-planner binary is identified separately in
their evidence. That explicit composer hash is specific to those archived campaigns;
it must not be assigned to other builds. New campaigns use producer-recorded hashes.
Concurrent campaign wall times are not isolated single-core benchmarks. The current
remote campaigns use Linux x86_64, AMD EPYC 9654 and GCC 13.3 Release, with contraction
disabled. Local diagnostic probes use AMD EPYC 7B12/GCC 12.2 Release. Independent jobs
have task-specific scratch/output directories, low CPU/I/O weights, bounded memory
and no additional swap; the shared host's other workloads are not modified.

The ledger requires a finished process, matching fresh terminal/RNG/checkpoint and
zero deaths before counting a completion. Practice/story cases additionally require
a natural `.rpy`, matching calculation-row count and final score, and explicit zero
`death_rows`/`bomb_input_rows`. These counters count observed rows, not death/bomb
events. Refreshing them on the four practice tapes preserved every original row byte.
Replay calculation rows and Session API calls are separate cost totals.

For continuous story, five native stage-control calls can complete without a
gameplay calculation. The observer is beside `gameplayFrameCounter++`, after the
setup early-return gates; the earlier trace/RNG hook stays at its original boundary.
Reports bind `frames = calculation_updates + control_updates` to explicit ordered
stage-transition events, and natural replay rows must equal the observed calculation
count. The measured 232408-action route has exactly 232403 calculations, with control
calls at 24158, 56541, 95948, 138845 and 178264. This is measured source behavior,
not permission to subtract the number of stages from an arbitrary replay.

For older completed campaigns, retain the original plan and same-binary verification.
A separate `<case>-accounting/verify.json` supplies missing observations from a new
fresh run, bound to exact raw action SHA, source/DAT/configuration, terminal, score
and RNG. Its executable and costs remain separate. New plan/verification pairs must
agree directly on counts and events. `th08_replay_check` also hashes its actual replay,
DAT and executable before capture. `--replay-dir` archives only verified whole-scene
originals and full action tapes, checks the replay hash, and emits both artifact hashes
in the ledger. Previously published originals remain under
[native replays](../reports/native/replays/). The owner additionally requested the
two complete online 4A story replay/action pairs and their publication audit.
All other new replays, action tapes and generated JSON stay under ignored
`reports/local/`, together with fresh-verification copies
and retained raw rows. Code and maintained prose continue through progress commits.
Git newline conversion is disabled for `.actions` and `.rpy` so
platform checkout settings preserve the exact bytes bound by these hashes.

Completed remote captures may be retained as `.rows.gz`. The storage audit checks
every restored raw SHA before removing its uncompressed copy; use
`gzip -dc capture.rows.gz > capture.rows` before a raw-row comparison. Later
successful captures retain raw SHA/size and native replay receipts, while their
reconstructible raw rows are removed. Replay/action files and failed diagnostic
captures remain available. The [route environment audit](../reports/native/archive/planner_route_environment.json)
records retained evidence, cleanup receipts and bounded shared-host resources.

For new batches, pass repeatable `--group NAME` arguments; each named directory must
exist. Explicit groups require `<case>-verify.exit` and, for whole scenes,
`<case>-rpy.exit` to be zero in addition to their successful JSON reports. Legacy
default groups remain reproducible without these previously unrecorded sidecars.
The suffixes `-verify`, `-rpy` and `-accounting` are reserved for validation sidecars.
Use a local `--failed-actions-dir`, as in the commands below, to retain each failed
attempt's final emitted tape with a hash, parsed NMNB input/count checks and the
role `failed_search_prefix`. These tapes are execution-unverified prefixes, not
successful demonstrations or all rejected internal rollouts. Existing artifact
names are idempotent only for identical bytes; conflicts fail without overwriting.

For full-run baseline batches, add `--matrix reports/native/archive/planner_route_matrix.json`.
The report checks the complete generated catalog, each attempt's checkpoint and
every successful native/replay stage path before archiving, and requires validation
process receipts in this mode. Its `coverage` object
binds the manifest SHA and counts distinct route IDs across all selected groups;
retries retain their separate attempt costs. `expected_combinations` includes the
whole manifest, while `observed_combinations` includes only IDs present in the
input groups. A zero attempt-level pending count does not imply full coverage.
Omitting `--matrix` preserves the historical report format.

For sparse random evaluations, pass `--sample FILE` instead of `--matrix`. A sample
manifest has `schema_version: 1`, `sampling_unit: "seed-case-pair"` and an explicit
`cases` array using the catalog descriptors, with each case's own seed. It may mix
seeds and omit unselected combinations. Optional `sample_phase` is `diagnosis` or
`holdout`; when present, every case must carry the same phase. The report checks
the canonical case IDs, initialization profiles and full stage paths, binds the
sample SHA, and reports pair coverage separately from distinct-seed coverage.
`verified_unique_seeds` counts seeds with at least one verified selected pair;
use pair coverage to assess completion of every planned run for a repeated seed.
The two manifest options are mutually exclusive. A zero pending count does not
mean that every planned sample pair has been observed.

```sh
./build-headless/th08_planner_report --root reports/local/seed-campaign --fidelity reports/local/seed-campaign/fidelity-v1/summary.json --group training-c16 --group training-c32 --group holdout-c16 --group holdout-c32 --group training-resume-c16 --group training-resume-c32 --group holdout-resume-c16 --group holdout-resume-c32 --replay-dir reports/local/seed-campaign/archive/replays --failed-actions-dir reports/local/seed-campaign/archive/failed-actions --output reports/local/seed-campaign/planner_seed_summary.json
```

The current full-run ledger uses all four portfolio groups:

```sh
./build-headless/th08_planner_report \
  --root reports/local/route-matrix-v2/results \
  --fidelity reports/local/route-matrix-v1/results/fidelity-v1/summary.json \
  --matrix reports/native/archive/planner_route_matrix.json \
  --group matrix0-v2-c16 --group matrix0-v2-resume-c16 \
  --group matrix0-v2-c32 --group matrix0-v2-resume-c32 \
  --replay-dir reports/local/route-matrix-v2/archive/replays \
  --failed-actions-dir reports/local/route-matrix-v2/archive/failed-actions \
  --output reports/local/route-matrix-v2/planner_route_summary.json
```

The first random diagnosis ledger uses its frozen 12-pair sample:

```sh
./build-headless/th08_planner_report \
  --root reports/local/seed-generalization-v2/results \
  --fidelity reports/local/route-matrix-v1/results/fidelity-v1/summary.json \
  --sample reports/local/seed-generalization-v2/diagnosis.json \
  --group random1-diagnosis-c16 --group random1-diagnosis-resume-c16 \
  --group random1-diagnosis-c32 --group random1-diagnosis-resume-c32 \
  --replay-dir reports/local/seed-generalization-v2/archive/replays \
  --failed-actions-dir reports/local/seed-generalization-v2/archive/failed-actions \
  --output reports/local/seed-generalization-v2/diagnosis-summary.json
```

The [execution protocol](../reports/native/archive/runtime_perf_protocol.json) fixes three
complete archived tapes, an incomplete prefix and three alternating paired repeats.
The [audit](../reports/native/archive/runtime_perf_summary.json) records every timing sample,
raw receipt hash and normalized execution contract. Its baseline is native source
`0215b7f`; the default cache candidate is `2b30ad1`, both GCC 13.3 Release with the
same floating-point flags. An independent IPO build uses the baseline source and
`-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`. IPO is not enabled by default or combined
with the measured cache gain.

Build each source snapshot into its own measurement directory. For example, the
seed-4455 full-tape workload on the current build is:

```sh
./build-headless/th08_plan --dat game_data_donottrack/th08.dat --out reports/local/perf-seed4455 --stage 1 --story 1 --difficulty 3 --seed 4455 --verify reports/native/archive/replays/training-resume-c16--s4455.actions
```

Use fresh output directories for each repeat and check process exits as well as the
JSON. Verification consumes the entire supplied tape; search-only option defaults
in its receipt do not cap that tape. Compare every execution field after excluding
only native timing and binary identity. Require identical frames, terminal, source,
DAT/action hashes, score, RNG and ordered calculation/control events. The incomplete
prefix must retain exit 2, `verified:false` and no complete terminal.

The cache build and the separate IPO build reproduce all 108 baseline case tuples
without changing the two reference failures. Additional raw captures of both failing
fixtures match byte for byte, as do all 3706547 calculation rows from the 16 new
seed routes. The additional training-only planning pair uses the original cell16
seed-4455 prefix and unchanged 900-second continuation profile; its full actions and
all deterministic plan fields match after excluding timing/producer identity.
Native timing excludes file I/O; shared-host scope CPU includes compilation and
validation, and recorded capture/API counts are separate units. These are execution
agreement checks, not independent physics or retail proofs.

One full source/adapter differential additionally plays the archived
`story-prefix-v1--p132200-r64-b384-c16.rpy` through the pinned full native Debug/O0
build, retaining production draw callbacks with private Xvfb, dummy audio and
llvmpipe rasterization disabled. All 232403 calculation rows (23240300 bytes)
match the headless capture byte for byte: SHA256
`c560ef53295b9a62e949e8fd79fd5c2c6b576d137e3398b108caa4252bb913ea`.
Both captures end naturally with score 169789855, zero death/bomb-input rows and
the same six-stage route. The full-native executable SHA256 is
`9aae4ce60d1a5e8426b1675b7fe7504e0a74f7ed70610ada5c4d692e5a456b00`.
The additional validation costs 232403 calculations and 356.01 seconds including
debugger, display setup and trace I/O, outside the solver campaign totals. This
checks that adapter/profile for one tape; the two paths share source and numerical
behavior, so it is not an independent retail physics oracle.

The export fix's full Release native suite passes 36/36 CTests in 950.99 seconds,
including complete real-data scenes. Earlier receipt hardening passed 35/35 in
751.96 seconds before the export regression was added. Concurrent host workloads
make these suite wall times unsuitable as isolated solver benchmarks. The current
full 108-fixture run retains exactly the prior row counts, fingerprints and outcomes:
106 pass and the same two failures remain. No expected fingerprint is changed.
After the repair utility and its additional contracts, all 35 tests excluding the
unchanged long scene pass again in 21.67 seconds; the core suite passes 29/29.

Prefix continuation is a new planning episode over a natively reconstructed world,
not a stage reset. Its raw input hash and reconstruction costs remain in the ledger;
the final full tape must replay from its original start. Contract checks cover
chunked/malformed/bomb inputs, explicit budgets and terminal boundaries. Native
checks cover successful continuation, exact-terminal completion with no rollouts,
trailing-action rejection, a death at update 382 and budget exhaustion before update 1.
`--verify` executes the entire supplied tape and records its actual native cost;
search frame/time limits apply to planning episodes. Bound verification separately
with a process timeout when running a campaign.

The updated upstream polar math also changes four Stage6b collision values. The named
`stage6b-02ff72f-semantic-v1` contract keeps the old `stage6b-semantic-v1` fixture intact,
pins the new source/geometry, and retains strict fresh replay and all 18 latch probes.
Neither contract asserts universal cross-host float identity.

### Retail replay export identity

The first Wine audit found a real export defect: archived native replays declared
`0100d` but serialized executable size/checksum as `0/0`. The native loader's
`TH08_MODERN_PORT` version bypass accepted them. Retail 1.00d instead failed its
`th08_0100d.ver` whitelist and crashed while listing the replay; its unterminated
last-line traversal reached address 1 in `strncmp`. A known accepted retail replay
loaded and played under the same Wine configuration. Native round trips alone had
therefore missed an incompatible file header.

Recording declares the pinned retail target explicitly: size `840704`, checksum
`2724749753`, version `0100d`. Reports keep that declaration separate from the native
producer's SHA256. It neither changes gameplay/RNG nor certifies retail execution.
`headless_replay_export_contract` first failed with serialized size zero, then passed
after the fix. It checks a complete fresh practice export, exact tape/replay outcome,
legacy repair row identity, idempotence, malformed input and refused output overwrite.

Preserve archived originals. To produce a separately named copy:

```sh
./build-headless/th08_replay_target reports/native/archive/replays/story-v2--r64-b384-c16.rpy reports/local/story-retail-1.00d.rpy
```

This C++ tool accepts only a validated zero-identity or already-targeted 1.00d stream.
It changes the two decoded identity fields at `[0x124,0x12c)`, rebuilds compression
and the enclosing checksum, and refuses an existing output. All other decoded bytes,
stage offsets and trailing user data persist; already-targeted files copy byte for
byte. It reports hashes and never claims gameplay validation. The
[repair receipt](../reports/native/archive/planner_story_retail_target.json) binds the original
to the separately saved
[retail-targeted copy](../reports/native/archive/replays/story-v2--r64-b384-c16-retail-1.00d.rpy).
Both produce the same 232069 native calculation rows, SHA256
`ad7d90ea5d36904d75498c621f18d7be88ca4f42554eb9100ddd306a3eb1c3b5`.
Original archived replay hashes and the campaign ledger remain unchanged.

The separately recorded [retail audit](../reports/native/archive/retail_story_validation.json)
plays this exact repaired file from Stage 1 through Final A in retail 1.00d under
Debian Wine 8.0. It observes the final stage-clear sequence, cumulative deaths/bombs
both zero, internal score `167865093` (display `1678650930`), and natural return to
the replay menu. The retail EXE SHA256 is
`330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`;
all 728064 loaded executable-code bytes match the original before and after playback.
No executable, gameplay memory or RNG patch is used.

This is a manual interoperability audit with read-only process sampling at a nominal
20 ms interval. The continuous sampler attached in Stage 2 at gameplay frame 14651;
the genuine Stage-1/frame-0 menu-start receipt is separate. Its original all-stage
coverage gate therefore reports `terminal_mismatch` (mask 118, missing Stage 1),
retained in the audit. Cumulative death/bomb counters persist through stage handoffs.
The separate terminal receipt observes the actual final clear and menu return;
replay's terminal branch does not set the normal game's `gameCleared` bit. It uses
`GUI_MSG_SHOW_STAGE_RESULTS` / `stageClearSequenceActive` instead. Retail also
continues fade calculations after the recorded input ends; these are not equated
with native capture rows. Recorded score and selected state restore at replay stage
handoffs, so matching terminal scores are not a complete physics differential.

To repeat the manual test, place an owned retail copy under ignored
`game_exe/retail-audit/game/`, put the repaired file in its `replay/` directory as
`th8_01.rpy`, and use a private 32-bit Wine prefix and display. Start windowed with
audio off, effects maximum, frameskip zero and slow mode off. Select Replay, the
first stage and the built-in Boss playback mode (2), which fast-forwards nonboss
sections without enlarging gameplay steps. The audit bounds its process group to
2 then 4 CPU cores, low CPU/I/O weight 10, nice 10, memory high/max 3/4 GiB and no
additional swap. Peak Wine-group memory is 429535232 bytes (about 410 MiB), with
zero memory-limit/OOM events. A prior black fullscreen startup was stopped before
the memory cap; that setup failure and the original header crash remain recorded.
The owned Wine/display processes are stopped after collecting terminal evidence.

Read-only probes are specific to the hashed x86 EXE: flags `0x0164d0b4`, current
stage `0x0164d2cc`, globals pointer `0x0160f510`, cumulative float deaths/bombs-used
at globals offsets `0x64/0x84`, and replay/title pointers `0x018b8a28/0x018bde08`.
No full per-frame retail projection or all-eleven-file retail validation is claimed.
Private raw game data, screenshots and diagnostic logs remain ignored.

The later online-story audit repeats the isolated retail procedure for the two
tracked shot-1 natural-4A replays. Start the read-only observer before confirming
Stage 1 so its first active sample covers frame 0. Derive the expected stage mask
from the frozen manifest: `[0,1,2,3,5,6]` yields `0x6f`; the earlier shot-0 4B route's
`0x77` is a different path. Preserve a failed observer report instead of rewriting
it. Adjudication must independently check the unchanged replay hash, manifest path,
logged cumulative death/bomb counters and reported sampled maxima, actual final clear sequence, expected
internal score, natural menu return and executable-code identity. Keep sampled
coverage distinct from a complete retail differential. A successful tape playback
does not establish an unpaused controller, deadline compliance or all-seed success.

## Local build and core checks

Requires CMake 3.16+, C++17 and OpenSSL development libraries. Linux x86_64 is tested.
Modern float32 with contraction disabled is the numerical profile; no fast-math.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
mkdir -p reports/local
```

The scenario CTest covers complete duration, seeded repeat/replay, phase/RNG continuity,
malformed checkpoints and the bounded-search counterexample/recovery. Planner tests
compare statuses, exact action/position bits, tie-breaking and attempted budgets with
the retained reference. Other tests guard their named component contracts.

## Reproduce the legacy subset evidence

Run serially for useful Release timings. Serialization occurs after timed execution.
Exit 0 means the report/check ran; inspect each outcome, since collision/search failure
is valid benchmark evidence and not a solved scene.

```sh
./build/th08_scenario_cases --scenario all --strategy all --seed 1 --duration 7200 --output reports/local/scenarios.json
./build/th08_scenario_cases --scenario all --strategy rolling-beam --seed 1 --duration 7200 --recover-goal 1 --output reports/local/recovery.json
./build/th08_scenario_cases --scenario relay --strategy rolling-beam --duration 72000 --recover-goal 1 --format tsv --output reports/local/long-relay.tsv
./build/th08_scenario_cases --scenario relay --strategy rolling-beam --visual-draws 0 --output reports/local/no-visual.json
./build/th08_spell_cases game_data_donottrack/th08.dat reports/local/id179 --verify-only
./build/th08_spell_cases game_data_donottrack/th08.dat reports/local/id179
./build/planner_bench > reports/local/planner_benchmark.json
```

ID179 verification checks exact events/terminal occupancy, RNG isolation, checkpoint
copy/resume and unknown-opcode rejection. The normal tool compares four strategies for
seeds 0, 1, 65535, storing twelve seed-qualified routes and a summary. It also checks illegal
and incomplete replay tapes. Raw assets are not extracted or committed.

The planner benchmark reference is the original sorting planner; its timing ratio
includes both heap selection and later early dedup improvements, not only the last
change. Report outcome/counters as well as time. Checked samples used Intel Xeon
Platinum 8573C, GCC 14.2, Release; timing is not portable or a guarantee.

## Exact input provenance

- DAT: 46,838,025 bytes, SHA256 `9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb`
- Component-oracle source: [N0zoM1z0/th08](https://github.com/N0zoM1z0/th08), pinned commit
  `a45e99fb1942714e6edded20847e32a654d56f97`
- Tracked native runtime: same upstream, branch `port/portable-64bit`, commit
  `02ff72f29e7b4928f21c5a2fb0f57ffa0cec5415`, adapted under `third_party/th08`.
  Earlier native reports identify the historical `861bec9` import and retain that profile
- Original preparation archive `TH08_AllCase_20260911.zip`: SHA256
  `8913fffc96824c27b681ff1b1133a4385f7c7ca3e8e377fede17e552e535b199`
- Older preparation reports mentioning a dirty worktree near `af72ca9` are historical
  evidence, not the pinned source or current runtime. `preparations/` remains
  unchanged locally and is excluded from published Git history

Legacy DAT analysis tools check the expected input hash. Native experiment tools
record the supplied DAT hash; compare evidence only for the stated input/profile.
Scenario reports retain relevant member/ANM/SHT hashes and instruction identities.
Source probe generators enforce per-file hashes before
extracting reviewed unchanged bodies; those maintained constants are the authoritative
file-hash list. Do not bypass a mismatch or use aggregate pass rates to hide it.

## Native headless reproduction and acceleration checks

The opt-in target requires SDL2, SDL2_image, SDL2_ttf and Fontconfig development
libraries (Debian/Ubuntu packages `libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
libfontconfig1-dev`). The CPU compatibility layer decodes resources; no video/audio
device or OpenGL renderer is initialized. No Python, Windows or Wine is used.

```sh
cmake -S . -B build-headless -DCMAKE_BUILD_TYPE=Release -DTH08_HEADLESS=ON -DTH08_HEADLESS_DAT="$PWD/game_data_donottrack/th08.dat"
cmake --build build-headless --parallel 2
ctest --test-dir build-headless --output-on-failure
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --spell-id 179 --difficulty 0 --seed 0 --strategy reactive --frames 2000 --actions reports/local/id179.actions --output reports/local/id179-native.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --spell-id 179 --difficulty 0 --seed 0 --frames 2000 --replay reports/local/id179.actions --output reports/local/id179-native-replay.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 195 --difficulty 4 --seed 0 --strategy spell-portfolio --frames 6000 --actions reports/local/id195.actions --output reports/local/id195-native.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 199 --difficulty 4 --seed 0 --strategy spell-portfolio --frames 5000 --actions reports/local/id199.actions --output reports/local/id199-native.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 201 --difficulty 4 --seed 0 --strategy spell-portfolio --frames 6000 --actions reports/local/id201.actions --output reports/local/id201-native.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 202 --difficulty 4 --seed 0 --strategy spell-portfolio --frames 6000 --actions reports/local/id202.actions --output reports/local/id202-native.json
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 204 --difficulty 4 --seed 0 --strategy spell-portfolio --frames 6000 --actions reports/local/id204.actions --output reports/local/id204-native.json
```

The real-data CTest runs Stage 1 to clear (22176 updates), ID179 through its original
wrapper/end (1292 updates, activation at 92) for seeds 0/1/65535, stationary collision
at 382, and retained/adapted boundaries for IDs 85, 89, 93, 198 and 201. IDs 85, 89,
93 and 198 complete for seeds 0/1/65535; ID201 preserves its generic WAIT collision at
362 and requires the seed-0 portfolio completion at 4292. Portfolio IDs 193/195/199
also run through their complete wrappers. ID202 retains its generic WAIT collision at
3265 and its WAIT-only portfolio collisions at 4204/4216/3613 for seeds 0/1/65535;
its original 12-update horizon and disabled future-ECL adapter are checked explicitly.
ID204 retains its generic relative-direction collision at 1205, requires complete
4832-update wrappers for seeds 0/65535, and preserves seed 1's collision at 3008.
ID32 preserves its generic 594 collision, checks the explicit 13-update/one-update-leg
profile and 192132 candidate evaluations for each complete 2372-update seed wrapper,
and runs Stage 3 through both its 7443 baseline collision and 39767 clear.
ID167 retains its generic 932 collision, requires all three 5492-update wrappers
with H12/four-update first leg and 444852 candidates. ID183 retains its generic 1041
collision and requires three complete 1292-update wrappers with H32/four-update leg
and 104652 candidates. It owns the single current continuous Stage6b gate: clear58853.
ID139 preserves its generic 5326 collision, requires all three 7292-update seed
wrappers, and runs Stage 6a through both its 48302 baseline collision and 61041 clear.
The native laser-item executable also checks 18 cancellation geometry cases, including
both source cancellation paths, angle quadrants, 32-unit spacing, exclusive end bounds,
item suppression and off-playfield X rejection.
A 10-update budget failure and the Stage 6b
reactive collision at update 854 remain covered.
Every execution tape replays in another process;
tests also reject unsupported input, excess tape and wrong-ID wrapper selection.
Generated tapes and `summary.json` are under `build-headless/headless-regression/`.
Without `TH08_HEADLESS_DAT`, these private-data tests are not registered.

The focused ID202 group can also run through the same generator, preserving its
baseline and all three failed seed wrappers rather than declaring the spell solved:

```sh
cmake -DEXECUTABLE="$PWD/build-headless/th08_headless" -DCOMPARE_EXECUTABLE="$PWD/build-headless-o0/th08_headless" -DDAT="$PWD/game_data_donottrack/th08.dat" -DWORK="$PWD/reports/local/id202-check" -DCASE_GROUP=id202 -P tests/headless_real_data.cmake
```

The 2026-10-02 Intel Xeon Platinum 8573C/GCC 14.2 run passed all 26 core tests and
the four focused ID202 scenes with fresh replay and native O0/O3 agreement. The normal
aggregate originally failed the unconditional historical Stage6b digest assertion:
this host produces 6279671846274327225, versus historical 4060221407534777929. Clean-main
and O0/O3 replay reproduce the current digest, while the 854/slot664 boundary, complete
collision object, RNG and feedback agree. The older profile did not identify solver
build/libm/CPU sufficiently to claim cross-environment float-bit equality. The precise
intermediate difference is not diagnosed.

The stage6b-semantic-v1 contract now guards DAT/profile/scene, terminal, RNG, feedback
and every collision field instead. It retains the old digest/report and emits
historical_digest_match plus numerical_profile_match=unverified; there is no compiler
whitelist or replacement digest. Strict current-run replay/O0/O3/diagnostic equality and
all 18 input-latch probes are unchanged. Negative DAT, RNG (including malformed fractional
integers), collision and replay mutations fail. This loses the unsupported historical
intermediate-trajectory oracle, so cross-host float drift outside the named boundary
is not proven absent. The historical aggregate remains an immutable provenance record.
`CASE_GROUP=id204` similarly runs the four ID204 fixtures and emits `id204-summary.json`.
Its baseline and seed-1 failure, as well as both complete seed wrappers, agree in fresh
processes and at native O0/O3 on this host. Both group selectors use the same scene/replay
checks as the full aggregate, whose historical digest remains provenance, with current-run equality still strict.

`CASE_GROUP=id32` runs the six ID32 wrapper/Stage3 cases and emits `id32-summary.json`.
All three seed wrappers and both baseline/complete stage tapes agree at O0/O3.
`CASE_GROUP=id167` now runs four wrapper fixtures and emits `id167-summary.json`.
The tracked five-case ID167 report is retained as preceding-checkpoint evidence,
including the then-genuine Stage6b ID183 collision. `CASE_GROUP=id183` runs five
current fixtures and emits `id183-summary.json`, including the new Stage6b clear.
All checked O3/O0 replays match; the full stage runs once in the aggregate.
For ID183 seed 0 on this GCC14 host, the complete 1292-update H32/81-path wrapper
uses 8894.1 ms of decision time; the failing 1041-update baseline uses 613.6 ms. These
different durations are not an equal-work performance comparison. Both exclude file
I/O and make the additional planning cost explicit.
`CASE_GROUP=id139` runs six baseline/complete scenes and emits `id139-summary.json`.
The checked report includes fresh O3 replay and separate O0 replay for every tape.
Pass `-DLASER_ITEMS_EXECUTABLE="$PWD/build-headless/th08_headless_laser_items"` to
include the focused native cancellation regression (the normal CTest already does).
An empty native `fsincos` stub previously caused undefined item geometry and an O0/O3
score difference at Stage 6a update 32432. Assigning both established sin/cos outputs
makes the long baseline and complete-stage tapes agree again: corrected digests are
`3641853114352264503` and `6502928606160698451`. The 18 geometry cases fail with both
pre-fix O0/O3 archives and pass after repair. This does not resolve or replace the
separate retained Stage 6b cross-host golden.

Reproduce the Stage 6b failure and compare the nine input directions locally:

```sh
mkdir -p reports/local/stage6b
./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --difficulty 0 --seed 0 --strategy reactive --frames 50000 --actions reports/local/stage6b/baseline.actions --trace reports/local/stage6b/baseline.tsv --output reports/local/stage6b/baseline.json
./build-headless/th08_headless_probe --executable build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --difficulty 0 --seed 0 --actions reports/local/stage6b/baseline.actions --frame 854 --output-dir reports/local/stage6b/late
./build-headless/th08_headless_probe --executable build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage 6b --difficulty 0 --seed 0 --actions reports/local/stage6b/baseline.actions --frame 853 --through-frame 854 --output-dir reports/local/stage6b/early
```

The baseline intentionally exits 2. It hits fired, untransformed bullet slot 664:
player bounds `[373.174988,431.174988,374.825012,432.825012]` overlap hazard bounds
`[371.920898,428.774048,375.920898,432.774048]`. Movement consumes input 133 from
update 853, while update 854 samples 4165. Nine replacements at 854 all collide;
replacing 853 with left/up-left/down-left survives the observed update 854. The
real-data CTest guards these witnesses and verifies diagnostics leave the current
run’s trace unchanged (the historical 4060221407534777929 is separately labeled), and compares each branch tape at O0/O3 when
configured. It does not assert a whole-stage solution or an independent physics oracle.

The probe serializes its nine fresh processes and writes tapes, per-branch JSON/TSV
and `summary.json` into the output directory. It checks the projection at the common
prefix, rejects an out-of-tape frame and removes an old summary before a rerun. It
records actual total replayed updates, process wall time including initialization,
output and cleanup, and per-child maximum RSS in KiB. Local runs use approximately
73 MiB per child; use `ulimit -v 2097152` for a 2 GiB address-space cap if desired.
No remote host is needed. Repeated prefix work is explicit; no unsafe native snapshot
or shared action-dependent future is introduced to hide that cost.

The maintained seed-0 sweep enumerates 56 Easy standard/Extra practice checkpoints.
The Release `spell-portfolio` run completed 54 and retained 2 genuine collision
prefixes: bullet IDs 202/203. Bounded native-prefix repair additionally completes both,
so the combined documented recipe solves 56/56. All 56 rolling tapes
then agreed in a fresh process on the semantic fields used by `agree()` above. This is
broad algorithm evidence, while the smaller real-data CTest keeps the affected
transform/profile boundaries practical to rerun on every local change.
The ID204 change was compared with its preceding main checkpoint across all 56 cases:
the other 55 semantic reports and action tapes remained unchanged, and both versions'
tapes freshly replayed. Continuous Extra also remained at its earlier ID192 collision
on update 7424 at that historical checkpoint. The later ID192 change advances Extra below.

The ID139 comparison runs the previous portfolio and the new isolated profile on the
same corrected laser-item runtime: 49 versus 50 complete spells. All 55 unaffected
semantic records and literal action tapes agree; all 112 tapes freshly replay. Stage
6a changes from its preserved 48302 collision to a 61041 clear. Older aggregate
reports retain their original numerical/runtime profile rather than silently receiving
corrected item scores or digests.

The subsequent ID32 comparison changes only ID32 among 56 spells (50 to 51 clears):
all 55 unaffected semantic records and literal action tapes agree, and all 112 tapes
freshly replay. Continuous Stage3 changes from collision7443 to clear39767. The new
13-update horizon and 81 one-update-leg paths are isolated to ID32; generic projection
and tie-breaking are unchanged.

The subsequent ID167 comparison changes only ID167 among 56 spells (51 to 52 clears):
all 55 unaffected semantic records and literal action tapes agree, and all 112 tapes
freshly replay. Continuous Stage6b advances from ID167 at 42560 to the retained ID183
collision at 54401. This does not increase the stage-clear count.

The subsequent ID183 comparison changes only ID183 among 56 spells (52 to 53 clears):
all 55 unaffected semantic records and literal action tapes agree, and all 112 tapes
freshly replay. Continuous Stage6b advances from its preserved ID183 collision54401
to clear58853, adding the eighth complete stage. The prior ID167 report retains its
historical failure; the current single Stage6b fixture belongs to ID183.

The subsequent source/body comparison uses a complete separately compiled source snapshot
of preceding main 54f926d (original headers, Session, CLI and native units), rather than
an old-adapter/new-layout hybrid. It changes 53 to 54 clears, preserving all 53 successes.
Only ID192 and the still-failing ID203 prefix differ; 54 other semantic records and
literal tapes agree, including IDs201/202. Both versions of all 56 spells and Extra
freshly replay (114 tapes). Extra advances7424→67738, stopping in ID202.

ID192's focused five-record report preserves baseline 466, complete seeds 0/65535 at 3692,
seed 1 collision 1470 and actual Extra 67738. All agree with fresh O3 and O0 replays.
The H12/81-path profile costs 299052 candidates and 11074.8ms decision time for seed 0,
versus 4194 candidates/84.5ms for the 466-update baseline; these are unequal workloads.
At preceding checkpoint bd678, ID203's four-record report retained baseline 1711
and failures 1711/4823/2896 with bounded WAIT/vector projections; the current five-record
report below supersedes that partial evidence.
Native-linked regressions check WAIT/vector activation and final-clear boundaries,
DESPAWN's last lethal movement, 27 body source/lifecycle/clock guards, native movement
containment and the reciprocal-multiply size cancellation boundary. Both O0/O3 pass.
Independent native differential checks cover 23976 WAIT/vector updates and complete
ID192 tapes, including continuous Extra's delayed EndSpell-immunity transition.
That preceding bd678 normal CTest took 401.05s: 26 core tests passed; its native aggregate ended
at the unchanged historical Stage6b golden assertion described below, not a new gate.

The subsequent ID203 change keeps the rolling portfolio at 54/56 and adds one complete
seed 0 wrapper through bounded native-prefix repair. Compare against a complete separately
built bd678 source snapshot: the other 55 spell tapes/semantic results and continuous
Extra remain unchanged; the new rolling ID203 fails 787. Every comparison tape freshly
replays. The repaired 5492 tape is a separate verified outcome with full search costs,
not a replacement of the rolling failure record or a hardcoded route.
The final search used 91 candidates and 138198 native candidate updates, including 119908
replayed-prefix updates, in 384041ms. Initial execution used 787 updates/3919.03ms; the
built-in final replay used 5492 updates/875.446ms. Zero interrupted updates were charged.
The selected tape digest is 2771752472810487521 and independently agrees at native O0/O3.
The preceding f372 normal CTest took 799.97s and stopped at the old unconditional
Stage6b digest assertion. The latest expanded protocol suite
also passes separately. These host timings include process setup/artifacts and concurrent verification; they are
not the candidate policy's decision-only time or a performance guarantee.

The optional `headless_repair_protocol` test uses an explicitly synthetic child process
to reject malformed/duplicate/trailing JSON, wrong scene/prefix/hold/exit artifacts,
existing output directories and a TERM-ignoring timeout. It checks conservative charges
for semantically invalid children. Native policy tests check the pending-input step,
all nine first-action families, deterministic ties and 16533-expansion bound. The real-DAT
ID203 group checks baseline 1711, rolling 787 for three seeds, strict replay/prefix control,
and the complete bounded search with fresh O3/O0 replay. Its public JSON records include
child/repair executable SHA256 identities; `source_revision` remains the imported native
runtime provenance and does not identify a planner build. Historical numerical-profile limitations remain explicit under the v1 semantic contract.

`CASE_GROUP=id203` accepts `-DREPAIR_EXECUTABLE=$PWD/build-headless/th08_headless_repair`
to include the search (the normal CTest supplies it). The tracked five-case report
reuses that frozen successful search, then freshly replays its selected tape at O3/O0;
original search costs are retained. Optional `REPAIR_RESULT_DIR` requires both recorded
executable hashes to match and a complete result, and labels `search_reused`; fresh search
remains the default. Reproduce the complete solve with:

```sh
./build-headless/th08_headless_repair --executable build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage extra --spell-id 203 --difficulty 4 --seed 0 --frames 15000 --output-dir reports/local/id203-new-search
```

The output directory must not already exist. `summary.json` separates initial,
candidate/prefix and final-replay work; `processes.tsv` and `process-costs.json` retain
process outcomes and conservative unverified-update bounds. A budget-limited or malformed
run is not completion. All failed candidate reports/tapes remain available for diagnosis.

### Easy stage sweep

The 2026-10-02 local sweep used every supported stage entry, Easy, seed 0, the Release
O3 native profile and a 100000-update cap. Stage mode keeps
shooting enabled and advances original message scripts. Each execution wrote an action
tape and every tape agreed in a fresh process on the semantic fields and collision object
used by `agree()`:

```sh
mkdir -p reports/local/easy-stage-sweep
for stage in 1 2 3 4a 4b 5 6a 6b extra; do
  ./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage "$stage" --difficulty 0 --seed 0 --strategy spell-portfolio --frames 100000 --actions "reports/local/easy-stage-sweep/$stage.actions" --output "reports/local/easy-stage-sweep/$stage.json" || [ $? -eq 2 ]
  ./build-headless/th08_headless --dat game_data_donottrack/th08.dat --stage "$stage" --difficulty 0 --seed 0 --frames 100000 --replay "reports/local/easy-stage-sweep/$stage.actions" --output "reports/local/easy-stage-sweep/$stage-replay.json" || [ $? -eq 2 ]
done
```

| Stage | Outcome | Updates | Blocking spell | Decision ms | Execution ms | Peak bullets |
|---|---:|---:|---:|---:|---:|---:|
| 1 | complete | 24135 | - | 1828.4 | 4009.8 | 258 |
| 2 | complete | 32448 | - | 1581.4 | 3370.9 | 293 |
| 3 | complete | 39767 | - | 2195.0 | 3957.6 | 294 |
| 4a | complete | 22089 | - | 7354.6 | 11400.8 | 528 |
| 4b | complete | 43700 | - | 2675.4 | 5216.4 | 548 |
| 5 | complete | 43348 | - | 7025.9 | 10772.8 | 665 |
| 6a | complete | 61041 | - | 27644.8 | 33844.9 | 1201 |
| 6b | complete | 58853 | - | 22694.2 | 27796.8 | 1199 |
| extra | collision | 67738 | 202 | 32346.5 | 41409.4 | 1536 |

The corrected-runtime stage records, including the subsequent ID32 Stage3 and ID167/183
Stage6b and ID192 Extra reruns, cover 393119 updates and eight clears. All nine tapes freshly replayed.
These Intel Xeon Platinum 8573C/GCC 14.2 samples were run
alongside other verification; timings are workload records, not performance guarantees.
The earlier AMD EPYC/GCC12 five-clear sweep is superseded for current coverage. The
older 22176-update Stage 1 CTest uses the different `reactive` action tape and remains
a separate baseline.

Acceleration removes wall-clock waiting and presentation work while retaining every
original calc-chain update, timer increment and shared RNG consumer in that chain.
The virtual clock depends only on executed update count, never CPU throughput. Do not
accelerate by multiplying dt, subsampling actions/collisions, shortening spell timers,
or sharing action-dependent futures. These change the experiment's semantics.

Compare a second native library optimization level against the exact same tapes:

```sh
cmake -S . -B build-headless-o0 -DCMAKE_BUILD_TYPE=Release -DTH08_HEADLESS=ON -DTH08_HEADLESS_OPTIMIZATION=0
cmake --build build-headless-o0 --parallel 2 --target th08_headless
cmake -S . -B build-headless -DTH08_HEADLESS_COMPARE_EXECUTABLE="$PWD/build-headless-o0/th08_headless"
ctest --test-dir build-headless --output-on-failure -R '^headless_real_data$'
```

`TH08_HEADLESS_OPTIMIZATION` controls the imported game translation units (0..3),
not planner budgets or frame semantics. The checked GCC 12.2 O0/O3 tapes match all
reported semantic fields and every frame's diagnostic projection for these cases.
This checks acceleration within the native profile, not every spell or retail x87.
The original calc chain includes collision/feedback; the policy's extrapolation is
only a proposal, never the acceptance oracle.

Reports separate `simulation_ms` (calc-chain/queue updates minus timed native file
open/read/write/seek/stat/close wrappers), `file_io_ms`, `decision_ms` (observation and
policy/tape selection), `diagnostics_ms` (optional ring copies), and `execution_ms`
(whole loop including projection/trace work and I/O).
Initialization, DAT hashing, tape loading, output serialization and cleanup are outside
these loop measurements. CPU decoding during an update remains simulation work; compare
the same profile/tape. Run serially and record hardware/compiler/outcome/budget alongside
timing. The checked native host is Linux x86_64, AMD EPYC 7B12, GCC 12.2, Release.

Fresh-process replay checks execution agreement and RNG/feedback, not an independent
physics oracle. Original runtime globals are not yet branchable solver checkpoints.
Keep the scope in [Scenarios](SCENARIOS.md) when interpreting complete-scene reports.

## Optional pinned-source comparisons

Use a fresh ignored checkout directory; do not repurpose an existing working copy
that may contain local changes. Adjust the path below if `.cache/th08` already exists.

```sh
git clone https://github.com/N0zoM1z0/th08.git .cache/th08
git -C .cache/th08 checkout a45e99fb1942714e6edded20847e32a654d56f97
cmake -S . -B build-reference -DCMAKE_BUILD_TYPE=Release -DTH08_REFERENCE_SOURCE="$PWD/.cache/th08"
cmake --build build-reference --parallel 2
ctest --test-dir build-reference --output-on-failure
cmake --build build-reference --parallel 2 --target source_enemy_motion source_world_motion source_camera_particle source_spawn
./build-reference/source_spawn
```

The other three opt-in targets isolate comparisons already in `source_oracle`.
`source_spawn` separately compares 6000 allocation/post-store transactions with a
controlled immediate-ECL boundary. `source_effect_pool` checks effect allocation and
callbacks; passing DAT explicitly adds actual ANM/shared-pool cases. They do not execute
a complete original game world.

The source comparison chain covers collision predicates, all-seed RNG/selected ECL
random assignments, launch modes, fractional direction/acceleration clocks, transforms,
laser lifetime, slot selection, ANM scalar/control execution, enemy/world motion and
timeline behavior. Thin test adapters are scaffolding, not reconstructed complete actors.
Context layout/template ownership is also source-inspected and checked by native tests.

Native source equivalence is not retail executable, x87 or Direct3D equivalence. Fresh
replay shares the simulation implementation and therefore is not an independent physics
oracle. No original executable or running game was used for these results.

## Other retained tools and report maintenance

`th08_audit`, `th08_slices`, `th08_motion_cases`, `th08_animation_cases` and
`th08_timeline_cases` accept DAT and an output directory. They report structural or
restricted/component evidence, not whole spells. The two motion fixtures remain
useful regression baselines. `th08_first_spell` and entry/camera/effect tests are older
optional diagnostics, not mandatory solver checkpoints or the current roadmap.

Use `reports/local/` for experiments. Refresh only affected tracked outputs in
`reports/native/`, preserving their scope labels and input identities. Investigate
changed hashes, event counts, digests, routes or outcomes before publishing; timing-only
variation is expected. Preserve 64-bit digests as strings/integer-safe data.

The documentation CTest checks maintained local links/headings, selected report/status
counts and CMake's core-test count. It does not prove semantic correctness or inspect
historical preparations. Keep README, Status, Scenarios and generated evidence consistent.

Generated source-oracle translation units belong only in ignored build directories.
Their maintained generators share `source_probe_support.*`, not another tool's main().
Game DAT/EXE/assets are not distributed. Keep [third-party notices](THIRD_PARTY_NOTICES.md)
and the original preparation attribution; do not assign those artifacts a new license.

### Restored full verification (2026-10-03)

The H12/H32 repair adapter derives half-horizon holds from the selected failure's
homogeneous terminal unsafe run. It does not relabel ID202 as H32. The rebuilt
GCC14 O3/O0 run completes ID202 at 4712: 24 candidates, 102001 candidate updates,
100602 replayed-prefix updates, digest `7360258866840071988`. ID203 reproduces
5492 with 91 candidates and 138198 candidate updates. Both reports preserve their
rolling failures and independently replay at O0/O3. These two bounded repairs plus
the unchanged 54 rolling successes establish 56/56 seed-0 checkpoint recipes.
The fresh full CTest run passed 29/29 in 732.98 seconds, including native real-DAT,
O0 comparison and the historical Stage6b semantic-contract/18-probe checks.
Earlier timing paragraphs are historical measurements, not this run's timings.

The optional broad sweep now has a maintained producer. It records compact current
results, literal tape hashes and native executable identity while retaining the
full reports/tapes in the local output directory:

```sh
cmake -DEXECUTABLE="$PWD/build-headless/th08_headless" \
  -DDAT="$PWD/game_data_donottrack/th08.dat" \
  -DWORK="$PWD/reports/local/full-sweep" -P tests/headless_sweep.cmake
```

Its fixed 56-case rolling sweep and nine actual stage entries are independent of
the repair searches. A collision remains a collision in this report; adding two
isolated repair successes does not imply continuous Extra is complete.

A stage-sized Extra episode explicitly uses 144 candidates, 14400000 candidate
updates and 1800 search seconds, not the smaller isolated-spell budget. The first
fresh run used 9809223 candidate updates and 872386 ms of search, reaching ID203 at
69227 (`496550657910513129`, fresh O0 replay agrees). It is a failure, not stage clear.
To continue only an improved validated frontier without repeating earlier branches:

```sh
./build-headless/th08_headless_repair --executable "$PWD/build-headless/th08_headless" \
  --dat "$PWD/game_data_donottrack/th08.dat" --stage extra --difficulty 0 --seed 0 \
  --shoot 1 --frames 100000 --seconds 1800 --update-budget 14400000 \
  --resume-search "$PWD/reports/local/extra-episode1" \
  --resume-producer /path/to/exact/previous/th08_headless_repair \
  --output-dir "$PWD/reports/local/extra-episode2"
```

Keep the previous producer binary when rebuilding the search tool. Resume verifies
its hash, copies the selected artifacts, reconstructs the selected world afresh,
and reports prior/new/cumulative costs. An unchanged frontier is rejected rather
than silently starting the same finite proposal family again.

### Final seed-0 completion

The resumed Extra episode completes the actual carried-world stage at 92339,
ending after ID204, digest `15529738245795540729`. Its 87 candidates cost 6078636
native updates, with 6040785 repeated-prefix updates. Fresh source regeneration
used 69227 updates; final replay used 92339. Across both episodes the verified total
is 16117163 updates, with no interrupted uncertainty charge, and about 1591.15 seconds
of managed process wall time. Independent O0 replay agrees; those additional validation
costs and broad-sweep runs are separate from the managed episode totals.
The final integrated normal CTest passes 29/29 in 692.31 seconds.

After the rolling sweep and repaired searches, generate the current combined report:

```sh
cmake -DEXECUTABLE="$PWD/build-headless/th08_headless" \
  -DCOMPARE_EXECUTABLE="$PWD/build-headless-o0/th08_headless" \
  -DDAT="$PWD/game_data_donottrack/th08.dat" \
  -DSWEEP_DIR="$PWD/reports/local/full-sweep" \
  -DSPELL202_DIR="$PWD/build-headless/headless-regression/id202-repair" \
  -DSPELL203_DIR="$PWD/build-headless/headless-regression/id203-repair" \
  -DEXTRA_DIR="$PWD/reports/local/extra-episode2" \
  -DWORK="$PWD/reports/local/completion-verification" \
  -DOUTPUT="$PWD/reports/native/archive/headless_current_summary.json" \
  -P tests/headless_completion.cmake
```

This verifies three repaired tapes afresh at O3/O0, checks binary/data identities,
and embeds the rolling failures, prior Extra search, hash-bound lineage, successful
searches and replay reports. The primary totals are 56 checkpoint recipes and nine
continuous stage recipes. The nested rolling-only totals remain 54/56 and 8/9.

### Multi-seed robustness baseline (2026-10-03)

The requested additional seeds were fixed before inspection: 1, 42 and 65535.
Each ran the same 56-checkpoint plus nine-continuous-stage rolling matrix and
frame caps, without automatic repair searches. All 195 fresh replays agree.
Use `-DSEED=<value>` with `tests/headless_sweep.cmake`; the default remains zero.

| Seed | Rolling spell clears | Rolling stage clears | Native updates including replay | Decision time, all 65 runs |
|---|---|---|---|---|
| 0 | 54/56 | 8/9 | 1160236 | 147.16 s |
| 1 | 51/56 | 7/9 | 1046086 | 113.64 s |
| 42 | 48/56 | 6/9 | 934060 | 100.44 s |
| 65535 | 50/56 | 5/9 | 916710 | 102.17 s |

These are rolling-only comparisons, not comparisons with seed zero's additional
repair wins. Earlier failed termination reduces work, so lower elapsed time is not
an efficiency improvement. Seed zero also ran alongside other verification;
these host timings are descriptive, not isolated performance benchmarks.

Retained spell failures (ID@update):
- Seed 1: 192@1470, 200@4161, 202@4216, 203@787, 204@3008
- Seed 42: 147@742, 192@727, 194@907, 200@1018, 201@2317, 202@4197, 203@787, 204@3008
- Seed 65535: 119@1511, 127@3964, 194@3236, 201@700, 202@3613, 203@787

Continuous-stage failures expose carried-world differences: ID127 in Stage6a
(seeds 1/65535), ID155 in Stage6b (42/65535), direct/imminent lasers in Stage4b
(42/65535), and different Extra failures. An isolated successful spell is not a
promise that the same spell clears inside a stage. ID201 seed 1 now completes on
this current native build; the earlier 3455 failure is historical evidence.

A diagnostic-only global H32 control, with the same nine focused constant paths,
avoids four local ID127/155 wall traps from identical replayed prefixes. It also
creates regressions: seed zero ID108/147 hit enemy bodies and ID175 hits an
unsupported polar transform. It is rejected as a general fix. No seed/frame or
new spell-ID branch is introduced to hide those failures. Seeds 7 and 2026 remain
reserved, uninspected holdouts until a justified candidate is frozen.


### Paused transform-contract experiment

The owner requested a tidy, published stopping point. The active source remains
at the reviewed body-observation/multi-seed checkpoint; the following experiment
was rejected rather than silently changing failure fixtures or tuning seeds.
The existing seed-zero completion report remains historical evidence for its
recorded native binary, not a newly regenerated search under this experiment.

Native-linked probes established three forecast defects:
- A VECTOR clearing at update 2 followed by a blocked POLAR produces x=2,4,8;
  the old unbounded vector projection gives 2,4,6. An allowed concurrent POLAR
  already changes the first update from predicted x=2 to actual x=4
- Native float recurrence gives x=8.480000496 at update 32 for vx=.1, ax=.01;
  the closed form gives 8.479999542. Exact float equivalence is not established
- WAIT timer 2 with the extra timer step and a subsequent POLAR gives x=1,2,6,
  whereas the nominal three-update linear certificate incorrectly gives x=1,2,3

The draft adds bounded VECTOR recurrence, explicit supported-clock checks and an
inactive-program linear certificate. It does not enable generic WAIT/POLAR or H32.
Focused core tests pass 26/26 and native bounds checks pass O3/O0 before the final
INT_MAX safe-prefix clamp; that final clamp has a focused O3 pass. The full normal
suite was **not** completed for this draft. Its old ID202 boundary assertion failed
because rolling collision moved from 4204 to 4212; it was not rebaselined.

The already-started four-seed, 65-route sweep completed with fresh replay agreement
for all 260 routes. Compared with the published rolling baseline:

| Seed | Draft spell clears | Draft stage clears | Published spell/stage clears |
|---|---|---|---|
| 0 | 54/56 | 8/9 | 54/56, 8/9 |
| 1 | 51/56 | 7/9 | 51/56, 7/9 |
| 42 | 47/56 | 6/9 | 48/56, 6/9 |
| 65535 | 49/56 | 5/9 | 50/56, 5/9 |

New failures are ID195 seed42 at 2752 and ID192 seed65535 at 2631. The unchanged
seed-zero bounded repair budgets still complete ID202 (4712 updates, six candidates,
25764 candidate updates) and ID203 (5492 updates, 77 candidates, 85144 candidate
updates), with built-in fresh replay. Independent O0 comparison of these draft
repair tapes was not run. No repaired Extra search or new completion claim follows.

[Compact results](../reports/experiments/transform-contract-rejected.json) record
binary/report hashes, failures, costs and verification limits. The accompanying
[archived patch](../reports/experiments/transform-contract-rejected.patch) applies
to fdc8f8fb0ccd4119e7c9cf0b17b5eb84961a9337, but is not compiled or applied.
The sweep binary precedes its final INT_MAX clamp; that edge change must not be
mistaken for a fully swept final binary. Do not apply it without integration review
and the same regression matrix. No DAT, extracted assets or action tapes are included.

Remaining work is bounded and separate: fix the demonstrated transform contracts
without losing successful scenes; cover lethal body motion only with valid
lifecycle/child/timeline certificates; improve the two late laser warnings without
hardcoded frames or unsupported geometry. Global H32 remains rejected. Holdout
seeds 7 and 2026 remain uninspected, and work is paused until requested again.
