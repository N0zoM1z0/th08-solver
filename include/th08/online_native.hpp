#pragma once
#include "th08/online.hpp"

namespace th08::headless {
class Session;
}
namespace th08::online {
// Read the current native world without stepping it. Snapshot storage belongs
// to the caller; no pointer into a reused Session view survives this call.
void capture_memory(headless::Session &session, std::uint64_t update, Observation &observation,
                    bool include_actors = false, unsigned queued_horizon = 0,
                    unsigned queued_laser_horizon = 0, bool include_turn_hints = false,
                    bool include_motion_alternatives = false, unsigned child_horizon = 0,
                    unsigned lifetime_horizon = 0);
// Shadow guidance uses the ordinary actor view and requests scoring inputs only
// when its immutable Policy configuration needs them. Both reads are current-only.
void capture_policy_memory(headless::Session &, std::uint64_t update, Observation &,
                           const Policy &);
} // namespace th08::online
