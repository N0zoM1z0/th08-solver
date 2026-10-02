#pragma once
#include <cstdlib>
#include <iostream>

namespace th08::test {
inline void check(bool valid, const char *message) {
    if (!valid) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
} // namespace th08::test
