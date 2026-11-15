# Campaign lessons, October 26–29, 2026

This condenses the dated experiment journals of the first scoring campaign
(tasks up to 2209). The original journals are in Git history at tag
`pre-cleanup-20261115` under `docs/experiments/`. Task numbers refer to rows of
[results.tsv](experiments/results.tsv) and the
[campaign ledger](experiments/campaign-ledger.tsv). Unless marked, scores are
complete NMNB Native runs with fresh and natural verification; captures optional.
[Scoring](SCORING.md) explains the mechanics behind these numbers.

## 1. Score progression

Scarlet seeds: Extra 9098, story 32796. Magic seeds: Extra 46997, story 1805.
Rows before task 786 predate these journals (October 26–27). Their causes come
from the `results.tsv` reason field, not from journal analysis.

### Scarlet complete Lunatic (story)

| Task | Score | Caps | Date | Gain | Cause |
| --- | ---: | --- | --- | ---: | --- |
| 669 | 5,011,063,470 | 35/37 | Oct 26–27 | first | Public H256 baseline: parent 100, human 5, upper grid, charge 1, reserve 1.5+240 |
| 723 | 5,262,569,250 | 32/37 | Oct 26–27 | +251.5M | Human Stage 2 spell-pose prefix, resumed after a host-memory abort |
| 722 | 5,317,717,330 | 35/37 | Oct 26–27 | +55.1M | Human Stage 2 road prefix, resumed |
| 737 | 5,383,092,450 | 35/37 | Oct 26–27 | +65.4M | Bounded Stage 3 road XY/fire/form pose guide, 45,000-input prefix |
| 768 | 5,544,203,730 | 35/37 | Oct 26–27 | +161.1M | Movement 4 continuation of the Stage 3 pose lineage |
| 794 | 5,594,687,960 | 35/37 | Oct 27 (VPS) | +50.5M | VPS full tail H256/publication 128/replan 64 from the 45,000 prefix |
| 799 | 5,621,456,240 | 36/37 | Oct 27 (VPS) | +26.8M | Same prefix, H128/64/32 tail; card 74 captured as well |
| 877 | 5,784,708,380 | 35/37 | Oct 27 | +163.3M | Kept the H256 world through card 47 (51,253 inputs), then a public H128 tail |
| 930 | 5,821,038,980 | 35/37 | Oct 27 | +36.3M | Kept 877 to card 171 entry, then H128/cash 4/reserve 1.2+120 tail |
| 1011 | 5,922,099,540 | 35/37 | Oct 27 | +101.1M | Final B road collector (H256/parent 100/human 5/collection 5) to first 151: 360 vs 348 points |
| 1022 | 5,961,138,320 | 35/37 | Oct 27 | +39.0M | Stage 5 source-clock NPC hints (H256/cash 4) + Final B human/collection 20 + reserve 1.5 tail |
| 1047 | 6,019,713,800 | 34/37 | Oct 27 | +58.6M | NPC-v5 source-clock road hints on raw 2/3/5/7 + dynamic horizon (road H256, spell H128) |
| 1114 | 6,130,475,890 | 34/37 | Oct 27 | +110.8M | 1047's first five stages + public Final B collector: 352 vs 308 points at 151 |
| 1172 | 6,291,093,190 | 36/37 | Oct 27 | +160.6M | Stage 2 spell plans: focused 17 (4,795 orbs), shoot/focused 25 and 29, then recompute |
| 1207 | 6,439,636,250 | 36/37 | Oct 27 | +148.5M | Card 77 shoot/focused 600 phase: 3,827 orbs vs 212, capture kept |
| 1225 | 6,512,532,100 | 36/37 | Oct 27 | +72.9M | Card 171 lower 368–432 feedback band in the new world: 16,788 orbs. **6.5B milestone** |
| 1381 | 6,513,456,180 | 36/37 | Oct 28 | +0.9M | Final-survival tail with ordinary movement 3 instead of 4 (V38 portfolio) |
| 1830 | 6,610,839,240 | 36/37 | Oct 29 | +97.4M | Card 77 retuned (H256/cash 0/reserve 600, top 368) and recomputed; card 171 top 208 retarget at its actual entry 146244 |
| 1847 | 6,618,962,600 | 36/37 | Oct 29 | +8.1M | Card 171 top 208/7/reserve 0 world (1838), ordinary suffix |
| 1931 | 6,622,525,640 | 36/37 | Oct 29 | +3.6M | Beam 384→64 on 187–191 |
| 1938 | 6,626,897,920 | 36/37 | Oct 29 | +4.4M | Kept 187 at beam 64; card 191 back to beam 384 |
| 2020 | 6,634,534,150 | 36/37 | Oct 29 | +7.6M | Native-32 portfolio that ranks certified motion prefixes by realized orbs (V192), last five survivals |
| 2030 | 6,635,012,890 | 36/37 | Oct 29 | +0.5M | Same portfolio, beam 128 on 191 |
| 2095 | 6,648,176,870 | 36/37 | Oct 29 | +13.2M | "Fine 144" guided pulses (1/2/4/8 updates, 9 directions, 4 fire/form modes) on 175–191 |
| 2113 | **6,653,194,550** | 36/37 | Oct 29 | +5.0M | Fine 144 with beam 64 on 191 only (3,286 orbs vs 3,021) |

