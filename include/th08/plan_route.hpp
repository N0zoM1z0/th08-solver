#pragma once

#include "modern/headless/session.hpp"
#include "th08/plan_contracts.hpp"
#include <cstdint>
#include <optional>

namespace th08::plan {

// This controller operates the original dialogue selection. Its resulting input
// belongs to the rollout tape and is committed verbatim; fresh replay never
// substitutes commands. The source interpreter consumes the preceding input.
inline std::optional<std::uint16_t>
final_route_input(FinalRoute route, const headless::State &state, std::uint64_t frame) {
    if (route == FinalRoute::Natural || !state.final_route_selection)
        return std::nullopt;
    const auto direction = route == FinalRoute::A ? 16 : 32;
    return std::uint16_t(4 | (frame % 2 ? direction : 1));
}

inline bool story_terminal(int stage, bool stage_complete, int until_stage) {
    return stage > until_stage || (stage >= 6 && stage <= 8 && stage_complete);
}

} // namespace th08::plan
