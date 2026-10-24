#pragma once
#include "th08/shadow_protocol.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace th08::shadow {
struct Comparison {
    const char *failure = nullptr;
    unsigned numeric_words = 0;
    unsigned inactive_words = 0;
    float player_error = 0;
};
// Live play is judged by behavior, not bitwise equality of every numeric digest.
// Digest drift is telemetry. Inputs, RNG, clocks, lifecycle, actors, feedback and
// outcomes remain strict. Only copied observations are compared; neither world,
// collision predicate nor RNG is modified. Exact mode remains a diagnostic gate.
inline Comparison compare_live(const Packet &owner, const Packet &native, bool exact) {
    Comparison result;
    result.failure = difference(owner, native);
    if (exact || !result.failure)
        return result;
    if (owner.kind != Kind::State || native.kind != Kind::State || owner.status || native.status ||
        owner.sequence != native.sequence || owner.epoch != native.epoch ||
        owner.input != native.input)
        return result;
    result.failure = nullptr;
    for (std::size_t i = 0; i < owner.state.values.size(); ++i) {
        if (owner.state.values[i] == native.state.values[i])
            continue;
        const auto field = Field(i);
        if ((field == Field::Spell && !owner.state[Field::SpellActive] &&
             !native.state[Field::SpellActive]) ||
            (field == Field::FinalRoute && !owner.state[Field::RouteSelection] &&
             !native.state[Field::RouteSelection])) {
            ++result.inactive_words;
            continue;
        }
        if (field == Field::BulletLow || field == Field::BulletHigh || field == Field::LaserLow ||
            field == Field::LaserHigh) {
            ++result.numeric_words;
            continue;
        }
        if (field == Field::X || field == Field::Y) {
            float actual, predicted;
            std::memcpy(&actual, &owner.state.values[i], sizeof(actual));
            std::memcpy(&predicted, &native.state.values[i], sizeof(predicted));
            const auto error = std::abs(actual - predicted);
            result.player_error = std::max(result.player_error, error);
            if (std::isfinite(actual) && std::isfinite(predicted) && error <= 1.f / 64.f) {
                ++result.numeric_words;
                continue;
            }
        }
        result.failure = field_names[i];
        break;
    }
    return result;
}
} // namespace th08::shadow
