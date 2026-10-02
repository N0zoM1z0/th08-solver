#pragma once
#include "session.hpp"
#include <chrono>
#include <stdint.h>

namespace th08 {
struct Float3;
}

// One original game instance per process. These are explicit input/clock ports,
// not candidate state: a planner must never memcpy pointer-rich game managers.
extern uint16_t th08_headless_input;
extern uint64_t th08_headless_frame;

namespace th08::headless {
void begin_update_observation();
CollisionEvent current_collision();
const std::vector<LaserHitboxView> &current_laser_hitboxes();
void prepare_observation_storage();
void record_laser_hitbox(const Float3 &center, const Float3 &size, const Float3 &origin,
                         float angle, bool graze_enabled);
void record_collision(CollisionKind, const Float3 &player_min, const Float3 &player_max,
                      const Float3 &hazard_min, const Float3 &hazard_max,
                      const Float3 *owner_position = nullptr);
// Single-threaded diagnostics only. Timing never feeds game clocks or RNG.
extern uint64_t file_io_ns;
struct FileIoTimer {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    ~FileIoTimer() {
        file_io_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(
                          std::chrono::steady_clock::now() - start)
                          .count();
    }
};
} // namespace th08::headless
