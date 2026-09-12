#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <th08/formats.hpp>

namespace th08::resources {
namespace {
void demand(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}
float finite_float(View data, std::size_t offset) {
    const auto value = f32(data, offset);
    demand(std::isfinite(value), "non-finite resource geometry");
    return value;
}
template <std::size_t N> std::array<float, N> floats(View data, std::size_t offset) {
    std::array<float, N> result;
    for (std::size_t i = 0; i < N; ++i)
        result[i] = finite_float(data, offset + i * 4);
    return result;
}
} // namespace
Sht parse_sht(View data) {
    data.sub(0, 56);
    const auto levels = u16(data, 2);
    demand(levels > 0 && levels <= 256, "invalid SHT power-level count");
    const std::size_t header_end = 56 + std::size_t(levels) * 8;
    data.sub(0, header_end);
    Sht result;
    result.header = {u16(data, 0),           finite_float(data, 4),  i32(data, 8),
                     finite_float(data, 12), finite_float(data, 16), finite_float(data, 20),
                     finite_float(data, 24), finite_float(data, 28), u32(data, 32),
                     finite_float(data, 36), finite_float(data, 40), finite_float(data, 44),
                     finite_float(data, 48), finite_float(data, 52)};
    demand(result.header.hurtbox_size >= 0 && result.header.normal_axis > 0 &&
               result.header.focused_axis > 0 && result.header.normal_diagonal > 0 &&
               result.header.focused_diagonal > 0,
           "invalid SHT movement or hitbox parameters");
    result.levels.reserve(levels);
    for (unsigned index = 0; index < levels; ++index) {
        const auto start = u32(data, 56 + index * 8);
        demand(start >= header_end && start < data.size, "invalid SHT descriptor offset");
        const auto first = std::uint32_t(result.shots.size());
        auto cursor = start;
        for (;;) {
            const auto interval = i16(data, cursor);
            if (interval < 0)
                break;
            data.sub(cursor, 56);
            ShotDescriptor shot;
            shot.offset = cursor;
            shot.interval = interval;
            shot.frame = i16(data, cursor + 2);
            shot.position = floats<2>(data, cursor + 4);
            shot.hitbox = floats<2>(data, cursor + 12);
            shot.angle = finite_float(data, cursor + 20);
            shot.speed = finite_float(data, cursor + 24);
            shot.damage = i16(data, cursor + 28);
            shot.gauge_behavior = i16(data, cursor + 30);
            shot.option = i16(data, cursor + 32);
            shot.shot_type = i16(data, cursor + 34);
            shot.animation = i16(data, cursor + 36);
            shot.sound = i16(data, cursor + 38);
            for (unsigned i = 0; i < 4; ++i)
                shot.callbacks[i] = u32(data, cursor + 40 + i * 4);
            result.shots.push_back(shot);
            cursor += 56;
        }
        result.levels.push_back({i32(data, 60 + index * 8), start, cursor, first,
                                 std::uint32_t(result.shots.size()) - first});
    }
    return result;
}
Anm parse_anm(View data) {
    demand(data.size <= std::numeric_limits<std::uint32_t>::max(), "ANM resource too large");
    Anm result;
    std::uint32_t base = 0;
    for (;;) {
        data.sub(base, 64);
        const auto sprite_count = u32(data, base), script_count = u32(data, base + 4);
        const auto next = u32(data, base + 56);
        demand(u32(data, base + 40) == 3 && sprite_count < 10000 && script_count < 10000,
               "invalid ANM v3 entry header");
        demand(next == 0 || (next >= 64 && next < data.size - base),
               "invalid ANM next-entry offset");
        const auto end = next ? base + next : std::uint32_t(data.size);
        const auto entry = std::uint32_t(result.entries.size());
        const std::size_t directory_end =
            std::size_t(base) + 64 + sprite_count * 4 + script_count * 8;
        demand(directory_end <= end, "ANM directory crosses entry boundary");
        result.entries.push_back({base, end, sprite_count, script_count});
        for (unsigned index = 0; index < sprite_count; ++index) {
            const auto relative = u32(data, base + 64 + index * 4);
            demand(relative <= end - base && end - base - relative >= 20,
                   "invalid ANM sprite offset");
            const auto offset = base + relative;
            demand(offset >= directory_end, "ANM sprite overlaps directory");
            result.sprites.push_back(
                {entry, u32(data, offset), offset, finite_float(data, offset + 4),
                 finite_float(data, offset + 8), finite_float(data, offset + 12),
                 finite_float(data, offset + 16)});
        }
        for (unsigned index = 0; index < script_count; ++index) {
            const auto directory = base + 64 + sprite_count * 4 + index * 8;
            const auto relative = u32(data, directory + 4);
            demand(relative < end - base, "invalid ANM script offset");
            auto cursor = base + relative;
            demand(cursor >= directory_end, "ANM script overlaps directory");
            const auto start = cursor, first = std::uint32_t(result.instructions.size());
            for (;;) {
                demand(cursor <= end && end - cursor >= 8, "unterminated ANM script");
                const auto opcode = i16(data, cursor), time = i16(data, cursor + 4);
                const auto size = u16(data, cursor + 2), mask = u16(data, cursor + 6);
                if (opcode != -1)
                    demand(size >= 8 && size <= end - cursor, "invalid ANM instruction length");
                result.instructions.push_back({cursor, opcode, time, size, mask});
                if (opcode == -1)
                    break;
                cursor += size;
            }
            result.scripts.push_back({entry, u32(data, directory), start, first,
                                      std::uint32_t(result.instructions.size()) - first});
        }
        if (!next)
            break;
        base = end;
    }
    return result;
}
Std parse_std(View data) {
    data.sub(0, 0x490);
    const auto objects = i16(data, 0), quads = i16(data, 2);
    const auto instances = u32(data, 4), script = u32(data, 8);
    demand(objects >= 0 && quads >= 0 && std::size_t(0x490 + objects * 4) <= instances &&
               instances < script && script < data.size,
           "invalid STD section directory");
    Std result;
    result.declared_quads = std::uint16_t(quads);
    result.objects.reserve(std::size_t(objects));
    result.quads.reserve(std::size_t(quads));
    for (int index = 0; index < objects; ++index) {
        const auto start = u32(data, 0x490 + index * 4);
        demand(start >= unsigned(0x490 + objects * 4) && start <= instances &&
                   instances - start >= 28,
               "invalid STD object offset");
        StageObject object;
        object.offset = start;
        object.id = i16(data, start);
        object.z_level = std::int8_t(data.data[start + 2]);
        object.flags = data.data[start + 3];
        object.position = floats<3>(data, start + 4);
        object.size = floats<3>(data, start + 16);
        object.first = std::uint32_t(result.quads.size());
        auto cursor = start + 28;
        for (;;) {
            demand(cursor <= instances && instances - cursor >= 4, "unterminated STD object");
            const auto type = i16(data, cursor);
            if (type < 0)
                break;
            const auto size = i16(data, cursor + 2);
            demand((size == 28 || size == 36) && std::uint32_t(size) <= instances - cursor,
                   "invalid STD quad");
            StageQuad quad{cursor, type, size, i16(data, cursor + 4), i16(data, cursor + 6), {}};
            for (unsigned i = 0; i < unsigned(size - 8) / 4; ++i)
                quad.geometry[i] = finite_float(data, cursor + 8 + i * 4);
            result.quads.push_back(quad);
            cursor += unsigned(size);
        }
        object.count = std::uint32_t(result.quads.size()) - object.first;
        result.objects.push_back(object);
    }
    for (auto cursor = instances;; cursor += 16) {
        demand(cursor <= script && script - cursor >= 4, "unterminated STD instances");
        const auto object = i16(data, cursor);
        if (object < 0)
            break;
        demand(script - cursor >= 16 && object < objects, "invalid STD instance");
        result.instances.push_back({cursor, object, floats<3>(data, cursor + 4)});
    }
    for (auto cursor = script;; cursor += 20) {
        data.sub(cursor, 8);
        const auto time = i32(data, cursor);
        const auto opcode = i16(data, cursor + 4);
        if (time < 0 || opcode < 0)
            break;
        data.sub(cursor, 20);
        demand(opcode <= 34, "unknown STD instruction");
        result.instructions.push_back(
            {cursor,
             time,
             opcode,
             i16(data, cursor + 6),
             {u32(data, cursor + 8), u32(data, cursor + 12), u32(data, cursor + 16)}});
    }
    demand(result.quads.size() == result.declared_quads, "STD quad count mismatch");
    return result;
}
} // namespace th08::resources
