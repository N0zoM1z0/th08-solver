#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <limits>
#include <openssl/evp.h>
#include <set>
#include <stdexcept>
#include <th08/resources.hpp>
#include <th08/schema.hpp>

namespace th08::resources {
namespace {
constexpr std::size_t max_output = 256 * 1024 * 1024;
void demand(bool valid, const char *why) {
    if (!valid)
        throw std::runtime_error(why);
}
} // namespace
View View::sub(std::size_t offset, std::size_t length) const {
    demand(offset <= size && length <= size - offset, "truncated binary field");
    return {data ? data + offset : nullptr, length};
}
std::uint16_t u16(View b, std::size_t offset) {
    const auto v = b.sub(offset, 2);
    return std::uint16_t(v.data[0] | (std::uint16_t(v.data[1]) << 8));
}
std::uint32_t u32(View b, std::size_t offset) {
    const auto v = b.sub(offset, 4);
    return std::uint32_t(v.data[0]) | (std::uint32_t(v.data[1]) << 8) |
           (std::uint32_t(v.data[2]) << 16) | (std::uint32_t(v.data[3]) << 24);
}
std::int32_t i32(View b, std::size_t offset) {
    auto v = u32(b, offset);
    std::int32_t result;
    std::memcpy(&result, &v, 4);
    return result;
}
std::int16_t i16(View b, std::size_t offset) {
    const auto bits = u16(b, offset);
    std::int16_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
float f32(View b, std::size_t offset) {
    const auto bits = u32(b, offset);
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
Bytes read_file(const std::filesystem::path &path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    demand(bool(f), "cannot open input file");
    const auto n = f.tellg();
    demand(n >= 0 && n <= std::streamoff(512 * 1024 * 1024), "input file size outside bound");
    Bytes data(static_cast<std::size_t>(n));
    f.seekg(0);
    if (!data.empty())
        f.read(reinterpret_cast<char *>(data.data()), n);
    demand(bool(f), "cannot read complete input file");
    return data;
}
std::string sha256(View bytes) {
    std::array<unsigned char, 32> digest{};
    unsigned count = 0;
    demand(EVP_Digest(bytes.data, bytes.size, digest.data(), &count, EVP_sha256(), nullptr) == 1 &&
               count == 32,
           "SHA256 failed");
    static constexpr char hex[] = "0123456789abcdef";
    std::string out(64, '0');
    for (std::size_t i = 0; i < digest.size(); ++i) {
        out[2 * i] = hex[digest[i] >> 4];
        out[2 * i + 1] = hex[digest[i] & 15];
    }
    return out;
}
void decrypt(Bytes &bytes, std::uint8_t key, std::uint8_t increment, std::size_t chunk,
             std::size_t limit) {
    demand(chunk > 0 && chunk <= 0x1400, "invalid decryption block size");
    const std::size_t tail =
        (bytes.size() % chunk < chunk / 4 ? bytes.size() % chunk : 0) + (bytes.size() & 1);
    demand(tail <= bytes.size(), "invalid encrypted tail");
    std::size_t remaining = bytes.size() - tail, pos = 0;
    std::array<std::uint8_t, 0x1400> scratch{};
    while (remaining && limit) {
        const auto n = std::min(chunk, remaining);
        std::copy_n(bytes.data() + pos, n, scratch.data());
        std::size_t source = 0;
        for (int parity = 0; parity < 2; ++parity)
            for (std::ptrdiff_t dest = std::ptrdiff_t(n) - 1 - parity; dest >= 0; dest -= 2) {
                bytes[pos + std::size_t(dest)] = std::uint8_t(scratch[source++] ^ key);
                key = std::uint8_t(key + increment);
            }
        pos += n;
        remaining -= n;
        limit = limit > n ? limit - n : 0;
    }
}
Bytes decompress(View input, std::size_t expected) {
    demand(expected <= max_output, "LZSS output exceeds bound");
    Bytes output(expected);
    std::array<std::uint8_t, 8192> dictionary{}, initialized{};
    std::size_t cursor = 0, written = 0, write_head = 1;
    std::uint64_t reservoir = 0;
    unsigned bits = 0, padding = 0;
    auto take = [&](unsigned n) {
        while (bits < n) {
            std::uint8_t byte = 0;
            if (cursor < input.size)
                byte = input.data[cursor++];
            else {
                demand(written == expected && padding < 2, "truncated LZSS stream");
                ++padding;
            }
            reservoir = (reservoir << 8) | byte;
            bits += 8;
        }
        bits -= n;
        return std::uint32_t((reservoir >> bits) & ((std::uint64_t(1) << n) - 1));
    };
    auto put = [&](std::uint8_t byte) {
        demand(written < expected, "LZSS output overflow");
        output[written++] = byte;
        dictionary[write_head] = byte;
        initialized[write_head] = 1;
        write_head = (write_head + 1) & 8191;
    };
    for (;;) {
        if (take(1))
            put(std::uint8_t(take(8)));
        else {
            const auto offset = take(13);
            if (!offset)
                break;
            const auto count = take(4) + 3;
            demand(count <= expected - written, "LZSS match overflows output");
            for (std::uint32_t i = 0; i < count; ++i) {
                const auto index = (offset + i) & 8191;
                demand(initialized[index] != 0, "LZSS uninitialized dictionary reference");
                put(dictionary[index]);
            }
        }
    }
    demand(written == expected, "LZSS output length mismatch");
    return output;
}
Archive::Archive(Bytes bytes) : bytes_(std::move(bytes)) {
    const auto b = view(bytes_);
    demand(b.size >= 16 && std::memcmp(b.data, "PBGZ", 4) == 0, "not a PBGZ archive");
    Bytes header(b.data + 4, b.data + 16);
    decrypt(header, 0x1b, 0x37, 12, 0x400);
    const auto h = view(header);
    demand(u32(h, 0) > 123456 && u32(h, 4) >= 345694 && u32(h, 8) >= 567891,
           "invalid encoded archive header");
    const auto count = u32(h, 0) - 123456, table_offset = u32(h, 4) - 345678,
               table_size = u32(h, 8) - 567891;
    demand(count < 100000 && table_offset < b.size, "invalid archive table location");
    Bytes encrypted(b.data + table_offset, b.data + b.size);
    decrypt(encrypted, 0x3e, 0x9b, 0x80, 0x400);
    auto table = decompress(view(encrypted), table_size);
    const auto t = view(table);
    std::size_t p = 0;
    entries_.reserve(count);
    std::set<std::string> names;
    for (std::uint32_t i = 0; i < count; ++i) {
        std::size_t end = p;
        while (end < t.size && t.data[end])
            ++end;
        demand(end < t.size && end > p && end - p <= 255, "invalid member name");
        std::string name(reinterpret_cast<const char *>(t.data + p), end - p);
        demand(name != "." && name != ".." &&
                   name.find_first_of("/\\\t\r\n") == std::string::npos &&
                   names.insert(name).second,
               "unsafe or duplicate archive name");
        p = end + 1;
        entries_.push_back({std::move(name), u32(t, p), u32(t, p + 4), u32(t, p + 8), 0});
        p += 12;
    }
    for (; p < t.size; ++p)
        demand(t.data[p] == 0, "unparsed archive table data");
    for (std::size_t i = 0; i < entries_.size(); ++i) {
        auto &e = entries_[i];
        const auto end = i + 1 < entries_.size() ? entries_[i + 1].offset : table_offset;
        demand(e.offset >= 16 && e.offset < end && end <= table_offset &&
                   e.packed_size <= max_output,
               "invalid archive member range");
        e.compressed_size = end - e.offset;
    }
}
Bytes Archive::decode(std::size_t index) const {
    const auto &e = entries_.at(index);
    auto data = decompress(view(bytes_).sub(e.offset, e.compressed_size), e.packed_size);
    if (data.size() < 3 || std::memcmp(data.data(), "edz", 3) != 0)
        return data;
    demand(data.size() >= 4, "truncated content encryption header");
    struct Parameters {
        unsigned marker, key, increment, chunk, limit;
    };
    static constexpr Parameters params[] = {
        {0x5d, 0x1b, 0x37, 0x40, 0x2800},   {0x74, 0x51, 0xe9, 0x40, 0x3000},
        {0x71, 0xc1, 0x51, 0x1400, 0x2000}, {0x8a, 3, 0x19, 0x1400, 0x7800},
        {0x95, 0xab, 0xcd, 0x200, 0x1000},  {0xb7, 0x12, 0x34, 0x400, 0x2800},
        {0x9d, 0x35, 0x97, 0x80, 0x2800},   {0xaa, 0x99, 0x37, 0x400, 0x1000}};
    for (unsigned i = 0; i < 8; ++i)
        if (data[3] == params[i].marker - (i << 4) - 0x10) {
            data.erase(data.begin(), data.begin() + 4);
            const auto &p = params[i];
            decrypt(data, std::uint8_t(p.key), std::uint8_t(p.increment), p.chunk, p.limit);
            return data;
        }
    throw std::runtime_error("unknown content encryption marker");
}
Ecl parse_ecl(View b) {
    demand(u32(b, 0) == 0x800, "invalid ECL version");
    const auto n = u16(b, 4), timelines = u16(b, 6);
    demand(n > 0 && timelines <= 16, "invalid ECL directory");
    const std::size_t header_end = 72 + std::size_t(n) * 4;
    b.sub(0, header_end);
    std::vector<std::uint32_t> starts, boundaries;
    starts.reserve(n);
    boundaries.reserve(n + timelines + 1);
    for (unsigned i = 0; i < n; ++i)
        starts.push_back(u32(b, 72 + i * 4));
    boundaries = starts;
    for (unsigned i = 0; i < timelines; ++i)
        boundaries.push_back(u32(b, 8 + i * 4));
    for (auto offset : boundaries)
        demand(offset >= header_end && offset < b.size && offset % 4 == 0,
               "invalid ECL section offset");
    std::sort(boundaries.begin(), boundaries.end());
    demand(std::adjacent_find(boundaries.begin(), boundaries.end()) == boundaries.end(),
           "duplicate ECL section");
    demand(b.size <= std::numeric_limits<std::uint32_t>::max(), "oversized ECL");
    boundaries.push_back(std::uint32_t(b.size));
    Ecl out;
    out.timeline_count = timelines;
    out.subs.reserve(n);
    out.instructions.reserve(b.size / 24);
    for (unsigned sid = 0; sid < n; ++sid) {
        const auto start = starts[sid],
                   end = *std::upper_bound(boundaries.begin(), boundaries.end(), start);
        const auto first = std::uint32_t(out.instructions.size());
        for (std::uint32_t p = start; p < end;) {
            demand(end - p >= 12, "truncated ECL instruction header");
            const auto time = i32(b, p);
            const auto raw = u16(b, p + 4);
            const auto opcode = raw == 0xffff ? std::int16_t(-1) : std::int16_t(raw);
            const auto length = u16(b, p + 6), flags = u16(b, p + 10);
            demand(length >= 12 && length % 4 == 0 && length <= end - p,
                   "invalid ECL instruction size");
            demand(raw <= 184 || (raw == 0xffff && time == -1 && p + length == end),
                   "invalid ECL opcode or sentinel");
            const auto mask = b.data[p + 9];
            if (opcode >= 0) {
                const auto payload = check_payload(unsigned(opcode), length - 12);
                if (payload == PayloadStatus::mismatch)
                    throw std::runtime_error(
                        "ECL payload schema mismatch: opcode=" + std::to_string(opcode) +
                        " offset=" + std::to_string(p) +
                        " payload_bytes=" + std::to_string(length - 12));
                out.unknown_payloads += payload == PayloadStatus::unknown;
            }
            out.instructions.push_back({p, time, opcode, length, flags, mask});
            if (opcode == 122) {
                demand(length >= 116, "truncated spell metadata");
                out.spells.push_back({u16(b, p + 14), sid, p, mask});
            }
            p += length;
        }
        const auto count = std::uint32_t(out.instructions.size()) - first;
        out.subs.push_back({start, end, first, count});
        for (std::uint32_t k = first; k < first + count; ++k) {
            const auto &ins = out.instructions[k];
            if (ins.opcode == 4 || ins.opcode == 5 || (ins.opcode >= 40 && ins.opcode <= 51)) {
                const auto operand = (ins.opcode >= 40) ? 12U : 4U;
                demand(ins.size >= 12 + operand + 4, "truncated ECL jump");
                const auto target = std::int64_t(ins.offset) + i32(b, ins.offset + 12 + operand);
                const auto begin = out.instructions.begin() + first, finish = begin + count;
                auto found = std::lower_bound(
                    begin, finish, target,
                    [](const Instruction &i, std::int64_t t) { return i.offset < t; });
                demand(found != finish && found->offset == target,
                       "ECL jump outside instruction boundary");
                ++out.jump_targets_checked;
            }
        }
    }
    // Timeline records have a distinct eight-byte header and one-byte length.
    // Their clocks can block on external events; timestamps are not wall time.
    constexpr unsigned payload_sizes[] = {24, 24, 28, 20, 28, 20, 4,  0, 8,
                                          4,  4,  28, 28, 4,  4,  24, 0};
    for (unsigned id = 0; id < timelines; ++id) {
        const auto start = u32(b, 8 + id * 4);
        const auto end = *std::upper_bound(boundaries.begin(), boundaries.end(), start);
        const auto first = std::uint32_t(out.timeline_instructions.size());
        auto p = start;
        for (;;) {
            demand(p <= end && end - p >= 8, "unterminated ECL timeline");
            const auto time = i32(b, p);
            if (time < 0)
                break;
            const auto opcode = u16(b, p + 4);
            const auto length = b.data[p + 6], mask = b.data[p + 7];
            demand(opcode < 17 && length >= 8 && length <= end - p, "invalid ECL timeline record");
            demand(unsigned(length - 8) == payload_sizes[opcode],
                   "timeline payload schema mismatch");
            out.timeline_instructions.push_back({p, time, opcode, length, mask});
            p += length;
        }
        out.timelines.push_back(
            {start, p, first, std::uint32_t(out.timeline_instructions.size()) - first});
    }
    return out;
}
} // namespace th08::resources
