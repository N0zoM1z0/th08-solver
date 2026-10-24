#pragma once
#include <cstdint>
namespace th08::shadow {
// The next receipt sequence includes control-only handoffs. Release/press Z in
// four-update groups; preserve movement/form and the existing alternating Ctrl.
inline std::uint16_t dialogue_input(std::uint16_t previous, std::uint64_t next) {
    return std::uint16_t((previous & 0xf4) | ((next % 8 >= 4) ? 1 : 0) | ((next & 1) ? 0x1000 : 0));
}
} // namespace th08::shadow
