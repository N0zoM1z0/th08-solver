# Experimental replanning branch value, version 2

`weights.tsv` is a 1697-byte C++-trained logistic snapshot, SHA-256
`3ff65d500af7a22d20117b9e5fec682f2667956d5cdefa62da078ddf2f5fa8d9`.
It is opt-in. Saving it does not establish better Native completion, whole-story
reliability, retail control or a universal deadline guarantee.

The explicit v2 target is `replan-h24-after24-v1`. Every fork inherits its own
Native world/RNG and the already-decided parent Policy. It submits the proposed
23 controls, repeats the last on Native call 24, warms 23 intervening observations
without search/tie draws, then resumes the inherited no-model H24 baseline after
update 24 through update 64 or the declared spell terminal. Survived/full-window
and Native-complete branches are positive; Native deaths are negative; faults and
incomplete surviving windows cannot label training data. This is a conditional
baseline-policy continuation, not optimal expert value or certified safety.

Features, clipping and train-only normalization follow
[v1](../branch-value-v1/README.md): 24 current/proposed inputs, with no seed, spell
ID, absolute time or teacher future. Training still uses 400 deterministic
full-batch steps, rate 0.05, L2 0.01 and equal total mass per parent. Native branch
labels now include actual executed submissions and separate child-policy work.
The trainer rejects mixed held-last/replan targets; old v1 bytes remain unchanged.

The same ordered 16 training/eight tuning seeds and 103/107/111/202 assignments
as v1 are reused as known training/tuning data. Twenty-four regular episodes use
stride128, first observation128, maximum32 checkpoints. Three additional known
failure-tail strata use inclusive windows and stride32, maximum8 checkpoints:

| Split | Seed / spell | Window | Prior failed nominal terminal |
| --- | --- | --- | --- |
| Train | 13055 / 202 | 3174..3429 | 3429 |
| Train | 48837 / 202 | 3304..3559 | 3559 |
| Tune | 40289 / 202 | 4294..4549 | 4549 |

These are alternate samples of the same whole parents. They preserve exact nominal
actions, terminal, score, RNG, calculation counts and actor/observation projection.
Sampling identities differ; parent identities and weighting do not. Grid anchors
avoid the regular checkpoints; duplicate parent/update/proposal rows fail rather
than being deduplicated. All parent outcomes and additional costs remain charged.
All 24 old evaluation seeds are excluded from subsequent independent draws.

Train has 1648 rows, 1535 positive, from 16 parents; tune has 852 rows, 787 positive,
from eight disjoint parents. Train SHA-256 is
`f8f6f1254535dc922513f7adc4cd6013a3ab6b04d7ad251d5d113a2884ae6b34`;
tune SHA-256 is
`df5d754d9d764dfafd1fed680e6598a06fbc1cbc6f8af7d9facbc44d57e5f7b9`.
Episode-weighted train BCE/Brier are 0.20254/0.05163; tune 0.21196/0.05458.
The target and class balance differ from v1, so comparing their raw losses does
not establish model improvement. A TRAIN-fitted equal-parent constant prior
(`p=0.9413043`) gives train BCE/Brier 0.22336/0.05525 and tune 0.23480/0.05889.
The model improves modestly on that baseline on this deliberately tail-biased
conditional sample; this is not population calibration. The complete row/sidecar
audit checks nominal-prefix identities, actual submissions and train-only scaling,
but does not independently re-execute every Native branch outcome.
All three global multipliers 0.25/1/4 complete 7/8 tuning proxies. The frozen
smallest-weight tie rule selects 0.25.

The subsequent frozen 172-trial comparison is complete. On 24 new Extra 202 seeds,
baseline completes 14/24, v1 and v2 each 9/24, and the generic no-model profile
23/24. All four arms complete 8/8 each on 107/111 and 3/3 dense 103 controls.
V2 therefore shows no controller benefit on this bank despite its conditional
classification improvement. It remains opt-in; neither weights nor the selected
multiplier were retrained or tuned on these outcomes. These shared spell-practice
proxies do not establish whole-stage/story reliability or a live deadline.

Collection uses H24/beam192/cell16/guard16/reach-filter, hard phantoms and motion
envelope; child-birth/heading options, lag, fallbacks, legacy risk and learned
models are off. Source archive SHA-256 is
`095ac59b8c5526e0e4364aa568f55e83bd8c255416b880cc7e67d0f92771d613`;
collector SHA-256 is
`9c863acb340f6442836b5c9341ebcbf0fa7a08c0fd6079a602bc01801db8e234`.
Manifest SHA-256 is
`bd126c3058d2c7ed2cb1c8f6e841a83ca1f145e5649d6ade291cd4c3eba1d57e`.
GCC13.3 Release on EPYC9654 uses policy `-march=native` and `-ffp-contract=off`;
Native library/DAT identities are unchanged from v1. Full source/binaries, labels,
actual branch inputs, parent tapes and failed receipts stay local and ignored.
The saved snapshot is not retrained on evaluation outcomes. Global weight tuning
and new-seed evaluation are separate gates owned by [Status](../../docs/STATUS.md).
