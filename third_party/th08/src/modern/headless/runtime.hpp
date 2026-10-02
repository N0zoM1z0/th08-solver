#pragma once
#include <chrono>
#include <stdint.h>

// One original game instance per process. These are explicit input/clock ports,
// not candidate state: a planner must never memcpy pointer-rich game managers.
extern uint16_t th08_headless_input;
extern uint64_t th08_headless_frame;

namespace th08::headless {
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
