#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace th08::shadow {
// Versioned little-endian values cross the 32/64-bit boundary. This projection
// detects divergence; it is neither a serialized world nor a physics oracle.
enum class Field : std::size_t {
    FrameLow,
    FrameHigh,
    CalculationLow,
    CalculationHigh,
    Stage,
    Spell,
    SpellActive,
    X,
    Y,
    Deaths,
    Lives,
    PlayerState,
    Bullets,
    Enemies,
    RngSeed,
    RngDraws,
    Score,
    Graze,
    Gauge,
    HurtHalfX,
    HurtHalfY,
    LatchedInput,
    SampledInput,
    Youkai,
    RetryMenu,
    StageComplete,
    RouteSelection,
    FinalRoute,
    ActorLow,
    ActorHigh,
    BulletLow,
    BulletHigh,
    LaserLow,
    LaserHigh,
    BombsUsed,
    Power,
    Rank,
    PointItems,
    Orbs,
    Clock,
    PointValue,
    Count
};
inline constexpr std::array<const char *, std::size_t(Field::Count)> field_names{
    "frame_low",
    "frame_high",
    "calculation_low",
    "calculation_high",
    "stage",
    "spell",
    "spell_active",
    "x",
    "y",
    "deaths",
    "lives",
    "player_state",
    "bullets",
    "enemies",
    "rng_seed",
    "rng_draws",
    "score",
    "graze",
    "gauge",
    "hurt_half_x",
    "hurt_half_y",
    "latched_input",
    "sampled_input",
    "youkai",
    "retry_menu",
    "stage_complete",
    "route_selection",
    "final_route",
    "actor_digest_low",
    "actor_digest_high",
    "bullet_digest_low",
    "bullet_digest_high",
    "laser_digest_low",
    "laser_digest_high",
    "bombs_used",
    "power",
    "rank",
    "point_items",
    "orbs",
    "clock",
    "point_value"};
struct Projection {
    std::array<std::uint32_t, std::size_t(Field::Count)> values{};
    std::uint32_t &operator[](Field field) {
        return values[std::size_t(field)];
    }
    std::uint32_t operator[](Field field) const {
        return values[std::size_t(field)];
    }
};
enum class Kind : std::uint32_t { State = 1, Advance = 2, Stop = 3, Failure = 4 };
struct Packet {
    Kind kind = Kind::State;
    std::uint64_t sequence = 0, epoch = 0;
    std::uint32_t input = 0, status = 0;
    Projection state;
};
inline constexpr std::uint32_t magic = 0x53483038, version = 1;
inline constexpr std::size_t header_words = 10;
using Bytes = std::array<std::uint8_t, 4 * (header_words + std::size_t(Field::Count))>;
inline void input_value(std::uint32_t input) {
    if (input & ~0x10f5u)
        throw std::runtime_error("unsupported_shadow_input");
}
inline Bytes encode(const Packet &packet) {
    input_value(packet.input);
    Bytes bytes{};
    const auto word = [&](std::size_t index, std::uint32_t value) {
        for (unsigned byte = 0; byte < 4; ++byte)
            bytes[index * 4 + byte] = std::uint8_t(value >> (byte * 8));
    };
    word(0, magic);
    word(1, version);
    word(2, std::uint32_t(packet.kind));
    word(3, std::uint32_t(bytes.size()));
    word(4, std::uint32_t(packet.sequence));
    word(5, std::uint32_t(packet.sequence >> 32));
    word(6, std::uint32_t(packet.epoch));
    word(7, std::uint32_t(packet.epoch >> 32));
    word(8, packet.input);
    word(9, packet.status);
    for (std::size_t i = 0; i < packet.state.values.size(); ++i)
        word(header_words + i, packet.state.values[i]);
    return bytes;
}
inline Packet decode(const void *data, std::size_t size) {
    if (size != Bytes{}.size())
        throw std::runtime_error("incomplete_shadow_packet");
    const auto *bytes = static_cast<const std::uint8_t *>(data);
    const auto word = [&](std::size_t index) {
        std::uint32_t value = 0;
        for (unsigned byte = 0; byte < 4; ++byte)
            value |= std::uint32_t(bytes[index * 4 + byte]) << (byte * 8);
        return value;
    };
    if (word(0) != magic || word(1) != version || word(3) != size || word(2) < 1 || word(2) > 4)
        throw std::runtime_error("incompatible_shadow_packet");
    Packet packet;
    packet.kind = Kind(word(2));
    packet.sequence = word(4) | (std::uint64_t(word(5)) << 32);
    packet.epoch = word(6) | (std::uint64_t(word(7)) << 32);
    packet.input = word(8);
    packet.status = word(9);
    input_value(packet.input);
    for (std::size_t i = 0; i < packet.state.values.size(); ++i)
        packet.state.values[i] = word(header_words + i);
    return packet;
}
inline const char *difference(const Packet &owner, const Packet &follower) {
    if (owner.kind != Kind::State || follower.kind != Kind::State || owner.status ||
        follower.status)
        return "packet_status";
    if (owner.sequence != follower.sequence)
        return "sequence";
    if (owner.epoch != follower.epoch)
        return "epoch";
    if (owner.input != follower.input)
        return "actual_input";
    for (std::size_t i = 0; i < owner.state.values.size(); ++i)
        if (owner.state.values[i] != follower.state.values[i])
            return field_names[i];
    return nullptr;
}
// Input receipts advance exactly once. Epoch changes are observed stage changes,
// including control-only handoffs; an ordinary frame may not reset any history.
inline void advance_receipt(Packet &receipt, std::uint32_t input, const Projection &after) {
    input_value(input);
    receipt.epoch += receipt.state[Field::Stage] != after[Field::Stage];
    ++receipt.sequence;
    receipt.input = input;
    receipt.state = after;
}
} // namespace th08::shadow
