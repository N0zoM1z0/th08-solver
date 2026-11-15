#pragma once
#include "th08/shadow_rollout.hpp"
namespace th08::online {
class Policy;
}
namespace th08::shadow {
struct FutureProposals {
    BranchStatus status = BranchStatus::Incomplete;
    std::vector<std::vector<std::uint16_t>> inputs;
    std::uint64_t attempts = 0, completed = 0, charged_updates = 0;
    std::uint64_t native_ns = 0, file_io_ns = 0;
    double wall_ms = 0;
};
struct MotionProposal {
    std::vector<std::uint16_t> inputs;
    // Zero holds the movement indefinitely; otherwise stop after this many
    // submitted movement inputs, preserving shoot/focus in the held tail.
    unsigned movement_updates = 0;
};
// Fixed generic recovery family: both focus states, nine held directions and
// 16/32/64-update pulses for the eight nonstationary directions. The actual lead
// remains immutable. These are proposals, without a safety verdict.
std::vector<MotionProposal> motion_recovery_proposals(const std::vector<std::uint16_t> &lead);
// Propose at the state where new inputs can actually take effect. Only the
// isolated child advances the forced lead and the inherited Policy history/tie
// RNG. These proposals certify nothing; verify each complete lead+candidate
// from the unchanged actual owner with predict().
FutureProposals propose_after_lead(headless::Session &, online::Policy &, const Packet &origin,
                                   const std::vector<std::uint16_t> &lead, Boundary,
                                   unsigned timeout_ms, bool advance_dialogue = false);
// The fixed candidate is followed by branch-owned Policy replanning in batches
// of at most eight submitted inputs. The complete horizon/terminal is required.
// An optional branch-owned goal policy is observed on every actual future
// packet; it owns fire intent while the movement policy supplies directions.
// Neither policy nor the actual owner is advanced outside the isolated child.
Prediction predict_with_policy(headless::Session &, online::Policy &, const Packet &origin,
                               const std::vector<std::uint16_t> &prefix, std::size_t horizon,
                               Boundary, unsigned timeout_ms, bool advance_dialogue = false,
                               online::Policy *goal_policy = nullptr);
// Keep the prefix immutable and hold its final action through a complete native
// horizon. Dialogue advancement, when requested, still follows owned GUI state.
Prediction predict_with_hold(headless::Session &, const Packet &origin,
                             const std::vector<std::uint16_t> &prefix, std::size_t horizon,
                             Boundary, unsigned timeout_ms, bool advance_dialogue = false);
} // namespace th08::shadow
