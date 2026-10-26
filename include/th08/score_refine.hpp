#pragma once
#include "th08/shadow_rollout.hpp"

namespace th08::shadow {
enum class ScoreEdit { Focus, Human, Fire, ReleaseFire };
enum class ScoreEditFamily { Form, Fire };
const char *score_edit_name(ScoreEdit);
struct ScoreRefineOptions {
    ScoreEditFamily family = ScoreEditFamily::Form;
    unsigned trials = 96, window = 240, interval = 1800;
    unsigned timeout_ms = 30000, wall_seconds = 1200;
};
struct ScoreRefineTrial {
    std::size_t first = 0, window = 0;
    ScoreEdit edit = ScoreEdit::Focus;
    TapeEvaluation evaluation;
    bool accepted = false;
};
struct ScoreRefineResult {
    TapeEvaluation initial, selected;
    std::vector<ScoreRefineTrial> trials;
    std::uint64_t owner_updates = 0, owner_native_ns = 0, owner_file_io_ns = 0;
    unsigned unchanged = 0;
    bool complete = false, search_budget_exhausted = false;
    double wall_seconds = 0;
};
// Entry-session coordinate refinement of our own complete Native input tape.
// The caller enables the source audit before constructing the session. Only
// unexecuted inputs change; each accepted suffix must finish NMNB, capture the
// identical source spell set, keep the final route and improve actual cash.
// Trials are sequential forks. Budget exhaustion keeps executing the certified
// incumbent, rather than treating a partial candidate as progress.
ScoreRefineResult refine_score_tape(headless::Session &, std::vector<std::uint16_t> &inputs,
                                    bool story, const ScoreRefineOptions &);
} // namespace th08::shadow
