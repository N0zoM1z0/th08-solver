#pragma once
#include <cstddef>
#include <cstdint>

namespace th08::headless {
constexpr unsigned lzss_dictionary_size = 8192;

// MSB-first fields, fetching only the bytes required for the requested field.
// Preserve the modern reader's zero padding: EOF does not advance the cursor.
// At field boundaries fewer than eight unread bits remain, so a 32-bit unsigned
// reservoir suffices for the largest (13-bit) field without speculative reads.
class LzssBitReader {
  public:
    LzssBitReader(const std::uint8_t *input, int size) : input_(input), size_(size) {}
    unsigned take(unsigned count) {
        while (available_ < count) {
            const unsigned byte = position_ < size_ ? input_[position_++] : 0;
            reservoir_ = (reservoir_ << 8) | byte;
            available_ += 8;
        }
        available_ -= count;
        return (reservoir_ >> available_) & ((1u << count) - 1);
    }
    void finish_byte() {
        if (available_)
            take(available_);
    }

  private:
    const std::uint8_t *input_;
    int size_, position_ = 0;
    std::uint32_t reservoir_ = 0;
    unsigned available_ = 0;
};

// Same caller contract as native Lzss::Decode: output capacity is supplied by
// the owner, not validated by this decoder. Archive callers use disjoint input/
// output buffers. The dictionary is caller-owned mutable state and deliberately
// survives calls, including interaction with the native encoder. Never zero it.
// Dictionary-to-dictionary overlap must remain sequential (no memcpy shortcut).
inline std::size_t decode_lzss_fields(const std::uint8_t *input, int input_size,
                                      std::uint8_t *output,
                                      std::uint8_t (&dictionary)[lzss_dictionary_size]) {
    LzssBitReader bits(input, input_size);
    auto *cursor = output;
    unsigned head = 1;
    for (;;) {
        if (bits.take(1)) {
            const auto value = std::uint8_t(bits.take(8));
            *cursor++ = value;
            dictionary[head] = value;
            head = (head + 1) & (lzss_dictionary_size - 1);
        } else {
            const auto offset = bits.take(13);
            if (offset == 0)
                break;
            const auto length = bits.take(4) + 3;
            for (unsigned i = 0; i < length; ++i) {
                const auto value = dictionary[(offset + i) & (lzss_dictionary_size - 1)];
                *cursor++ = value;
                dictionary[head] = value;
                head = (head + 1) & (lzss_dictionary_size - 1);
            }
        }
    }
    bits.finish_byte();
    return std::size_t(cursor - output);
}
} // namespace th08::headless
