#include "th08/shadow_protocol.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace s = th08::shadow;
void check(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}
template <class F> void rejects(F run) {
    bool rejected = false;
    try {
        run();
    } catch (const std::runtime_error &) {
        rejected = true;
    }
    check(rejected, "invalid wire accepted");
}
int main() try {
    s::Packet p;
    p.sequence = 0x123456789abcdef0ULL;
    p.epoch = 0x1020304050607080ULL;
    p.input = 0x1045;
    p.state[s::Field::X] = 0x80000000u;
    p.state[s::Field::Spell] = 0xffffffffu;
    auto bytes = s::encode(p);
    check(bytes[16] == 0xf0 && bytes[23] == 0x12, "wire byte order changed");
    auto q = s::decode(bytes.data(), bytes.size());
    check(!s::difference(p, q), "wire lost scalar or float bits");
    rejects([&] { s::decode(bytes.data(), bytes.size() - 1); });
    bytes[4] = 2;
    rejects([&] { s::decode(bytes.data(), bytes.size()); });
    rejects([&] {
        p.input = 2;
        s::encode(p);
    });
    p.input = q.input;
    q.state[s::Field::RngSeed] = 1;
    check(std::strcmp(s::difference(p, q), "rng_seed") == 0, "first divergence lost");
    q = p;
    q.epoch++;
    check(std::strcmp(s::difference(p, q), "epoch") == 0, "stale epoch accepted");
    s::Packet receipt;
    auto after = receipt.state;
    after[s::Field::Stage] = 1;
    s::advance_receipt(receipt, 1, after);
    check(receipt.sequence == 1 && receipt.epoch == 1 &&
              receipt.state[s::Field::CalculationLow] == 0,
          "control-only handoff conflated with gameplay calculation");
    s::advance_receipt(receipt, 0x41, after);
    check(receipt.sequence == 2 && receipt.epoch == 1 && receipt.input == 0x41,
          "ordinary receipt changed epoch or actual input");
    std::cout << "shadow protocol contracts passed\n";
    return 0;
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
}
