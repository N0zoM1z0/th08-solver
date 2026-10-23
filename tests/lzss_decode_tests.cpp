#include "modern/headless/lzss_decode.hpp"
#include "test_support.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
using Byte = std::uint8_t;
using Dictionary = std::array<Byte, 8192>;
using th08::test::check;

// Independent scalar oracle: the original modern reader fetches one byte only
// at its MSB, shifts each field one bit at a time, and supplies zero after EOF.
struct ScalarBits {
    const std::vector<Byte> &input;
    std::size_t cursor = 0;
    unsigned mask = 128, byte = 0;
    unsigned take(unsigned count) {
        unsigned result = 0;
        while (count--) {
            if (mask == 128)
                byte = cursor < input.size() ? input[cursor++] : 0;
            result = result * 2 + ((byte & mask) != 0);
            mask /= 2;
            if (!mask)
                mask = 128;
        }
        return result;
    }
};

std::vector<Byte> scalar_decode(const std::vector<Byte> &input, Dictionary &dictionary) {
    ScalarBits bits{input};
    std::vector<Byte> output;
    unsigned head = 1;
    const auto write = [&](Byte value) {
        // An invalid test stream must fail before invoking an unbounded legacy
        // write. This guard belongs only to the oracle, not production semantics.
        if (output.size() >= input.size() * 16 + 64)
            throw std::runtime_error("test stream exceeded output bound");
        output.push_back(value);
        dictionary[head] = value;
        head = (head + 1) % 8192;
    };
    for (;;) {
        if (bits.take(1)) {
            write(Byte(bits.take(8)));
        } else {
            const auto offset = bits.take(13);
            if (!offset)
                break;
            const auto length = bits.take(4) + 3;
            for (unsigned i = 0; i != length; ++i)
                write(dictionary[(offset + i) % 8192]);
        }
    }
    while (bits.mask != 128)
        bits.take(1);
    return output;
}

struct Stream {
    std::vector<Byte> bytes;
    unsigned position = 0;
    void field(unsigned value, unsigned width) {
        for (unsigned bit = width; bit; --bit) {
            if (position % 8 == 0)
                bytes.push_back(0);
            bytes.back() |= ((value >> (bit - 1)) & 1) << (7 - position % 8);
            ++position;
        }
    }
    void literal(Byte value) {
        field(1, 1);
        field(value, 8);
    }
    void match(unsigned offset, unsigned length) {
        field(0, 1);
        field(offset, 13);
        field(length - 3, 4);
    }
    void end() {
        field(0, 1);
        field(0, 13);
    }
};

std::uint32_t random_word(std::uint32_t &state) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

void compare(const std::vector<Byte> &input, Dictionary &expected, Dictionary &actual) {
    const auto output = scalar_decode(input, expected);
    // Exact caller-supplied output capacity, with both ends guarded. An empty
    // stream still supplies a valid pointer for the zero-length result.
    std::vector<Byte> storage(output.size() + 2, 0xa5);
    Byte native_dictionary[8192];
    std::copy(actual.begin(), actual.end(), native_dictionary);
    const auto count = th08::headless::decode_lzss_fields(input.data(), int(input.size()),
                                                          storage.data() + 1, native_dictionary);
    check(count == output.size(), "LZSS output length differs from scalar oracle");
    check(std::equal(output.begin(), output.end(), storage.begin() + 1),
          "LZSS output differs from scalar oracle");
    check(storage.front() == 0xa5 && storage.back() == 0xa5, "LZSS output boundary overwritten");
    std::copy(std::begin(native_dictionary), std::end(native_dictionary), actual.begin());
    check(actual == expected, "LZSS final persistent dictionary differs");
}
} // namespace

int main() {
    try {
        Dictionary expected{}, actual{};
        std::uint32_t random = 0x8e63452d;
        for (auto &value : expected)
            value = Byte(random_word(random));
        actual = expected;

        Stream overlap;
        overlap.literal(0x42);
        overlap.match(1, 18);    // Every read observes the preceding write.
        overlap.match(8191, 18); // Source wraps through index zero.
        overlap.end();
        compare(overlap.bytes, expected, actual);

        Stream wrap;
        for (unsigned i = 0; i < 8192 + 37; ++i)
            wrap.literal(Byte(i));
        wrap.match(8190, 18);
        wrap.match(35, 18);
        wrap.end();
        compare(wrap.bytes, expected, actual);

        // Back-to-back calls deliberately carry all dictionary bytes. External
        // changes stand for an interleaved encoder initialization/use.
        for (unsigned trial = 0; trial < 256; ++trial) {
            if (trial % 17 == 0) {
                for (auto &value : expected)
                    value = Byte(random_word(random));
                actual = expected;
            }
            Stream stream;
            for (unsigned token = 0; token < 256; ++token) {
                if (random_word(random) % 3)
                    stream.literal(Byte(random_word(random)));
                else
                    stream.match(random_word(random) % 8191 + 1, random_word(random) % 16 + 3);
            }
            stream.end();
            compare(stream.bytes, expected, actual);
            if (trial < 8) {
                // Truncated input is zero-padded, not rejected. Include every
                // byte cut of several streams with nonzero carried state.
                for (std::size_t length = 0; length < stream.bytes.size(); ++length) {
                    const std::vector<Byte> prefix(stream.bytes.begin(),
                                                   stream.bytes.begin() + length);
                    compare(prefix, expected, actual);
                }
            }
        }
        for (unsigned length = 0; length < 97; ++length) {
            compare(std::vector<Byte>(length, 0), expected, actual);
            compare(std::vector<Byte>(length, 255), expected, actual);
            std::vector<Byte> arbitrary(length);
            for (auto &value : arbitrary)
                value = Byte(random_word(random));
            compare(arbitrary, expected, actual);
        }
        Stream ended;
        ended.end();
        ended.bytes.insert(ended.bytes.end(), {255, 128, 17, 42});
        const auto before = expected;
        compare(ended.bytes, expected, actual);
        check(expected == before, "LZSS decoded bytes after end marker");
        std::cout << "LZSS scalar differential checks passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
