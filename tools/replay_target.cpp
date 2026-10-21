// Repair legacy native replay export identity without changing gameplay bytes.
// The target declaration is not evidence of successful retail execution.
#include "modern/headless/replay_check.hpp"
#include "pbg/Lzss.hpp"
#include "th08/resources.hpp"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <unistd.h>

namespace {
using th08::resources::Bytes;
using th08::resources::view;
constexpr std::size_t header_size = 0x68, payload_limit = 4 * 1024 * 1024;
constexpr std::size_t identity_begin = 0x124 - header_size, identity_end = 0x12c - header_size;
constexpr std::uint32_t checksum_origin = 0x3f000318;
struct Failure : std::runtime_error {
    using std::runtime_error::runtime_error;
};
void require(bool ok, const char *reason) {
    if (!ok)
        throw Failure(reason);
}
Bytes read(const char *path) {
    std::ifstream input(path, std::ios::binary);
    require(bool(input), "input_unreadable");
    Bytes result;
    std::array<char, 65536> buffer{};
    while (input.read(buffer.data(), buffer.size()) || input.gcount()) {
        require(result.size() + std::size_t(input.gcount()) <= 2 * payload_limit,
                "input_too_large");
        result.insert(result.end(), buffer.data(), buffer.data() + input.gcount());
    }
    require(input.eof(), "input_read_failed");
    return result;
}
void put(Bytes &bytes, std::size_t offset, std::uint32_t value) {
    for (unsigned i = 0; i < 4; ++i)
        bytes.at(offset + i) = std::uint8_t(value >> (8 * i));
}
void obfuscate(Bytes &bytes, std::size_t end, bool encode) {
    auto key = bytes.at(0x15);
    for (std::size_t i = 0x18; i < end; ++i) {
        bytes[i] = std::uint8_t(encode ? bytes[i] + key : bytes[i] - key);
        key = std::uint8_t(key + 7);
    }
}
std::uint32_t checksum(const Bytes &bytes, std::size_t end) {
    auto result = checksum_origin;
    for (std::size_t i = 0x15; i < end; ++i)
        result += bytes[i];
    return result;
}
Bytes decode(const Bytes &wire, std::size_t size) {
    try {
        return th08::resources::decompress(view(wire).sub(header_size, size),
                                           th08::resources::u32(view(wire), 0x1c));
    } catch (...) {
        throw Failure("invalid_compressed_payload");
    }
}
void release(std::uint8_t *bytes) {
    GlobalFree(bytes);
}
void write_new(const char *path, const Bytes &bytes) {
    // O_EXCL also rejects aliases/symlinks to the immutable input.
    const int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
    require(fd >= 0, "output_exists_or_unwritable");
    std::size_t written = 0;
    while (written < bytes.size()) {
        const auto count = write(fd, bytes.data() + written, bytes.size() - written);
        if (count < 0 && errno == EINTR)
            continue;
        if (count <= 0) {
            close(fd);
            unlink(path);
            throw Failure("output_write_failed");
        }
        written += std::size_t(count);
    }
    require(close(fd) == 0, "output_close_failed");
}
} // namespace

int main(int argc, char **argv) try {
    require(argc == 3, "usage_th08_replay_target_INPUT_OUTPUT");
    const auto input = read(argv[1]);
    auto wire = input;
    require(wire.size() >= header_size && std::memcmp(wire.data(), "T8RP", 4) == 0 &&
                th08::resources::u16(view(wire), 4) == 6,
            "unsupported_replay_format");
    const auto stored_size = th08::resources::u32(view(wire), 0xc);
    require(stored_size >= header_size && stored_size <= wire.size(), "invalid_stored_size");
    obfuscate(wire, stored_size, false);
    require(checksum(wire, stored_size) == th08::resources::u32(view(wire), 0x10),
            "replay_checksum_mismatch");
    const auto compressed_size = th08::resources::u32(view(wire), 0x18);
    const auto decoded_size = th08::resources::u32(view(wire), 0x1c);
    require(compressed_size == stored_size - header_size && decoded_size >= 0xcc &&
                decoded_size <= payload_limit,
            "invalid_payload_size");
    const auto original = decode(wire, compressed_size);
    const auto &target = th08::headless::replay_executable_target;
    require(std::memcmp(original.data() + identity_end, target.version, 6) == 0,
            "unsupported_executable_version");
    const auto old_size = th08::resources::u32(view(original), identity_begin);
    const auto old_checksum = th08::resources::u32(view(original), identity_begin + 4);
    const bool changed = old_size == 0 && old_checksum == 0;
    require(changed || (old_size == target.size && old_checksum == target.checksum),
            "unsupported_executable_identity");
    Bytes output = input;
    if (changed) {
        auto corrected = original;
        put(corrected, identity_begin, target.size);
        put(corrected, identity_begin + 4, target.checksum);
        int size = 0;
        std::unique_ptr<std::uint8_t, decltype(&release)> encoded(
            th08::Lzss::Encode(corrected.data(), int(corrected.size()), &size), release);
        require(bool(encoded) && size > 0 && std::size_t(size) <= 2 * payload_limit,
                "encoding_failed");
        output.assign(wire.begin(), wire.begin() + header_size);
        output.insert(output.end(), encoded.get(), encoded.get() + size);
        put(output, 0xc, std::uint32_t(output.size()));
        put(output, 0x18, std::uint32_t(size));
        // Check the actual emitted stream before obfuscation/output. All input,
        // stage state, FPS data, reserved payload and trailing user bytes persist.
        require(decode(output, std::size_t(size)) == corrected, "encoding_round_trip_failed");
        put(output, 0x10, checksum(output, output.size()));
        obfuscate(output, output.size(), true);
        output.insert(output.end(), input.begin() + stored_size, input.end());
    }
    auto preserved = original;
    preserved.erase(preserved.begin() + identity_begin, preserved.begin() + identity_end);
    write_new(argv[2], output);
    std::printf("{\"producer\":\"th08_replay_target\",\"target\":\"retail-1.00d\","
                "\"input_sha256\":\"%s\",\"output_sha256\":\"%s\",\"changed\":%s,"
                "\"decoded_payload_bytes\":%zu,\"preserved_payload_sha256\":\"%s\","
                "\"input_exe_size\":%u,\"input_exe_checksum\":%u,"
                "\"replay_target_exe_size\":%u,\"replay_target_exe_checksum\":%u,"
                "\"all_other_decoded_bytes_preserved\":true,\"certifies_retail_gameplay\":false}\n",
                th08::resources::sha256(view(input)).c_str(),
                th08::resources::sha256(view(output)).c_str(), changed ? "true" : "false",
                original.size(), th08::resources::sha256(view(preserved)).c_str(), old_size,
                old_checksum, target.size, target.checksum);
    return std::fflush(stdout) == 0 ? 0 : 2;
} catch (const Failure &failure) {
    std::fprintf(stderr, "%s\n", failure.what());
    return 2;
} catch (...) {
    std::fprintf(stderr, "replay_target_failed\n");
    return 2;
}
