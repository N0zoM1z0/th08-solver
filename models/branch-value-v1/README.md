# Experimental conditional branch value, version 1

`weights.tsv` is a 1670-byte L2 logistic model, SHA-256
`8132db97530939510d0f5b6898103de6e8f40ad79671ce48a85bf603cfabe828`.
It is an experimental search aid, disabled by default. Offline fit quality does
not establish improved native completion, complete-stage reliability or 60 FPS.

The target is the native outcome of one proposed 24-update movement/focus prefix,
followed by holding its last input through update 64 or the explicit spell
terminal. The already latched input owns movement one. Every input keeps shooting
and forbids bombs. Each proposed sequence receives its own forked native world,
including carried state and RNG; no action branch borrows another branch's future.
Survived/full-window and native-complete branches are positive; native deaths are
negative. Faults, missing/truncated results and incomplete surviving windows are
rejected. This is not optimal state value or a collision-safety certificate.

The 24 named inputs use current observations and the student's proposed endpoint:
position/travel/wall distances, final movement/focus, ordinary search score,
local current bullet/ghost/spawning/actor counts, canonical-motion mismatch,
current-model forecast cell counts, form and global observed hazard counts.
They contain no gameplay seed, spell ID, absolute episode time or oracle future.
Features are clipped to [-32, 32]. The file records training-only means/scales;
standardized inputs are clipped to [-8, 8]. Inference is one linear logit/sigmoid
per retained endpoint, used only to rank complete H24 beams.

Training uses 16 independently drawn seed/parent episodes and 1584 branch rows
(815 positive). The disjoint tuning split uses eight seed/parent episodes and
820 rows (413 positive). Both rotate 103/107/111/Extra 202 with shot 0. Raw
datasets and every parent outcome remain local. Dataset SHA-256 values:

- Train: `da6e9fdcc4c3d1807054dbb279836f84bab2dfb2089a25fa20e2b55762cdf941`.
- Tune: `867e1fd3c0fc89d6616b192249cba003a5cde04a9a210cdd2e6f06f54b774e4f`.

The C++17 trainer performs 400 deterministic full-batch steps, learning rate 0.05,
L2 coefficient 0.01 and equal total weight per parent episode. Tuning rows do not
fit weights or normalization. Episode-weighted BCE/Brier are 0.65494/0.23135 on
training and 0.65674/0.23217 on tuning; probabilities are not certified calibrated.
No inherited legacy-risk table or gameplay RNG resampling contributes.
Data generation uses the pinned native library SHA-256
`17400047d94bc62114a112b9209f63c287e2c06302cd67335a4499549eb30e86`
and DAT SHA-256
`9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb`.
Producer/compiler details and full manifests are retained in local experiment
receipts. The paired spell-practice evaluation is summarized below; current
scoring records are maintained separately in [Status](../../docs/STATUS.md).
The frozen producer archive SHA-256 is
`332619d8f370b5dffc31c4fb1a14975efb3be021b841fe247ccc5400eb7e6b5a`;
collector binary SHA-256 is
`d078b2470ec2362ad78a0a706f4c952057c0d5b4e43fa03714a0604b3666d1b5`.
Collection used GCC 13.3 Release on an EPYC 9654, policy `-march=native`, explicit
`-ffp-contract=off`, H24/beam 192/cell 16/spawn-guard 16/reach-filter, hard phantoms
and the motion envelope. Every 128 updates from update 128, up to 32 checkpoints
sample four retained score ranks. Each parent has 20000-update/1800-second limits;
child windows are separately bounded and charged. No legacy-risk tables, lag or
fallbacks contribute.

The single OS-entropy draw excluded known historical seeds before assigning whole
episodes; there was no outcome filtering. Ordered train seeds are 14406, 33406,
55891, 13055, 58642, 54103, 63351, 48837, 52104, 36764, 62089, 43823, 2693,
55626, 29921, 9195. Ordered tuning seeds are 56130, 10825, 6900, 40289, 63700,
30773, 51365, 31065. Within each split, cases cycle 103/107/111/202 in that order.
The independently assigned evaluation seeds are 22390, 3464, 8381, 40863, 33619,
12501, 36274, 43476, 1477, 48816, 32288, 55941, 38826, 12344, 58495, 15326,
33489, 44154, 43066, 17196, 24670, 17532, 22551, 27092. These evaluation seeds
must not become future training/tuning data under a claim of untouched evaluation.

The first producer retains parent tapes, checkpoint/rank identities and its exact
binary/source to reconstruct sampled prefixes. The maintained collector also
writes explicit branch-input sidecars. This first dataset does not retroactively
gain those sidecars; it remains distinct from later collections.

The supported online configuration requires H24, hard phantoms and the current
canonical-motion envelope, without legacy-risk tables, delayed execution or
warm/held/tail fallbacks. `--value-model` loads this snapshot explicitly;
`--value-weight` controls its terminal ranking penalty. Weight selection is a
separate frozen global tuning experiment, followed by independent-seed native
comparisons against H24 and H64. The tested global penalties 0.25/1/4 each complete
7/8 tuning episodes, so the predeclared smallest-weight tie rule selects **0.25**.
This does not change the model weights or use independent evaluation outcomes.
The frozen independent comparison finds an encouraging 202 signal, with retained
regressions and unresolved paired uncertainty. It does not establish a population
reliability or whole-stage timing guarantee. Future retraining needs a new
snapshot and split ledger. This experimental model remains explicitly opt-in.

The later frozen 172-trial comparison uses this unchanged snapshot on 24 different
Extra 202 seeds: v1 completes 9/24, versus baseline 14/24, v2 9/24 and the generic
no-model profile 23/24. All arms pass the smaller 107/111/103 controls. This second
bank retains real regressions and does not support a general benefit claim from
the first bank's encouraging signal. No weights or penalty were changed from
these evaluation outcomes. Original paired-run receipts remain local; the
comparison above states their measured scope.
