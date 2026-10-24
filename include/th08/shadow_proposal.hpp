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
// Propose at the state where new inputs can actually take effect. Only the
// isolated child advances the forced lead and the inherited Policy history/tie
// RNG. These proposals certify nothing; verify each complete lead+candidate
// from the unchanged actual owner with predict().
FutureProposals propose_after_lead(headless::Session &, online::Policy &, const Packet &origin,
                                   const std::vector<std::uint16_t> &lead, Boundary,
                                   unsigned timeout_ms);
// The fixed candidate is followed by branch-owned Policy replanning in batches
// of at most eight submitted inputs. The complete horizon/terminal is required.
Prediction predict_with_policy(headless::Session &, online::Policy &, const Packet &origin,
                               const std::vector<std::uint16_t> &prefix, std::size_t horizon,
                               Boundary, unsigned timeout_ms);
} // namespace th08::shadow
