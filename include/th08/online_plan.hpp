#pragma once
#include "th08/online_movement.hpp"

namespace th08::online::detail {
// At decision u the already latched input owns movement one. Of the N inputs
// committed at u..u+N-1, only the first N-1 have moved the player at u+N; the
// last is then latched. This is a position prediction, not a future-world step.
inline Vector prefix_arrival(const Movement &movement, Vector position, std::uint16_t latched,
                             const std::vector<std::uint16_t> &committed) {
    for (std::size_t i = 0; i < committed.size(); ++i)
        apply_movement(movement, position.x, position.y,
                       movement_delta(movement, i == 0 ? latched : committed[i - 1]));
    return position;
}
} // namespace th08::online::detail
