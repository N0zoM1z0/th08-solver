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
    std::uint64_t guidance_ns = 0, guidance_requests = 0;
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
// A complete remaining input tape is evaluated without storing per-update
// projections. The enabled owner score audit is inherited, then observed in
// the isolated child. Its exact visited-stage spell set is part of the goal.
// This is an offline terminal-score oracle, with no real-time publication use.
struct TapeEvaluation {
    BranchStatus status = BranchStatus::Incomplete;
    Projection terminal;
    std::array<std::uint64_t, 4> expected_spells{};
    unsigned expected_captures = 0, captured = 0;
    bool all_captures = false;
    std::uint64_t requested = 0, attempts = 0, completed = 0, charged_updates = 0;
    std::uint64_t native_ns = 0, file_io_ns = 0;
    double wall_ms = 0;
    bool certifies() const {
        return status == BranchStatus::Terminal && all_captures && completed > 0 &&
               attempts == completed && !terminal[Field::Deaths] && !terminal[Field::BombsUsed] &&
               terminal[Field::PlayerState] != 2;
    }
    std::uint64_t score_displayed() const {
        return std::uint64_t(terminal[Field::Score]) * 10;
    }
};
// first is the number of inputs already executed by this owner. No reconstruction
// or checkpoint reset occurs: all carried state and true RNG come from session.
TapeEvaluation evaluate_tape(headless::Session &, const Packet &origin,
                             const std::vector<std::uint16_t> &inputs, std::size_t first, Boundary,
                             unsigned timeout_ms = 30000);
// Guidance lives in the forked branch. It observes the current owned rollout
// and proposes additional inputs after the fixed prefix; it must not step or
// mutate the native world. Native execution still decides every outcome.
class RolloutGuidance {
  public:
    virtual ~RolloutGuidance() = default;
    virtual void observe(const Packet &) = 0;
    virtual std::uint16_t next_input() = 0;
};
Prediction predict_guided(headless::Session &, const Packet &origin,
                          const std::vector<std::uint16_t> &prefix, std::size_t horizon,
                          RolloutGuidance &, Boundary, unsigned timeout_ms = 1000);
} // namespace th08::shadow
