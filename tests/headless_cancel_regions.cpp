#include "Player.hpp"
#include "modern/headless/runtime.hpp"

#include <array>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
using namespace th08;
using headless::CancelRegionScope;

void check(bool valid, const char *message) {
    if (!valid)
        throw std::runtime_error(message);
}

struct Snapshot {
    std::array<unsigned char, sizeof(g_Player.cancelRegions)> regions;
    int item_type;
};

Snapshot snapshot() {
    Snapshot result;
    std::memcpy(result.regions.data(), g_Player.cancelRegions, result.regions.size());
    result.item_type = g_Player.bulletCancelItemType;
    return result;
}

void restore(const Snapshot &saved) {
    std::memcpy(g_Player.cancelRegions, saved.regions.data(), saved.regions.size());
    g_Player.bulletCancelItemType = saved.item_type;
}

void reset() {
    for (auto &region : g_Player.cancelRegions)
        region.Reset();
    for (auto &region : g_Player.damageRegions)
        region.Reset();
    g_Player.bulletCancelItemType = -73;
}

// Outside the callback scope the original native loop remains the reference.
// Compare every region byte and both observable results, not just hit/no-hit.
void compare_query(Float3 *position) {
    check(!CancelRegionScope::known_empty(&g_Player), "scope leaked across queries");
    const auto before = snapshot();
    const auto expected = g_Player.CheckBulletCancelCollision(position, nullptr);
    const auto after = snapshot();
    restore(before);
    {
        CancelRegionScope scope(g_Player);
        const auto actual = g_Player.CheckBulletCancelCollision(position, nullptr);
        const auto observed = snapshot();
        check(actual == expected && observed.item_type == after.item_type &&
                  observed.regions == after.regions,
              "scoped cancel query changed native results or side effects");
    }
}

void check_predicates() {
    reset();
    Float3 center(10, 20, 0), interior(13, 20, 0), edge(14, 20, 0), outside(100, 200, 0);
    compare_query(nullptr); // Empty native regions never dereference the position.
    compare_query(&center);
    g_Player.CreateCircleCancelRegion(&center, 4, 0, 10, 9);
    compare_query(&interior);
    compare_query(&edge); // Native circular comparison is strict.
    compare_query(&outside);

    reset();
    auto *rect = g_Player.CreateRectCancelRegion(&center, 8, 6, 7, 10);
    compare_query(&edge); // Native rectangle comparison includes its boundary.
    rect->angle = 0.25f;
    compare_query(&interior);
    g_Player.CreateCircleCancelRegion(&center, 4, 0, 10, 9);
    compare_query(&center); // First matching slot controls item type and hit count.

    reset();
    rect = g_Player.CreateRectCancelRegion(&center, 8, 6, 7, 1);
    g_Player.cancelRegions[191] = *rect;
    rect->Deactivate();
    compare_query(&center); // The final slot must prevent the empty fast path.
    {
        CancelRegionScope scope(g_Player);
        g_Player.UpdateCollisionRegions();
        check(g_Player.CheckBulletCancelCollision(&center, nullptr) == 0,
              "expired region remained lethal to bullets");
    }
    {
        CancelRegionScope scope(g_Player);
        check(CancelRegionScope::known_empty(&g_Player), "new scope retained expired state");
    }
}

void check_same_callback_creation(bool circular) {
    reset();
    Float3 center(10, 20, 0);
    {
        CancelRegionScope outer(g_Player);
        check(CancelRegionScope::known_empty(&g_Player), "empty callback was not recognized");
        {
            CancelRegionScope inner(g_Player);
            if (circular)
                g_Player.CreateCircleCancelRegion(&center, 4, 0, 10, 9);
            else
                g_Player.CreateRectCancelRegion(&center, 8, 6, 9, 10);
            check(!CancelRegionScope::known_empty(&g_Player), "creation retained stale emptiness");
            check(g_Player.CheckBulletCancelCollision(&center, nullptr) == 2,
                  "new region was missed in the same callback");
        }
        check(!CancelRegionScope::known_empty(&g_Player), "nested creation restored stale state");
        check(g_Player.CheckBulletCancelCollision(&center, nullptr) == 2 &&
                  g_Player.cancelRegions[0].hitAccumulator == 2 &&
                  g_Player.bulletCancelItemType == 9,
              "nested scope changed native hit accounting");
    }
    check(!CancelRegionScope::known_empty(&g_Player), "callback proof escaped its lifetime");
}

void check_owner() {
    reset();
    auto other = std::make_unique<Player>();
    for (auto &region : other->cancelRegions)
        region.Reset();
    Float3 center(10, 20, 0);
    CancelRegionScope scope(g_Player);
    other->CreateCircleCancelRegion(&center, 4, 0, 10, 9);
    check(CancelRegionScope::known_empty(&g_Player) &&
              !CancelRegionScope::known_empty(other.get()) &&
              other->CheckBulletCancelCollision(&center, nullptr) == 2,
          "empty proof crossed its native player owner");
}
} // namespace

int main() {
    try {
        check_predicates();
        check_same_callback_creation(false);
        check_same_callback_creation(true);
        check_owner();
        std::cout << "headless cancel-region scope contracts passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
