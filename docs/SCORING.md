# TH08 score mechanics

All citations are to `third_party/th08/src/`. Values are **displayed** points unless
stated: `GameManager::AddScore(x)` stores `x / 10` (GameManager.cpp:201) and the HUD
shows stored × 10. The stage clear bonus is the exception: it is computed in stored
units `C` and paid as ten `AddScore(C)` calls, about `10·C` displayed (Gui.cpp:1114).

## Counters

| Counter | Scope | Notes |
| --- | --- | --- |
| `pointItemsCollected` (P) | Whole run | HUD "Point"; prices time orbs; extends at 100/250/500/800/1100 (story) and 200/666 (Extra) |
| `pointItemValue` (PIV) | Whole run | Starts at 300,000 on Lunatic and Extra; never decreases |
| `graze` | Whole run | +3/+2/+1 per graze at extreme/moderate human/otherwise |
| `currentTimeOrbs`, `totalTimeOrbs`, `pointItemsCollectedInStage`, `grazeInStage` | Per stage | Reset at each stage start (GameManager.cpp:895–908) |

## Income sources

**Point item** (`Item::CollectPoint`, ItemManager.cpp:458): value is PIV if the item's
y is above the shot's collection line (Scarlet 128, Magic 160), otherwise
`PIV/2 − (y − line)·floor(PIV/1000)`; floored to 10, then **doubled at extreme
human gauge**. Height is the item's y at pickup.

**Time orb** (`Item::CollectTimeOrb`, ItemManager.cpp:602): pays `max(100, 10·floor(P/2))`,
which is 5·P; a flat 10,000 once 2000 points were collected in one stage (never
reached). Each orb also adds 8000 to the active spell bonus (capped at its start
value for ordinary cards) and ±111 to the gauge (+ if focused).

**PIV growth** (`AddTimeOrbs`, GameManager.cpp:208): +10 whenever the stage's orb
total becomes odd, so `PIV = 300,000 + Σ_stages 10·ceil(T_s/2)`. The story record
ends at exactly 300,000 + 1,172,880 = 1,472,880.

**Orb generation:**

- Spell capture: 100–1000 pending orbs (1000 if captured in the first seventh of the
  timer, linear to 100), 700 for timeout cards; 1000 pending spawns 935 orbs.
  Orbs spawning at x outside [−64, 448] are lost.
- Graze at extreme youkai while a boss is present: 1 orb, +1 during a spell for
  non-lasers, +1 more except for solo youkai; none for solo humans 4/6/8.
- Kills at either extreme gauge: +1. Extreme human shot damage: +1 per 40 accumulated damage.
- Chain release (`DetachEnemyChain`, EnemyManager.cpp:241): when a parent with
  attached children dies, each child bursts `min(2j + 10, 26)` orbs for teams
  (j = chain depth) plus a cancel circle that converts nearby bullets to orbs, and
  the parent drops `2 × linkedChildCount` more.
- The item pool has 2096 slots; a time orb fails if the next ring slot is busy
  even when others are free, and failed spawns are never retried (ItemManager.cpp:67).

**Spell bonus**: starts at the card's value (Lunatic 10 M in Stage 1 up to 33 M for
the Final B Imperishable Night cards), decays linearly to about 1/7 over the timer,
and is paid 130 updates after the card ends. A timeout card (survival) has no decay
and no cap: every orb adds a full 8000.

**Graze**: 2000 each (4000 at moderate youkai). **Phase-end cancels** convert every
bullet into a star worth 2000–8000. **Extreme gauge**: +100 per frame.

**Stage clear** (Gui.cpp:1062): `C = Base + 50·G_s + 5000·P_s + 100·T_s`, plus at the
final stage `2,500,000·lives + 500,000·bombs` and, in Final B, `2,000,000·(12 − clock)`;
× 1.5 on Lunatic (× 2 Extra). Displayed per unit on Lunatic: **75,000 per point,
1,500 per orb, 750 per graze unit, 37.5 M per spare life, 7.5 M per bomb, 30 M per
clock tick**. Stage bases total 240 M for 1→2→3→4A→5→Final B.

