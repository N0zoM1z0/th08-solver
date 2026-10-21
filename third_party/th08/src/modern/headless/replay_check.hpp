#pragma once
// Headless retail-equivalence check: records the upstream 25-field replay
// state rows (scripts/replay-external-schema.json, version 4) at the same
// boundary as src/modern/linux/replay_trace.cpp, but streams them to a file
// instead of comparing with a captured retail trace. A SHA-256 of the schema
// plus rows is directly comparable with config/replay-fixtures.json.
#include <cstdint>
#include <cstdio>

namespace th08::headless {
struct ReplayCheckSummary {
    std::uint32_t rows = 0;
    // Cumulative observation counts across all recorded calculation rows.
    std::uint32_t death_rows = 0, bomb_input_rows = 0;
    bool finished = false, natural = false, write_failed = false;
    int stage_count = 0;
    int stages[9] = {};
    std::uint32_t stage_frames[9] = {}, stage_scores[9] = {}, rng_origins[9] = {};
};
void replay_check_enable(std::FILE *rows);
const ReplayCheckSummary &replay_check_summary();
// Observes the gameplayFrameCounter increment after all GameManager BREAK gates.
// It neither resets across stages nor depends on replay row recording.
void observe_gameplay_calculation();
std::uint64_t replay_calculation_updates();
} // namespace th08::headless