Story total from 5,621,456,240 to 6,653,194,550: **+1,031.7M**. By source: road,
NPC and Final B collector compositions 877–1114 gave +509.0M. Early spell phases
(17/25/29, 77, 171) gave +382.1M. Retuning 77/171 in a changed world (1830) gave
+97.4M. All late-scene beam/portfolio tuning (1381, 1847–2113) gave only +43.3M.

### Scarlet Extra

| Task | Score | Date | Gain | Cause |
| --- | ---: | --- | ---: | --- |
| 667 | 2,872,562,940 | Oct 26–27 | first | Public H128/publication 64/replan 32/cash 4 baseline |
| 694 | 2,888,408,190 | Oct 26–27 | +15.8M | Human first road + private focused Keine prefix (19,000 inputs), then an H256 tail |
| 716 | 2,939,109,600 | Oct 26–27 | +50.7M | Same prefix; only the tail horizon changed (256→128) |
| 734 | 2,969,055,340 | Oct 26–27 | +29.9M | "human260" first road + Probe2 prefix: 634 points/26,103 orbs/4 captures |
| 744 | 2,979,717,840 | Oct 26–27 | +10.7M | Four 16-input cardinal-movement cash comparisons during 205 only |
| 761 | 2,980,273,700 | Oct 26–27 | +0.6M | Movement 4 on all Mokou active spells |
| 769 | 3,072,999,650 | Oct 26–27 | +92.7M | Second road pose guide + movement 4, resumed after a resource abort |
| 815 | 3,073,007,650 | Oct 27 (VPS) | +8,000 | Fire coordinate refinement: one accepted edit at input 67000 |
| 786 | 3,087,585,240 | Oct 27 (VPS) | +14.6M | VPS full tail H128/cash 4/movement 4 from the retained 19,000-input prefix |
| 1272 | 3,164,667,710 | Oct 28 | +77.1M | Card 193 ordinary restart (H128/cash 0) parent + recomputed public cash 4 tail |
| 1304 | 3,170,906,200 | Oct 28 | +6.2M | Card 205 ordinary restart H128/cash 0/movement 4 |
| 1314 | 3,179,193,860 | Oct 28 | +8.3M | 205 movement 3 (up/down/left cash comparisons). **First above the all-shot reference (+733,030); only Wine-validated record (1343)** |
| 1517 | 3,194,638,140 | Oct 28 | +15.4M | Card 192 lower-band feedback 368/600 world + corrected cash 4/movement 4 tail |
| 1539 | 3,203,494,130 | Oct 28 | +8.9M | 205 suffix cash 0/movement 4 |
| 1621 | 3,212,949,960 | Oct 28 | +9.5M | Card 197 lower-band world + card 202 feedback 368/600, cash 4 |
| 1740 | 3,255,579,400 | Oct 28 | +42.6M | Retimed 202 in the higher-point road world: all 45 point drops spawn (no 2096-slot pool exhaustion), 1,051 points |
| 1903 | 3,282,132,040 | Oct 29 | +26.6M | Card 195 true-no-fire feedback (V120) + focused capture reserve 1500, then F4 cash 4 |
| 1917 | 3,296,376,500 | Oct 29 | +14.2M | 205 beam 384→128, cash 0 |
| 2019 | 3,304,500,070 | Oct 29 | +8.1M | Native-32 realized-orb portfolio on 205 (5,952 orbs vs 5,475) |
| 2035 | 3,306,483,020 | Oct 29 | +2.0M | Native-32 portfolio on 203 with H256; 205 beam 128 |
| 2047 | **3,308,826,050** | Oct 29 | +2.3M | 205 Native-32 portfolio, H128, beam 64 |

