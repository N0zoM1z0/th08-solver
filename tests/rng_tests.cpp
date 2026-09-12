#include <cstdlib>
#include <iostream>
#include <th08/rng.hpp>

using th08::random::Rng;
void check(bool valid, const char *message) {
    if (!valid) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
int main() {
    Rng rng(1234);
    check(!rng.restore_saved_seed() && rng.seed() == 1234,
          "unknown saved seed silently replaced current state");
    check(rng.range_u16(0) == 0 && rng.range_u32(0) == 0 && rng.generation_count() == 0,
          "zero integer range consumed RNG");
    check(rng.range_float(0) == 0 && rng.generation_count() == 2, "zero float range skipped RNG");
    rng.save_seed();
    const auto before = rng.state();
    const auto first = rng.next_u32();
    check(rng.restore_saved_seed() && rng.generation_count() == before.generation_count + 2,
          "saved seed incorrectly rolled back generation count");
    check(rng.next_u32() == first, "saved seed failed deterministic replay");
    Rng clone(before);
    check(clone.next_u32() == first && clone.generation_count() == before.generation_count + 2,
          "full RNG snapshot failed deterministic replay");
    rng.set_seed(1);
    check(rng.generation_count() == 6, "setting seed reset draw count");
    rng.reset_generation_count();
    check(rng.seed() == 1 && rng.generation_count() == 0, "resetting draw count changed seed");
    Rng wrapped(0, 0xffffffffU);
    wrapped.next_u16();
    check(wrapped.generation_count() == 0, "draw counter failed unsigned wrap");
    for (unsigned seed = 0; seed < 65536; ++seed) {
        Rng words{std::uint16_t(seed)}, separate{std::uint16_t(seed)};
        const auto high = separate.next_u16();
        const auto low = separate.next_u16();
        check(words.next_u32() == ((std::uint32_t(high) << 16) | low), "U32 word ordering changed");
        check(words.generation_count() == 2 && words.seed() == separate.seed(),
              "U32 draw count or final seed changed");
    }
    std::cout << "Explicit RNG state, word order, zero-range draws and seed restoration: passed\n";
}
