#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <th08/formats.hpp>
#include <th08/schema.hpp>

using namespace th08::resources;
namespace {
void check(bool valid, const char *message) {
    if (!valid) {
        std::cerr << message << '\n';
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
void store16(Bytes &b, std::size_t offset, std::uint16_t value) {
    b.at(offset) = std::uint8_t(value);
    b.at(offset + 1) = std::uint8_t(value >> 8);
}
void store32(Bytes &b, std::size_t offset, std::uint32_t value) {
    for (unsigned i = 0; i < 4; ++i)
        b.at(offset + i) = std::uint8_t(value >> (i * 8));
}
void store_float(Bytes &b, std::size_t offset, float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, 4);
    store32(b, offset, bits);
}
void sht_contracts() {
    Bytes bytes(130);
    store16(bytes, 2, 2);
    store_float(bytes, 12, 1.65f);
    for (unsigned offset : {36, 40, 44, 48})
        store_float(bytes, offset, 2);
    store32(bytes, 56, 72);
    store32(bytes, 64, 72);
    store32(bytes, 68, 32);
    store16(bytes, 72, 5);
    store16(bytes, 74, 2);
    store_float(bytes, 92, .5f);
    store_float(bytes, 96, 7);
    for (unsigned i = 0; i < 4; ++i)
        store32(bytes, 112 + i * 4, i + 1);
    store16(bytes, 128, 0xffff);
    const auto parsed = parse_sht(view(bytes));
    check(parsed.levels.size() == 2 && parsed.shots.size() == 2, "shared SHT ranges were merged");
    check(parsed.shots[1].speed == 7 && parsed.shots[1].callbacks[3] == 4,
          "SHT field offset mismatch");
    check(parsed.header.hurtbox_size == 1.65f, "SHT hitbox changed");
    auto malformed = bytes;
    malformed.resize(129);
    rejects([&] { parse_sht(view(malformed)); });
    malformed = bytes;
    store32(malformed, 56, 4);
    rejects([&] { parse_sht(view(malformed)); });
    malformed = bytes;
    store32(malformed, 36, 0x7fc00000);
    rejects([&] { parse_sht(view(malformed)); });
}
void anm_contracts() {
    Bytes bytes(116);
    store32(bytes, 0, 1);
    store32(bytes, 4, 1);
    store32(bytes, 40, 3);
    store32(bytes, 64, 76);
    store32(bytes, 68, 111);
    store32(bytes, 72, 96);
    store32(bytes, 76, 270);
    store_float(bytes, 88, 16);
    store_float(bytes, 92, 8);
    store16(bytes, 96, 25);
    store16(bytes, 98, 12);
    store32(bytes, 104, 1);
    store16(bytes, 108, 0xffff);
    auto parsed = parse_anm(view(bytes));
    check(parsed.scripts[0].raw_id == 111 && parsed.sprites[0].raw_id == 270, "ANM raw IDs lost");
    check(parsed.instructions.size() == 2 && parsed.instructions[0].opcode == 25,
          "ANM instruction mismatch");
    auto malformed = bytes;
    store16(malformed, 98, 0);
    rejects([&] { parse_anm(view(malformed)); });
    malformed = bytes;
    store32(malformed, 72, 4);
    rejects([&] { parse_anm(view(malformed)); });
    malformed = bytes;
    malformed.resize(111);
    rejects([&] { parse_anm(view(malformed)); });
}
void stage_contracts() {
    constexpr unsigned object = 0x494, quad = object + 28, instances = quad + 32,
                       script = instances + 20;
    Bytes bytes(script + 28);
    store16(bytes, 0, 1);
    store16(bytes, 2, 1);
    store32(bytes, 4, instances);
    store32(bytes, 8, script);
    store32(bytes, 0x490, object);
    store16(bytes, quad + 2, 28);
    store16(bytes, quad + 28, 0xffff);
    store16(bytes, instances + 16, 0xffff);
    store16(bytes, script + 6, 12);
    store32(bytes, script + 8, 123);
    store32(bytes, script + 20, 0xffffffff);
    const auto parsed = parse_std(view(bytes));
    check(parsed.objects.size() == 1 && parsed.quads.size() == 1 && parsed.instances.size() == 1,
          "STD geometry structure mismatch");
    check(parsed.instructions[0].words[0] == 123, "STD fixed-width payload mismatch");
    auto malformed = bytes;
    store16(malformed, quad + 2, 24);
    rejects([&] { parse_std(view(malformed)); });
    malformed = bytes;
    store16(malformed, instances, 1);
    rejects([&] { parse_std(view(malformed)); });
    malformed = bytes;
    store16(malformed, 2, 2);
    rejects([&] { parse_std(view(malformed)); });
}
void timeline_contracts() {
    Bytes bytes(116);
    store32(bytes, 0, 0x800);
    store16(bytes, 4, 1);
    store16(bytes, 6, 1);
    store32(bytes, 8, 88);
    store32(bytes, 72, 76);
    store32(bytes, 76, 0xffffffff);
    store16(bytes, 80, 0xffff);
    store16(bytes, 82, 12);
    store16(bytes, 92, 7);
    bytes[94] = 8;
    bytes[95] = 255;
    store16(bytes, 100, 13);
    bytes[102] = 12;
    bytes[103] = 255;
    store32(bytes, 104, 9);
    store32(bytes, 108, 0xffffffff);
    const auto parsed = parse_ecl(view(bytes));
    check(parsed.timelines.size() == 1 && parsed.timeline_instructions.size() == 2,
          "timeline instruction loss");
    check(parsed.timeline_instructions[1].opcode == 13, "timeline gate opcode changed");
    auto malformed = bytes;
    malformed[102] = 8;
    rejects([&] { parse_ecl(view(malformed)); });
    malformed = bytes;
    store32(malformed, 108, 0);
    rejects([&] { parse_ecl(view(malformed)); });
    check(check_payload(122, 232) == PayloadStatus::known &&
              check_payload(122, 104) == PayloadStatus::mismatch,
          "spell comments omitted from payload length");
    check(check_payload(35, 0) == PayloadStatus::unknown, "unverified opcode became known");
}
} // namespace
int main() {
    sht_contracts();
    anm_contracts();
    stage_contracts();
    timeline_contracts();
    std::cout << "SHT, ANM, STD, timeline and payload boundary contracts: passed\n";
}