Extra total from 3,087,585,240: **+221.2M**. Card 193 (+77.1M) and card 202
pool/retiming (+52.1M) are the largest contributions. Five successive card 205
profiles add +26.7M after 1903.

### Other shots and validation

| Item | Score | Note |
| --- | ---: | --- |
| Magic Extra 474 | 3,010,040,230 | Public early full tail; 17,000 inputs kept with three Keine captures; 14/14 |
| Magic Extra 597 | 3,013,913,970 | Upper grid only on last card 205; 14/14 |
| Magic story 487 → 568 | 5,025,195,380 → 5,162,703,270 | Captured-77 prefix; then early-cash H5/P128/R64 tail (35/37) |
| Magic story 575 | 5,107,553,670 | 37/37 all-capture category; human Stage 2 road prefix |
| Magic story 595 → 620 | 5,457,960,010 → 5,550,599,270 | Human Stage 3 road + upper-sample 47; then a human-pose Stage 4A road |
| Solo Sakuya Extra 1674 / story 1712 | 2,182,761,340 / 4,417,570,020 | First baselines, seeds 53346/24238; not records |
| Wine retail playback 1343 | 3,179,193,860 | Only retail-validated result. Newer Native records do not inherit this validation |

## 2. What produced complete-route gains

1. **Original-entry composition with recomputation.** Every record keeps a literal
   input prefix and replans the rest in its actual world. Example: 877 kept
   51,253 inputs and gained 163.3M. 1172 chained 1112→1122→1133→1141→1161, each
   recomputed. Copied suffixes and score arithmetic never produced a record.
2. **Phase plans for cards with large orb headroom**, measured by realized orbs:
   - 17: 440 → 2,922 (shoot) → 4,795 (focused 600).
   - 29: 1,084 → 3,408. 36: 731 → 2,764 (lost when the incoming world changed).
   - 77: 212 → 3,827 (+148.5M route).
   - 171: 7,788 → 16,788 at 368–432 → 17,504 at top 208 (+72.9M and +8.1M).
   - 195: true no-fire with a 1500 capture reserve gives +720 orbs (+26.6M route).
3. **Final B road point collection** at H256/parent-chain 100/human 5/collection 5.
   First-151 points rose 348→360 (+101.1M) and 308→352 (+110.8M). Point cash
   explains 96,315,080 of the 101,060,560 gain.
4. **Source-clock NPC road hints.** Keys are stage/sub/bossTimer/DAT IP, not
   absolute frames, because drift reaches −3184…+2560. Combined with road H256 /
   spell H128 scheduling: +58.6M (1047). Became the public `--spell-horizon`.
5. **Avoiding item-pool exhaustion at card 202 (Extra).** Retiming restored 45
   point drops: 990/1006 → 1051 points; +98.7M over its parent and +42.6M overall.
6. **Ordinary restart at a checkpoint.** Card 193 restart: 7,183 orbs vs 922
   (+77.1M route). Card 205 restart: +6.2M. Restart controls often beat targeted
   geometry.
7. **Late-scene search knobs, small but real:**
   - Movement 3 vs 4: +0.9M story; +8.3M on Extra 205.
   - Smaller beams on 205/187/191: +14.2M, +3.6M, +5.0M.
   - Realized-orb portfolio: +8.1M Extra, +7.6M story.
   - Fine 1/2/4/8 pulses: +13.2M story, but Extra fell to 3,297,953,740.
8. **Cash 4/movement 4 for full tails.** Running the whole Extra tail at cash 0 (V48)
   put all 12 tails below the record (best 3,069,116,400). Correcting to cash 4
   produced 3,194,638,140 (1517).

## 3. Negative and non-transferring results

