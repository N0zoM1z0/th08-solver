#pragma once
#include <cstddef>
#include <stdexcept>

namespace th08::resources {
struct OpcodeSchema {
    const char *name;
    // i: signed 32-bit, I: raw unsigned 32-bit, f: float32, h: packed 16-bit.
    // Null means the payload layout is not yet verified.
    const char *fields;
};
enum class PayloadStatus { known, unknown, mismatch };
const OpcodeSchema &opcode_schema(unsigned opcode);
PayloadStatus check_payload(unsigned opcode, std::size_t bytes);
} // namespace th08::resources
