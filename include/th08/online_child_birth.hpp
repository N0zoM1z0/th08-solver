#pragma once
#include "th08/online.hpp"
#include <cmath>
#include <limits>
#include <stdexcept>

namespace th08::online::detail {
inline constexpr std::uint32_t kChildSpawnKind = 0x1000000u, kChildWaitKind = 0x20000u;
struct ChildCursorRecord {
    std::uint32_t kind = 0;
    bool allow_while_active = false;
};
struct ChildBirthCursor {
    int index = -1;
    unsigned update = 0;
    bool unsupported = false;
};
// Decode only a child record reachable through mask-disabled records. Any other
// enabled record ends this narrow preview. Native checks the active gate before
// skipping a disabled record; one blocked WAIT delays the whole remaining scan.
inline ChildBirthCursor child_birth_cursor(const std::array<ChildCursorRecord, 18> &records,
                                           int start, std::uint32_t enabled, std::uint32_t active,
                                           int wait_current, float wait_subframe,
                                           unsigned horizon) {
    if (!horizon || start >= int(records.size()))
        return {};
    if (start < 0)
        return {-1, 0, true};
    bool blocked = false;
    for (int index = start; index < int(records.size()); ++index) {
        const auto &record = records[index];
        if (!record.kind)
            return {};
        blocked = blocked || (active && !record.allow_while_active);
        if (!(record.kind & enabled))
            continue;
        if (record.kind != kChildSpawnKind)
            return {};
        if (index + 1 >= int(records.size()))
            return {-1, 0, true};
        unsigned update = 1;
        if (blocked) {
            if (active != kChildWaitKind || wait_current < 0 || wait_subframe != 0.f)
                return {-1, 0, true};
            const auto due = std::uint64_t(wait_current) + 2;
            if (due > horizon)
                return {};
            update = unsigned(due);
        }
        return {index, update, false};
    }
    return {};
}
// Pool order may give a newborn zero or one movement on its birth update.
// Enclose both orders through the following update, without drawing an angle.
inline std::array<Box, 2> child_birth_boxes(const ChildBirth &birth, float grow) {
    float min_x = birth.x, max_x = birth.x, min_y = birth.y, max_y = birth.y;
    const float infinity = std::numeric_limits<float>::infinity();
    std::array<Box, 2> result;
    for (unsigned age = 0; age < result.size(); ++age) {
        min_x = std::nextafter(min_x - birth.max_speed, -infinity);
        min_y = std::nextafter(min_y - birth.max_speed, -infinity);
        max_x = std::nextafter(max_x + birth.max_speed, infinity);
        max_y = std::nextafter(max_y + birth.max_speed, infinity);
        const float margin = grow * (birth.update + age);
        result[age] = {std::nextafter(min_x - birth.width / 2 - margin, -infinity),
                       std::nextafter(min_y - birth.height / 2 - margin, -infinity),
                       std::nextafter(max_x + birth.width / 2 + margin, infinity),
                       std::nextafter(max_y + birth.height / 2 + margin, infinity)};
        const auto &box = result[age];
        if (!std::isfinite(box.x0) || !std::isfinite(box.y0) || !std::isfinite(box.x1) ||
            !std::isfinite(box.y1))
            throw std::runtime_error("unsupported_online_child_birth_bounds");
    }
    return result;
}
} // namespace th08::online::detail