| Experiment (tasks) | Measured outcome | Reason given |
| --- | --- | --- |
| VPS charge/reserve portfolio (787–801) | Charge H256 / early / H512 Extra: NMNB failures near 1.63–1.68B. Story late reserve failed. H512 story 5.453B | Charging and reserve variants are unsafe or lower; profile changes combined several causes |
| Terminal fire/form coordinate refinement (812–819) | Only +8,000 Extra; story unchanged | Short local edits do not close large gaps |
| Point-potential ranking, cash + K·PIV (835–842) | Extra K=256/1024/4096: 2.911/3.048/3.033B vs 3.088B at K=0. Story 4096 failed NMNB | Future-point equivalent is a heuristic; actual production is action-dependent |
| Stage 1 human-direction guide (821–828) | Best Stage 1 cash +2.4M, but 2,768 fewer carried orbs | An orb-only or fragment ranking cannot certify terminal profit |
| Card 6 focus/precharge/bottom latch (858–863, 972–979, 1078–1081) | Bottom Y 432: 134–209 orbs vs 1,321. Best entry trajectory: 1,603 | Matching the human median Y misses the human's orb-pickup gauge bootstrap |
| Card 6 prelude macros (1412–1428) | 128/256/512-input focus holds die before card 6. Best safe 13 continuation still below record | Long literal focus holds are unsafe |
| Card 116 farming (926–929, 1582–1587) | All fixed-600-reserve plans time out (best 2,907 orbs). Reserves 1200–2400 capture 2,024–3,423 orbs | Farming far from the boss leaves too little damage time; the plan overrides learned HP pacing |
| Card 13 farming (942–945) | 4,047–4,618 orbs, all timeouts | Lost capture bonus |
| Early card 10 capture (952) | 37/37 but 5,213,714,340 | Captures do not pay for downstream world changes |
| Stage 4A road from the human (954–959) | 184 vs 161 entry points; full routes 5.727/5.634B, below 5.821B | Point counts are selection evidence only |
| Stage 5 absolute-clock guide (966–970), source-clock boss poses (1163–1166) | Entry cash at or below the old entry | Lookup coverage does not imply reward quality |
| Final B parent 0 / human/collection 50–200 / H512 (1000–1006, 1082–1085) | Parent 0: 316→204 and 360→212 points. All others below 1014 at 151 | Disabling parent-chain focus cannot replace the human strategy |
| NPC-v5 static portfolio (1027–1042) | Best 5,748,286,080; one NMNB failure | Only the dynamic road/spell horizon made it pay |
| Late 171 tails on old worlds (895–902, 1069–1076) | All below the record, best 5,945,610,810 | Local 171 gains do not dominate earlier worlds; a restart is itself a strategy change |
| Stage 2 spells re-applied after changed worlds (931–936, 1061–1068, 1092–1093) | 36: 2,764→1,157 orbs. 47: 6,568→1,127. 21/25: 2,580/513→697/29 | Strong coupling to the incoming world and gauge |
| Final-survival phase geometry (1391–1402, 1844–1845, 1899–1900) | All 12 V38 geometries lose (6.423–6.484B). No-fire/shoot all-five plans: 6.534/6.529B | Survivals already farm well; uniform fire rules lose card 187 density |
| Wider beam / policy horizon (1319–1322, 1667–1672, 1809–1810, 1908–1909) | Extra beam 768/1536: 3.134–3.165B in 440–1,210 s. Story 6.394–6.498B. Verify 512: NMNB failures. Tail H256/512: 6.584/6.590B | Slower and worse under a fixed 60 ms proposal and 2000 ms admission |
| Card 193 high-orb worlds (1250, 1281–1288 → 1325–1336) | 11,752-orb parent lost 192.8M vs the 7,183-orb parent. 10,237-orb capture → 3.127B | Orb income does not survive the later route |
| Extra 203 lower band; 205 geometry (1269–1275, 1294–1306) | Lower 203 loses on 3 of 4 parents. Every targeted 205 geometry is below the ordinary restart | Ordinary controls win |
| Within-spell 205 restarts (1351–1362) | Best 3,176,324,740, below 3,179,193,860 | Queue/trajectory continuity has a cost |
| Extra 197/202 higher PIV (1573–1575) | 3.038–3.161B | Higher PIV (946,220) but 45 fewer points from pool exhaustion |
| Card 199 true no-fire (1875 → 1879) | 25,040 orbs locally; full 3,155,221,160 | 199 timeout omitted a 45-point drop batch; capture award lost (110.5M ledger) |
| Card 199 early capture (1853) | Captured with 2,171 updates left, 9,864 vs 20,385 orbs: −116.9M | Feedback fires while charging; damage ends the farm |
| Card 198 smaller beams (1935–1947) | +895/+376 orbs locally; full 3.280/3.254B | Later trajectories change |
| Charging controllers (1681–1688, 1704–1711, 1732–1737, 1822–1825) | 195 orb gains 172–601. Story 6 gains 124–185. One 6 timeout | Cold gauge never warms; recovery changes focus. Withdrawn from public code |
| Fast-stream speed cap 3→8 (1803–1804) | +51.1M within its family; both below record | Coupled proposal change, not a record |
| Cold card 6 true no-fire through full story (1955–1978) | 6,264,693,320 and 6,456,972,200 | 1962 world was +7.5M at Stage 5, but 151/155 close early in Final B |
| Unchanged 1962 world, cold 171 (2044–2069) | 37/37 at 6,510,239,280 / 6,499,591,740 | Extra captures and a better Stage 5 do not guarantee a better final world |
| Human first 2500/2700 actions (1990–2005) | More orbs at card 2 (1,679/2,072) but 2–4 fewer points. Stage 5 3.168/3.369B | Copying human inputs does not reproduce the human economy |
| Fixed-tail portfolio (no adaptive guidance), short scouts, move-then-dwell (2060–2063, 2081–2084, 2111–2112) | All below the records, e.g. 3,286,940,860 / 6,612,119,210 | Fixed futures mostly fail; short nomination drops useful long futures; dwelling is not farming |
| Focused 151 / delayed 155 (2119–2142) | Best 6,579,464,290; 3,402-orb 151 lost to the 2,729-orb one | Ranking reverses downstream |
| Card 47 focused / movement retained (2150–2161, 2176–2189) | Local 47 gains of 8,149–11,319 orbs vs 3,999. Full routes 6.436–6.554B; card 74 times out after | Changed worlds lose 74 and later Final B income |
| Card 74 continuous fire (2199–2204) | One capture (2201) in one world only; never continued | The reserve does not carry across incoming worlds |
| Solo Sakuya (1673–1716) | Extra 2,182,761,340; story 4,417,570,020 37/37 | Solo humans get 0 extreme-youkai graze orbs; Sakuya's +2000 gauge cap is below the 8000 extreme threshold |
| Learned branch value v1/v2 (removed November 15, 2026) | 24 new Extra 202 seeds: v1 and v2 9/24, baseline 14/24, generic no-model 23/24 | A better conditional classifier does not give a controller benefit |
| Bomb experiments (366–387, before the journals) | Bombs-allowed refinement adopted no bomb (scores unchanged at 2,620,568,090 / 4,311,260,760). Adaptive human bomb Extra 370 failed at 1,118,686,830; story 371/382/383 process failures | No profitable bomb edit was found; experiments paused |

