#include <cstdlib>
#include <iostream>
#include <random>
#include <th08/bullet_slots.hpp>

using th08::bullet::Slots;
void check(bool valid, const char *message) {
    if (!valid) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
std::optional<std::uint16_t> scan(const Slots::Occupancy &occupied, unsigned cursor) {
    for (unsigned count = 0; count < Slots::capacity; ++count) {
        if (!occupied[cursor])
            return std::uint16_t(cursor);
        if (++cursor == Slots::capacity)
            cursor = 0;
    }
    return std::nullopt;
}
int main() {
    Slots slots;
    const auto parent = slots.reserve();
    check(parent == 0 && slots.cursor() == 0, "reservation advanced the source cursor too early");
    const auto child = slots.reserve();
    check(child == 1 && slots.cursor() == 0, "nested child reused its parent slot");
    slots.finish_spawn(*child);
    slots.finish_spawn(*parent);
    check(slots.cursor() == 1 && slots.next() == 2,
          "parent completion did not overwrite the nested child cursor");
    check(slots.release(0) && !slots.release(0) && slots.available() == 1535,
          "duplicate release corrupted free count");
    for (unsigned hole = 0; hole < Slots::capacity; ++hole) {
        Slots::Occupancy occupied;
        occupied.fill(true);
        occupied[hole] = false;
        for (unsigned cursor : {0U, hole, (hole + 1) % Slots::capacity, 1535U}) {
            Slots one_hole(occupied, cursor);
            check(one_hole.next() == hole && one_hole.reserve() == hole && !one_hole.next() &&
                      !one_hole.reserve() && one_hole.cursor() == cursor &&
                      one_hole.available() == 0,
                  "near-full ring selection or full-pool failure changed cursor");
        }
    }
    std::mt19937 random(20260915);
    Slots::Occupancy occupied{};
    Slots actual;
    unsigned cursor = 0, available = Slots::capacity;
    for (unsigned operation = 0; operation < 300000; ++operation) {
        if (random() % 3 == 0) {
            const unsigned index = random() % Slots::capacity;
            const bool was_occupied = occupied[index];
            check(actual.release(index) == was_occupied,
                  "release status differed from occupancy scan");
            if (was_occupied) {
                occupied[index] = false;
                ++available;
            }
        } else {
            const auto expected = scan(occupied, cursor);
            check(actual.reserve() == expected, "bitset selection differed from source-order scan");
            if (expected) {
                occupied[*expected] = true;
                --available;
                actual.finish_spawn(*expected);
                cursor = (*expected + 1) % Slots::capacity;
            }
        }
        check(actual.cursor() == cursor && actual.available() == available &&
                  actual.next() == scan(occupied, cursor),
              "pool cursor, count or next slot drifted");
    }
    actual.release(0);
    const auto snapshot = actual;
    const auto selected = actual.reserve();
    check(selected.has_value(), "snapshot fixture unexpectedly has no free slot");
    check(!snapshot.occupied(*selected) && actual.occupied(*selected),
          "pool snapshot borrowed mutable occupancy");
    std::cout << "Slot occupancy, ring selection, saturation, nested cursor order and snapshots: "
                 "passed\n";
}
