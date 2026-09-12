#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <th08/resources.hpp>
using namespace th08::resources;
void check(bool value, const char *why) {
    if (!value) {
        std::cerr << why << '\n';
        std::exit(1);
    }
}
template <class F> void rejects(F function) {
    try {
        function();
    } catch (const std::runtime_error &) {
        return;
    }
    check(false, "malformed resource accepted");
}
void store16(Bytes &b, std::size_t p, std::uint16_t v) {
    b.at(p) = std::uint8_t(v);
    b.at(p + 1) = std::uint8_t(v >> 8);
}
void store32(Bytes &b, std::size_t p, std::uint32_t v) {
    for (unsigned i = 0; i < 4; ++i)
        b.at(p + i) = std::uint8_t(v >> (i * 8));
}
struct BitWriter {
    Bytes bytes;
    unsigned bits = 0;
    void put(unsigned value, unsigned count) {
        for (unsigned i = count; i > 0; --i) {
            if (bits % 8 == 0)
                bytes.push_back(0);
            bytes.back() |= std::uint8_t(((value >> (i - 1)) & 1) << (7 - bits % 8));
            ++bits;
        }
    }
};
int main() {
    check(sha256({nullptr, 0}) ==
              "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
          "SHA256 mismatch");
    rejects([] { u32({nullptr, 0}, 0); });
    rejects([] { Archive a(Bytes(16)); });
    BitWriter literal;
    literal.put(1, 1);
    literal.put('A', 8);
    literal.put(0, 1);
    literal.put(1, 13);
    literal.put(0, 4); // Overlapping AAA copy.
    literal.put(0, 1);
    literal.put(0, 13);
    check(decompress(view(literal.bytes), 4) == Bytes({'A', 'A', 'A', 'A'}),
          "overlapping LZSS match failed");
    rejects([&] { decompress(view(literal.bytes), 3); });
    rejects([&] { decompress(view(literal.bytes), 5); });
    rejects([] { decompress({nullptr, 0}, 1); });
    BitWriter uninitialized;
    uninitialized.put(0, 1);
    uninitialized.put(1, 13);
    uninitialized.put(0, 4);
    rejects([&] { decompress(view(uninitialized.bytes), 3); });
    Bytes encrypted{0, 1, 2, 3, 4, 5, 6, 7};
    decrypt(encrypted, 0, 0, 4, 4);
    check(encrypted == Bytes({3, 1, 2, 0, 4, 5, 6, 7}), "decryption scatter or limit mismatch");
    Bytes ecl(100);
    store32(ecl, 0, 0x800);
    store16(ecl, 4, 1);
    store32(ecl, 72, 76);
    store16(ecl, 80, 3);
    store16(ecl, 82, 12);
    ecl[85] = 255;
    store32(ecl, 88, 0xffffffff);
    store16(ecl, 92, 0xffff);
    store16(ecl, 94, 12);
    const auto parsed = parse_ecl(view(ecl));
    check(parsed.subs.size() == 1 && parsed.instructions.size() == 2, "ECL parsing failed");
    auto bad = ecl;
    store16(bad, 82, 0);
    rejects([&] { parse_ecl(view(bad)); });
    bad = ecl;
    store32(bad, 72, 75);
    rejects([&] { parse_ecl(view(bad)); });
    bad = ecl;
    store16(bad, 80, 200);
    rejects([&] { parse_ecl(view(bad)); });
    bad = ecl;
    store16(bad, 80, 4); // Jump opcode without its required payload.
    rejects([&] { parse_ecl(view(bad)); });
    std::cout << "resource bounds, scatter decryption, LZSS overlap, ECL structure: passed\n";
}
