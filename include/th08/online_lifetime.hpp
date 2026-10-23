#pragma once
#include <array>
#include <cstdint>

namespace th08::online::detail {
inline constexpr std::uint32_t kLifetimeWaitKind = 0x20000u;
inline constexpr std::uint32_t kLifetimeDespawnKind = 0x40000u;
struct LifetimeRecord {
    std::uint32_t kind = 0;
    bool allow_while_active = false;
};
struct CollisionLifetime {
    unsigned collision_until = 0;
    bool unsupported = false;
};
// Only the first enabled DESPAWN reached through masked records is covered.
// Native tests the active gate before skipping a masked record. WAIT clears
// after AdvanceTransformProgram, so a blocked timer K executes at K+2. The
// executing Fired callback still collides; the following callback does not.
// This assumes unchanged callbacks/program/timing, not universal Native safety.
inline CollisionLifetime scheduled_collision_lifetime(const std::array<LifetimeRecord, 18> &records,
                                                      int start, std::uint32_t enabled,
                                                      std::uint32_t active, int wait_current,
                                                      float wait_subframe, unsigned horizon,
                                                      bool clock_supported) {
    if (!horizon || start >= int(records.size()))
        return {};
    if (start < 0)
        return {0, true};
    bool blocked = false;
    for (int index = start; index < int(records.size()); ++index) {
        const auto &record = records[index];
        if (!record.kind)
            return {};
        blocked = blocked || (active && !record.allow_while_active);
        if (!(record.kind & enabled))
            continue;
        if (record.kind != kLifetimeDespawnKind)
            return {};
        if (!clock_supported || (active && active != kLifetimeWaitKind))
            return {0, true};
        unsigned update = 1;
        if (blocked) {
            if (wait_current < 0 || wait_subframe != 0.f)
                return {0, true};
            const auto due = std::uint64_t(wait_current) + 2;
            if (due > horizon)
                return {};
            update = unsigned(due);
        }
        return {update, false};
    }
    return {};
}
} // namespace th08::online::detail
