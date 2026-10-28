# Native evidence and replays

Current score-attack replays are grouped by **shot and route**. Frozen component,
planner and runtime evidence lives in [archive/](archive/README.md), including its
historical replay/action pairs and failed prefixes. New experiments, raw action
tapes, run JSON and videos stay under ignored `reports/local/`.

| Shot / route | Best published Native NMNB score | Captures |
| --- | ---: | ---: |
| Scarlet Extra | [3,255,579,400](replays/scarlet/extra/extra-scarlet-native-nmnb-3255579400-seed9098.rpy) |14/14 |
| Scarlet Lunatic 1→2→3→4A→5→Final B | [6,513,456,180](replays/scarlet/lunatic/story-scarlet-native-nmnb-4a-finalb-6513456180-seed32796.rpy) |36/37 |
| Magic Extra | [3,013,913,970](replays/magic/extra/extra-native-nmnb-3013913970-seed46997.rpy) |14/14 |
| Magic Lunatic 1→2→3→4A→5→Final B | [5,550,599,270](replays/magic/lunatic/story-native-nmnb-4a-finalb-5550599270-seed1805.rpy) |36/37 |
| Magic Lunatic, all captures | [5,107,553,670](replays/magic/lunatic/story-native-nmnb-4a-finalb-5107553670-seed1805.rpy) |37/37 |

All five have original-entry fresh Native terminal agreement and natural replay
score, zero-miss/bomb, capture-outcome and name checks. These are Native execution
checks. New Extra exceeds the all-shot3,178,460,830 reference by77,118,570 in
Native. The earlier3,179,193,860 replay passes original Wine validation at the
exact score, NMNB14/14 (1343/1367), with its bytes preserved. The new Extra and
the story remain Native-only; the story reference remains higher.

| Location | Contents and provenance |
| --- | --- |
| [replays/scarlet/extra/](replays/scarlet/extra/) · [lunatic/](replays/scarlet/lunatic/) | Selected verified Scarlet progress, original retail seeds9098/32796 |
| [replays/magic/extra/](replays/magic/extra/) · [lunatic/](replays/magic/lunatic/) | Selected Magic progress, preserving each historical seed and exact bytes |
| [replays/progress.tsv](replays/progress.tsv) | The38 selected progress replays: score, shot, route, seed, actual misses/bombs, capture outcomes, producer task, source, receipt and replay/action SHA-256 |
| [replays/online/](replays/online/) | Selected original online stories and split-capture recording replays; separate from Native score attack |
| [archive/README.md](archive/README.md) | Historical producer/input/scope index; original machine reports and tapes are unchanged |
| [archive/relocations.tsv](archive/relocations.tsv) | Old→new paths and unchanged SHA-256 for all272 relocated historical payloads |

[Status](../../docs/STATUS.md) owns current progress, costs, failed trials and next
work. [Validation](../../docs/VALIDATION.md#scarlet-team-verified-native-baselines)
owns full-entry lineage and checks; earlier Magic provenance remains in its
[selected progress section](../../docs/VALIDATION.md#selected-native-progress-replays).
New scoring replays use the exact eight-byte name `ysopwnab`; historical bytes
and names are preserved. The owner authorized these selected progress uploads
and this organization on October27. Archived JSON may record its original paths;
use the relocation table without rewriting the frozen evidence.
