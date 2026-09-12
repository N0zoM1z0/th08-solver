#pragma once
#include <th08/bullet_slots.hpp>

struct SlotComparison {
    std::uint64_t operations = 0, mismatches = 0;
};
inline SlotComparison compare_slots() {
    using th08::bullet::Slots;
    namespace reference = slot_reference;
    SlotComparison result;
    std::mt19937 random(20260916);
    for (unsigned scenario = 0; scenario < 1000; ++scenario) {
        reference::BulletManager source;
        Slots::Occupancy occupancy;
        unsigned available = Slots::capacity;
        for (unsigned i = 0; i < Slots::capacity; ++i) {
            occupancy[i] = random() % 4 < scenario % 5;
            if (occupancy[i]) {
                --available;
                source.bullets[i].state = reference::BULLET_STATE_FIRED;
            }
        }
        const unsigned cursor = random() % Slots::capacity;
        source.bulletCursor = &source.bullets[cursor];
        Slots actual(occupancy, cursor);
        std::array<unsigned, 8> pending{};
        unsigned depth = 0;
        for (unsigned operation = 0; operation < 300; ++operation) {
            bool equal = true;
            const unsigned action = random() % 3;
            if (action == 0) {
                const unsigned index = random() % Slots::capacity;
                const bool used = source.bullets[index].state != reference::BULLET_STATE_UNUSED;
                equal = actual.release(index) == used;
                if (used)
                    ++available;
                source.bullets[index].state = reference::BULLET_STATE_UNUSED;
            } else if ((action == 1 && depth != 0) || depth == pending.size()) {
                const unsigned index = pending[--depth];
                source.finish(index);
                actual.finish_spawn(index);
            } else {
                unsigned selected = 0;
                const bool found = source.select(selected) == 0;
                const auto allocated = actual.reserve();
                equal = allocated.has_value() == found && (!found || *allocated == selected);
                if (found) {
                    source.bullets[selected].state = reference::BULLET_STATE_FIRED;
                    pending[depth++] = selected;
                    --available;
                }
            }
            equal = equal && actual.available() == available &&
                    actual.cursor() == unsigned(source.bulletCursor - source.bullets);
            if (!equal) {
                if (result.mismatches == 0)
                    std::cerr << "Slot divergence scenario=" << scenario
                              << " operation=" << operation << '\n';
                ++result.mismatches;
                break;
            }
            ++result.operations;
        }
    }
    return result;
}
