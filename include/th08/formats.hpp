#pragma once
#include "resources.hpp"
#include <array>

namespace th08::resources {
struct ShtHeader {
    std::uint16_t reserved;
    float initial_bombs;
    std::int32_t deathbomb_frames;
    float hurtbox_size, graze_size, item_auto_collect_speed, item_collection_size, point_value_line;
    std::uint32_t reserved20;
    float normal_axis, focused_axis, normal_diagonal, focused_diagonal, item_speed;
};
struct ShotDescriptor {
    std::uint32_t offset;
    std::int16_t interval, frame;
    std::array<float, 2> position, hitbox;
    float angle, speed;
    std::int16_t damage, gauge_behavior, option, shot_type, animation, sound;
    std::array<std::uint32_t, 4> callbacks;
};
struct PowerLevel {
    std::int32_t minimum_power;
    std::uint32_t offset, terminal, first, count;
};
struct Sht {
    ShtHeader header;
    std::vector<PowerLevel> levels;
    // Occurrences are retained even when two levels reference the same bytes.
    std::vector<ShotDescriptor> shots;
};
Sht parse_sht(View data);

struct AnmEntry {
    std::uint32_t offset, end, sprites, scripts;
};
struct Sprite {
    std::uint32_t entry, raw_id, offset;
    float x, y, width, height;
};
struct AnmInstruction {
    std::uint32_t offset;
    std::int16_t opcode, time;
    std::uint16_t size, mask;
};
struct AnmScript {
    std::uint32_t entry, raw_id, offset, first, count;
};
struct Anm {
    std::vector<AnmEntry> entries;
    std::vector<Sprite> sprites;
    std::vector<AnmScript> scripts;
    std::vector<AnmInstruction> instructions;
};
Anm parse_anm(View data);

struct StageQuad {
    std::uint32_t offset;
    std::int16_t type, size, anm_script, vm;
    std::array<float, 7> geometry{};
};
struct StageObject {
    std::uint32_t offset, first, count;
    std::int16_t id;
    std::int8_t z_level;
    std::uint8_t flags;
    std::array<float, 3> position, size;
};
struct StageInstance {
    std::uint32_t offset;
    std::int16_t object;
    std::array<float, 3> position;
};
struct StageInstruction {
    std::uint32_t offset;
    std::int32_t time;
    std::int16_t opcode, payload_size;
    std::array<std::uint32_t, 3> words;
};
struct Std {
    std::uint16_t declared_quads;
    std::vector<StageObject> objects;
    std::vector<StageQuad> quads;
    std::vector<StageInstance> instances;
    std::vector<StageInstruction> instructions;
};
Std parse_std(View data);
} // namespace th08::resources
