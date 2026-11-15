#pragma once
#include "th08/shadow_rollout.hpp"
#include <memory>

namespace th08::shadow {
struct RecordedOptions {
    unsigned horizon = 128, beam = 192, refinements = 4;
    // Zero verifies the full recording horizon. A shorter value requests a
    // separate complete Native rollout of that length; recording proves none.
    unsigned verification_horizon = 0;
    int diversity_cell = 16;
    bool region_goals = true, advance_dialogue = false;
};
struct RecordedWork {
    std::uint64_t forecasts = 0, attempts = 0, completed = 0, charged = 0;
    std::uint64_t native_ns = 0, file_io_ns = 0, capture_ns = 0, search_ns = 0;
    std::uint64_t searches = 0, paths = 0, forecast_timeouts = 0, search_timeouts = 0;
    std::uint64_t forecast_deaths = 0;
};
struct RecordedResult {
    Prediction selected;
    // Includes every failed and successful independent verification. The
    // selected prediction is also present here; callers account for it once.
    std::vector<Prediction> checks;
    RecordedWork work;
    bool native_fault = false;
    double wall_ms = 0;
};
// Records conditional Native futures and searches their geometry for proposals.
// Only a separate complete Native execution can select a result. The caller is
// single-threaded; the actual world, RNG and forced lead remain unchanged.
// Geometry storage and proposal tie RNG belong to this planner, which may be
// reused across actual receipts. It retains no mutable Native world checkpoint.
class RecordedPlanner {
  public:
    explicit RecordedPlanner(RecordedOptions options = {});
    ~RecordedPlanner();
    RecordedPlanner(const RecordedPlanner &) = delete;
    RecordedPlanner &operator=(const RecordedPlanner &) = delete;
    RecordedResult solve(headless::Session &, const Packet &origin,
                         const std::vector<std::uint16_t> &lead,
                         const std::vector<std::uint16_t> &nominal, Boundary, unsigned budget_ms);

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
// Execute literal proposal inputs after their immutable forced lead.
// Dialogue inputs after the lead use the current branch's GUI state. This does
// no proposal search, and still requires the entire requested Native horizon.
Prediction predict_recorded_tape(headless::Session &, const Packet &origin,
                                 const std::vector<std::uint16_t> &inputs, std::size_t forced,
                                 Boundary, unsigned budget_ms, bool advance_dialogue = false);
} // namespace th08::shadow
