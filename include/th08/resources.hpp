#pragma once
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace th08::resources {
using Bytes = std::vector<std::uint8_t>;
struct View {
    const std::uint8_t *data = nullptr;
    std::size_t size = 0;
    View sub(std::size_t offset, std::size_t length) const;
};
inline View view(const Bytes &b) {
    return {b.data(), b.size()};
}
std::uint16_t u16(View b, std::size_t offset);
std::uint32_t u32(View b, std::size_t offset);
std::int32_t i32(View b, std::size_t offset);
Bytes read_file(const std::filesystem::path &path);
std::string sha256(View bytes);
void decrypt(Bytes &bytes, std::uint8_t key, std::uint8_t increment, std::size_t chunk,
             std::size_t limit);
Bytes decompress(View input, std::size_t expected);
struct Entry {
    std::string name;
    std::uint32_t offset, packed_size, metadata;
    std::size_t compressed_size;
};
class Archive {
    Bytes bytes_;
    std::vector<Entry> entries_;

  public:
    explicit Archive(Bytes bytes);
    const std::vector<Entry> &entries() const {
        return entries_;
    }
    View bytes() const {
        return view(bytes_);
    }
    Bytes decode(std::size_t index) const;
};
struct Instruction {
    std::uint32_t offset;
    std::int32_t time;
    std::int16_t opcode;
    std::uint16_t size, flags;
    std::uint8_t mask;
};
struct Subprogram {
    std::uint32_t offset, end, first, count;
};
struct SpellSite {
    std::uint16_t id;
    std::uint32_t sub, offset;
    std::uint8_t mask;
};
struct Ecl {
    // Contiguous instruction arena; subs refer to ranges, operands remain in
    // the caller-owned decrypted resource. No per-instruction payload allocation.
    std::vector<Instruction> instructions;
    std::vector<Subprogram> subs;
    std::vector<SpellSite> spells;
    std::size_t jump_targets_checked = 0;
    std::uint16_t timeline_count = 0;
};
Ecl parse_ecl(View input);
} // namespace th08::resources