## 4. Economics facts

**Point income dominates the gap to the same-shot human.** The human is Levea's
6,755,587,090 run (1 miss / 9 bombs, different seed).

| Measurement | Value |
| --- | --- |
| Gap at 5.821B (987/988) | Total 934,548,110: point cash 902,215,610, everything else 32,332,500 |
| Points at 5.821B | Native 1,557 pickups (1,383 extreme-human) = 2,068,835,200. Human 1,661 (1,633) = 2,971,050,810 |
| Same-pickup repricing at 5.821B | 299,149,120 (166,635,750 height + 132,513,370 doubling) |
| Same-pickup repricing at 5.922B (1043) | 229,880,420 (138,723,130 height + 91,157,290 doubling) |
| At 6.513B (1366) | 1,623 pickups = 2,488,535,730. Envelope **191,843,350** (158,373,630 height, 33,469,720 doubling). Still 295,624,550 short of 7B |
| Largest height losses at 6.513B | 4A 76,181,390; Final B 42,411,940; Stage 5 22,823,240 |
| Point cash vs the human at 6.513B | Human +482,515,080. Swapping in the human point cash gives only 6,995,047,180. Derived: the total gap was 243,054,990, so Native led non-point income by about 239.5M |
| Points per stage at 6.513B, Native/human | 55/61, 173/173, 321/327, 376/381, 260/269, 438/450 |
| Final PIV | Native 1,446,380 vs human 1,445,850, but Native reaches it later |
| Extra repricing (1,035 pickups) | 1,059,358,150 actual vs 1,095,591,520 perfect: **36,233,370**, 4.6% of the then 787,050,040 gap to 4B. 1,023 pickups already doubled. Human envelopes: Scarlet 1,606,140, Sakuya 799,740 |

