#pragma once
#include "th08/shadow_protocol.hpp"
#include <vector>

namespace th08::headless {
class Session;
}
namespace th08::shadow {
enum class BranchStatus : std::uint32_t { Incomplete, Horizon, Terminal, Death, Fault, Timeout };
struct Boundary {
    bool story = false;
    int spell = -1;
};
struct Prediction {
    Packet origin;
    std::vector<Packet> frames;
    BranchStatus status = BranchStatus::Incomplete;
    std::size_t requested = 0;
    std::uint64_t attempts = 0, completed = 0, charged_updates = 0;
    std::uint64_t native_ns = 0, file_io_ns = 0;
    double wall_ms = 0;
    bool certifies() const {
        return !frames.empty() && completed == frames.size() && attempts == completed &&
               ((status == BranchStatus::Horizon && frames.size() == requested) ||
                (status == BranchStatus::Terminal && frames.size() <= requested));
    }
};
// The caller and session are single-threaded. The child owns all mutable world
// state, including true RNG and input-dependent consumers. Death/fault/timeout
// never certify a prefix. The parent's actual world is left unchanged.
Prediction predict(headless::Session &session, const Packet &origin,
                   const std::vector<std::uint16_t> &inputs, Boundary boundary,
                   unsigned timeout_ms = 1000);
} // namespace th08::shadow
