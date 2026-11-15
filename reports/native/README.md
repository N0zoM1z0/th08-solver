# Native replays

Selected replays grouped by shot and route. All are complete No Miss / No Bomb runs
exported by the original replay writer with player name `ysopwnab`, verified by
fresh Native re-execution and natural playback ([Validation](../../docs/VALIDATION.md#verification-gate)).
New experiments, tapes and run JSON stay in ignored `reports/local/`.

| Shot / route | Best Native NMNB score | Captures |
| --- | ---: | ---: |
| Scarlet Extra | [3,308,826,050](replays/scarlet/extra/extra-scarlet-native-nmnb-3308826050-seed9098.rpy) | 14/14 |
| Scarlet Lunatic 1→2→3→4A→5→Final B | [6,653,194,550](replays/scarlet/lunatic/story-scarlet-native-nmnb-4a-finalb-6653194550-seed32796.rpy) | 36/37 |
| Magic Extra | [3,013,913,970](replays/magic/extra/extra-native-nmnb-3013913970-seed46997.rpy) | 14/14 |
| Magic Lunatic 1→2→3→4A→5→Final B | [5,550,599,270](replays/magic/lunatic/story-native-nmnb-4a-finalb-5550599270-seed1805.rpy) | 36/37 |
| Magic Lunatic, all captures | [5,107,553,670](replays/magic/lunatic/story-native-nmnb-4a-finalb-5107553670-seed1805.rpy) | 37/37 |

The Scarlet Extra [3,179,193,860](replays/scarlet/extra/extra-scarlet-native-nmnb-3179193860-seed9098.rpy)
replay also passed complete retail playback in Wine (October 27, 2026). Newer
replays are Native-verified only.

| Location | Contents |
| --- | --- |
| [replays/scarlet/](replays/scarlet/) | Scarlet Team progress, original retail seeds 9098 (Extra) and 32796 (story) |
| [replays/magic/](replays/magic/) | Magic Team progress; seeds 46997/1805 and earlier fixed-seed runs |
| [replays/progress.tsv](replays/progress.tsv) | The 51 selected progress replays with score, seed, misses, bombs, captures, producer task, source revision and replay/action SHA-256 |
| [replays/online/](replays/online/) | Earlier live Wine online runs (Magic Team), kept as regression fixtures and history |