**Stage balance for record 2113 vs the same-shot human.** At Stage 5 exit:
3,433,629,060 vs 3,825,571,550, a **−391,942,490** deficit. The Final B increment is
3,219,565,490 vs 2,930,015,540, a **+289,549,950** Native gain. Net −102,392,540.
Stage 1: 130,212,230/55 points/12,755 orbs vs 196,524,230/61/21,873 (−66.3M).
Orbs, Native vs human: Stage 1/2/3 12,755/33,391/36,052 vs 21,873/37,662/46,219.
4A: 50,628 vs 39,567. Final B: 75,471 vs 53,125.

**Per-card orbs, Native vs human (story).**
- 2: 1,065 vs 895. 6: 1,321 vs 4,069. 10: 2,933 vs 4,071 (human captures at −48;
  Native times out). 13: 3,765 vs 5,639.
- 47: **3,999 vs 11,608** (964 vs 3,882 extreme-youkai events; the human 47 follows
  a bomb 33 updates earlier).
- 116: **2,332 (1381 audit) / 1,952 (6.291B) vs 5,813**.
- 77: 212 vs 3,949 (6.291B). 74: incumbent 6,818 vs human 3,685.
- 167: 5,113 vs 378. 171: 10,620 → 17,504 vs 16,862.
- Final B already beats the human on 155/159/167 and on four of the last five
  survivals.

**Card 6 bootstrap.** In ages 0–40 the human collects 151 focused time orbs:
151 × 111 = +16,761 gauge, from −8000 to +10000 by age 41. Native collects 1 orb
at −10000 and is still at 679 at age 120. Before card 2 the human has 2,515 orbs vs
Native 810. 1,185 of the 1,705-orb shortfall comes before the boss.

**Last five survivals.**
- 1381: 2,306/2,483/3,615/5,962/2,893 orbs.
- 1847: 2,103/2,481/3,547/6,111/2,892.
- Orbs/bonuses/in-spell cash: 17,259/302,096,000/191,799,710 (gold) vs
  15,690/289,744,000/178,784,800 (V64).
- First four survival awards = 33M + 8,000 × orbs.

**Time-orb pool (Extra).** Native: 131,442 requests → 129,878 allocations, 1,559
busy-slot failures (143 with a free slot elsewhere). Human Scarlet: 107,051 →
105,665, with 1,273 busy-slot failures (349 with free capacity). Too small to
explain the gap.

**Point pool (Extra).** The item pool has 2,096 slots and SpawnItem returns a
sentinel when full. At card 202, 45 point requests failed at 2096/2096 occupancy,
leaving 990 vs 1,035 points. Card 199 timeout is a separate mechanism: it omits a
45-point drop batch (DropItems runs only on the HP-death path).

**Extra point requests/allocations/collections.** Native 1,041/1,040/1,035. Human
1,072/1,071/1,071. Later road requests 286 vs 315.

**Extra card orbs and grazes (Native).**
- Orbs: 192: 4,821; 193: 11,747; 198: 18,528; 199: 21,319; 204: 11,126; 205:
  about 6,355.
- Ordinary extreme-youkai grazes, Native vs human: 193 3,868 vs 2,363; 198 6,075 vs
  3,727.
- Record vs human Scarlet orbs: 192 2,173/2,837, 197 541/590, 202 1,128/699.
- The old card 193 had 922 vs the human's 7,014.

**Pricing rules (source).**
- Time orb: 10 × floor(points/2), minimum 100, cap 10,000 above 2,000 points. At
  1,033 vs 965 points this is 5,160 vs 4,820.
