#pragma once

#include <cerrno>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <sys/wait.h>

namespace th08::plan {

enum class RolloutStatus {
    Running,
    Horizon,
    Terminal,
    Death,
    Capacity,
    StepError,
    Incomplete,
    ChildError,
};

struct RolloutSummary {
    int frames = 0;
    int death = 0; // One-based native update; zero means no observed death.
    RolloutStatus status = RolloutStatus::Running;
};

inline bool verified_window(const RolloutSummary &r, int horizon) {
    if (r.death || r.frames <= 0 || r.frames > horizon)
        return false;
    return (r.status == RolloutStatus::Horizon && r.frames == horizon) ||
           r.status == RolloutStatus::Terminal;
}

inline bool failed_rollout(const RolloutSummary &r, int horizon) {
    if (verified_window(r, horizon))
        return false;
    return r.status != RolloutStatus::Death || r.death != r.frames || r.frames <= 0 ||
           r.frames > horizon;
}

inline bool fits_tape(std::size_t bullets, std::size_t lasers, std::size_t regions,
                      std::size_t bullet_capacity, std::size_t laser_capacity,
                      std::size_t region_capacity) {
    return bullets <= bullet_capacity && lasers <= laser_capacity && regions <= region_capacity;
}

// Recording mode executes one GameManager calculation, or a controller-only
// stage handoff. Playback may batch calculations and is outside this contract.
inline bool supported_recording_step(std::uint64_t calculations, bool stage_changed) {
    return calculations == 1 || (calculations == 0 && stage_changed);
}

struct ChildResult {
    bool waited = false;
    int status = 0;
    int error = 0;

    bool exited(int expected = 0) const {
        return waited && WIFEXITED(status) && WEXITSTATUS(status) == expected;
    }
};

// Never interpret EINTR, an unrelated child, a signal, or a failed fork as a
// successful rollout. Only this exact child can publish its tape.
inline ChildResult wait_for_child(pid_t pid) {
    if (pid <= 0)
        return {false, 0, EINVAL};
    int status = 0;
    pid_t result;
    do {
        result = waitpid(pid, &status, 0);
    } while (result < 0 && errno == EINTR);
    return result == pid ? ChildResult{true, status, 0} : ChildResult{false, 0, errno};
}

template <class Integer> Integer integer(std::string_view value, std::string_view option) {
    Integer result{};
    const auto parsed = std::from_chars(value.data(), value.data() + value.size(), result);
    if (value.empty() || parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size())
        throw std::invalid_argument(std::string(option) + " requires an integer");
    return result;
}

inline std::uint16_t nmnb_input_value(unsigned input) {
    if (input > 65535 || (input & ~0x10f5u))
        throw std::invalid_argument("verification requires supported NMNB input bits");
    return std::uint16_t(input);
}

// Offline score experiments may admit X explicitly. Menu, pause and other
// unsupported bits remain invalid; existing callers retain the NMNB contract.
inline std::uint16_t scoring_input_value(unsigned input, bool allow_bombs) {
    if (!allow_bombs)
        return nmnb_input_value(input);
    if (input > 65535 || (input & ~0x10f7u))
        throw std::invalid_argument("verification requires supported scoring input bits");
    return std::uint16_t(input);
}

inline std::uint16_t nmnb_input(std::string_view value) {
    return nmnb_input_value(integer<unsigned>(value, "action"));
}

enum class FinalRoute { Natural = 0, A = 1, B = 2 };

} // namespace th08::plan
