#include "th08/hazard_probe.hpp"

#include <iostream>
#include <limits>

namespace {
void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}
} // namespace

int main() {
    using namespace th08::hazard_probe;
    try {
        Laser first{100, 200, 590, 160, 192, 128, 0.25f, -1, false};
        Laser second{120, 220, 20, 40, 192, 128, 0.5f, 7, true};
        const std::vector<Frame> baseline{{393, {first, second}}, {394, {first}}};
        auto alternative = baseline;
        std::reverse(alternative.front().lasers.begin(), alternative.front().lasers.end());
        require(same_window(baseline, alternative, 393, 2), "call ordering changed geometry");
        alternative.front().lasers.push_back(first);
        require(!same_window(baseline, alternative, 393, 2), "duplicate call was discarded");
        alternative = baseline;
        alternative.back().lasers.front().pooled_slot = 0;
        require(!same_window(baseline, alternative, 393, 2), "source attribution was ignored");
        alternative = baseline;
        alternative.back().lasers.front().graze_enabled = true;
        require(!same_window(baseline, alternative, 393, 2), "graze feedback was ignored");
        alternative = baseline;
        alternative.back().lasers.front().angle = std::nextafter(first.angle, 1.0f);
        require(!same_window(baseline, alternative, 393, 2), "float drift was tolerated");
        alternative = baseline;
        alternative.pop_back();
        require(!same_window(baseline, alternative, 393, 2), "truncated window counted as equal");
        require(!same_window({}, {}, 393, 0), "empty windows counted as agreement");
        alternative = baseline;
        alternative.back().update = 395;
        require(!same_window(baseline, alternative, 393, 2), "wrong update counted as equal");
        alternative = baseline;
        alternative.back().lasers.front().angle = std::numeric_limits<float>::quiet_NaN();
        bool rejected = false;
        try {
            same_window(baseline, alternative, 393, 2);
        } catch (const std::runtime_error &) {
            rejected = true;
        }
        require(rejected, "non-finite geometry was accepted");
        std::cout << "hazard probe comparison contract passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