- Each orb adds 8,000 to spell-bonus progress and ±111 gauge depending on focus.
- Extra clear term: 2,000 displayed per settled orb.
- Clear bonus has no direct PIV multiplier.
- Points double at extreme-human gauge.
- Graze at extreme-youkai requests 3 orbs for teams (laser graze only 1); 0 for
  solo humans.
- Capture bonus is credited 130 updates after EndSpell.

## 5. Methodological lessons

1. **Checkpoint unit = executed owner inputs.** Audit `start_update` counts source
   calculations. V104 found 62528/82449 were 3 inputs before the real 58/77 entries.
   Story 171 entry is owner 146244 vs audit 146239. Natural rows lag the producer by
   0/19/38/57/76/95 per stage. A natural-row lookup misread the 171 entry gauge as
   +6915 (actual −9112). A human 116 window based on audit counts was retracted.
2. **Local cash/PIV/orbs are not additive and do not order worlds.**
   - 1186 had lower cash and PIV than 1185 but won by 78.2M.
   - The 18,522-orb 171 world (1842) lost to the 17,504-orb world (1838).
   - The 3,402-orb 151 world lost to the 2,729-orb world by 40.1M.
   - 1902 had more cash plus pending award than 1901 but finished 64.3M lower.
   - On card 203, the higher-orb H128 world lost to the H256 world.
3. **Recompute every downstream component.** Strategies transfer; suffixes do not.
   The 171 band ranking reversed between the 6.291B and 6.440B worlds.
4. **A capture boundary is before settlement.** The award arrives 130 updates later,
   beyond what an H128 plan sees. Carry the pending award; do not add it. 1875's
   apparent +19.56M lead was an accounting artifact.
5. **Check realized inputs, not controller labels.** V116/V120 "no fire" produced
   byte-identical tapes because cash 4 fire/form comparisons overrode the intent.
   Only V122 (cash 0 in the phase) realized no-fire (2 Fire inputs of 4,200).
6. **Equal labels are not equal policies.** Legacy parity needed the whole
   proposal + recovery graph, including the recovery capture-form prior (V44–V49).
   A stale `phase_card=29` report literal misidentified card 17.
7. **A restart is a strategy change.** The same-parameter restart 1072 differs from
   1071. Always include an ordinary-restart control; it often wins (193, 202, 205).
8. **Deduplicate inactive parameters.** 1225/1226 and 1838/1839 are identical tapes
   (reserve inactive), as are 1865/1866 and 1414/1415. The last 4-owner VPS batch had
   only 2 unique tapes.
9. **Name confounds.** The V48 cash 0 tail, V46 pacing that also changed unrelated
   attacks, and phase geometry coupled with fire/cash changes.
10. **Diagnose per world.** One 45-point loss was pool exhaustion (1574, 1916);
    another was a timeout drop omission (1879). Others start after 202. Raw HP is
    not damage-to-zero (use callback-adjusted capture HP). A post-deadline lastHP of
    2 is not a near-capture.
11. **Reserves are neither monotone nor portable.** On 193, 900 beat both 600 and 1200.
    On 116, 600 was too short. The 77 reserve 600 timed out in the 1956 world. The
    74 reserve captured only in 2188.
12. **Diversity helps but was not proven to beat greedy.** The 1517 record came from
    the lower-orb 192 parent. In 1272, the highest-cash parent also won.
13. **Human references are leads, not oracles.** They have different seeds and
    bombs/misses. Copying human inputs (1990/1991) raised orbs but cut points.
14. **Execution parity is not retail proof.** The Wine run had a 64-byte-header
    decoder bug that misreported score and bombs. RPY USER footers differ in
    timestamp bytes.
15. **Orchestration failures were frequent.** Missing fail-fast dispatched a broken
    batch (804–811). `Array.map` passed an index argument twice (1237, V196).
    Ungated scp/service starts ran before checks. Stale caps (1289). A clang-format
    child exceeded 4 GiB (1158). Host-memory aborts (803, 1293, 1529) and a host
    reboot (1848/1849) occurred. An SSH host-key change left 19 remote receipts
    unresolved.

## 6. Cost and throughput