**Night clock**: +1 per stage when collected orbs reach the threshold (Lunatic
3000/7200/8800/9999/9999), otherwise +2. With every threshold met the clock is 5
entering Final B (210 M bonus); Final B adds ticks only for uncaptured Imperishable
Night cards. The Last Spell gate counts collected, pending and on-field orbs.

## Gauge

Teams reach extreme at ±8000 (limit ±10000); solo Youmu at ±3000 (limit ±5000);
the Youmu team at −3000 human. Solo humans (including solo Sakuya) cannot go
youkai; solo youkai cannot go human. Shooting moves the gauge by up to 21 per frame
toward the focus side; orb pickups move it ±111 each; kills ±200; grazing as youkai
+100. A 935-orb capture reward therefore saturates the gauge in the direction of the
focus state at pickup.

## Whole-run model

Let P(t), T(t) be cumulative points and orbs, m = 2 when a point is doubled and
h = 1 at full height. Then approximately

```text
S ≈ Σ_points m·h·(300k + 5·T(t))      point cash
  + Σ_orbs   5·P(t)                   orb cash
  + 240M + 75k·P + 1.5k·T + 750·G     clear bonuses (Lunatic)
  + 37.5M·lives + 7.5M·bombs + 30M·(12 − clock)
  + spell bonuses + cancels + graze + gauge frames
```

Every (orb, point) pair contributes 5 once, whichever comes first, so without
doubling the cross term is exactly `5·P·T` and its order does not matter. With
doubling the symmetry breaks: an orb collected **before** a doubled point is worth
10 for that point, one collected after it only 5. The marginal final values are:

| Unit collected at time t | Final value (all later points doubled, full height) |
| --- | --- |
| Point item | `600k + 5·T_final + 5·T(t) + 75k` (+ extend steps) |
| Time orb | `10·P_final − 5·P(t) + 1.5k` (+ spell bonus credit) |

For the story record (P = 1628, T = 234,573) a point is worth 1.9–3.0 M and an
orb 10–18 k, so one point item is worth roughly 150 orbs. A Stage 1 orb is worth
about 1.7× a Final B orb; a Final B point is worth about 1 M more than a Stage 1 point.

## Why local cash misleads

A planner that ranks futures by displayed score at the end of a short horizon
credits each unit only with its **immediate** pickup value:

| Unit | Credited by horizon cash | Actual final contribution | Ratio for an early-stage unit |
| --- | --- | --- | --- |
| Time orb at P(t) ≈ 50 | 5·P(t) ≈ 250 | ≈ 17,500 | about 70× too low |
| Point item at T(t) ≈ 10k | 2·PIV ≈ 700k | ≈ 1.9 M | about 2.7× too low |

The bias is largest at the start of the run, which is exactly where the record
trails the same-shot human (Stage 1–3 orbs 12,755/33,391/36,052 versus
21,873/37,662/46,219; see [Lessons](LESSONS.md#4-economics-facts)). It also explains
why local scene cash, orbs or PIV have repeatedly failed to order complete
futures: none of them is the final value of the state.

## Story record by stage

From task 2113 (`stage_results` in [results.tsv](experiments/results.tsv)):

| Stage | Points | Orbs | Graze | PIV at clear | Score at clear |
| --- | ---: | ---: | ---: | ---: | ---: |
| 1 | 55 | 12,755 | 4,758 | 363,780 | 130,212,230 |
| 2 | 173 | 33,391 | 8,084 | 530,740 | 505,494,810 |
| 3 | 321 | 36,052 | 8,119 | 711,000 | 1,224,907,800 |
| 4A | 376 | 50,628 | 19,061 | 964,140 | 2,386,640,830 |
| 5 | 269 | 26,276 | 11,692 | 1,095,520 | 3,433,629,060 |
| Final B | 434 | 75,471 | 27,354 | 1,472,880 | 6,653,194,550 |
| **Total** | **1,628** | **234,573** | **79,068** | | |