Hardware. The VPS is an AMD EPYC 9654 with 192 logical CPUs, capped at 32 owners
on 32 cores (one core each). The local host had 16 CPUs/62 GiB before a reboot and
32 threads (Xeon 2.80 GHz)/270 GB after. The local service allows 2 owners on 2
cores. Concurrent wall times are not isolated benchmarks.

| Work | Typical wall |
| --- | --- |
| Literal full replay / natural row check | Story 18–24 s / about 8 s. Extra fresh 19.6 s / natural 3.1 s. Read-only maps 4–16 s |
| One spell scene | 20–70 s (VPS and local) |
| Card 205 → end (Extra) | 22–76 s ordinary; 235 s Native-32 portfolio; 330 s fine pulses; beam 768/1536: 440–1,210 s |
| Story 171/175 → end | 41–156 s ordinary; 404 s portfolio; 492 s fine pulses. Card 191 alone: about 148 s |
| Story Final B 151 → end | About 880–970 s summed (151→171 alone about 430–460 s) |
| Stage 2 card 29 → Stage 5 boundary, local | 1,124–1,169 s. Stage 5 NPC segment alone: 440–530 s |
| Complete story from Stage 1, local 2-core | About 2,600 s (derived: 5,161 s summed for two complete chains, 1955–1978) |
| Complete story on the VPS (one owner) | 1,700–1,830 s at H256/H128 (794/799); H512 3,005 s; full NPC dynamic 2,028 s (1047) |
| Complete Extra, VPS | 540–925 s (786: 909 s, 68,901 inputs, 921,792 branch updates); phase compositions 1,030–1,090 s |
| Complete Extra from card 195/197/199/202, local | About 810 / 660 / 380–400 / 245 s |
| Verification | Fresh + natural + map: tens of seconds. Wine retail playback 1,940.5 s (26.6 replay fps). Normal CTest 874–1,200 s (62/62 in 933.58 s) |
| Builds | 2–53 s; clean phase-adapter builds 100–200 s |

Batch figures:
- V38: 30 owners summed 2,807 s.
- Recent local closed batches: 3,800–6,100 s summed per 18–74 allocations.
- Guidance dominates cost. For 1903, branch Native time was 106.3 s vs guidance
  626.9 s. For 1917/1918, Native 5.1 s vs guidance 23.5–42.5 s.

There was never an equal-budget local-vs-VPS test, so the journals claim no
hardware speedup.


## 7. Open hypotheses never tested

1. Archive/diversity scheduler (Go-Explore/MAP-Elites style) against the batch
   dispatcher at equal CPU and wall budget, plus worker scaling at 1/4/8/16/32. An
   adaptive racing (Hyperband-like) allocator was never built.
2. Persistent per-entry workers that reexecute a prefix once and fork candidates
   copy-on-write. Measure prefix cost first. Parallel economic comparisons
   (fire/form then movement) are also unimplemented.
3. Continue 2201's captured card 74 world (input 82780) through 77, Stage 5 and
   Final B.
4. Card 13 (entry 13817): keep a damaged prefix, pause fire for a bounded interval,
   then capture (V240; HP traces already exist).
5. Card 6 chain economy: sub-16 familiar-hit income → DetachEnemyChain settlement
   (26 orbs per child at chain ≥ 8) → focused pickup into the spell. Also a
   spawn/collect probe separating carried field items from new production, and the
   prepared V182 reconstruction of the 1155/1168 gauge-9086 opening.
6. V179: stationary danger proxy 3→1 during farming phases (unbuilt).
7. Human card 47 fire-timing pattern (per-300-update fire 75/300/300/300/68/3/0/27)
   as an explicit timing proposal.
8. Tune entry form, charge and damage timing jointly with geometry (1853 lesson).
9. Other shots. Solo Youmu (±3000 gauge thresholds, 10-orb graze; both all-shot
   references are solo Youmu) needs a gauge controller that does not exist. Ghost
   Team (−3000) is untried. Solo Sakuya's chain reward (40/child) and hit threshold
   27 are untested.
10. The four Lunatic card 58 partials (1777–1780) were never continued.
11. Wine validation of any Native record after 3,179,193,860, and the story retail
    gate above 7,053,332,460.
