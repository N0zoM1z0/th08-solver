#include <stdexcept>
#include <th08/effect_animation.hpp>
#include <th08/kinematics.hpp>

namespace th08::effect {
Effect51Animation compile_effect51_animation(resources::View bytes) {
    if (resources::sha256(bytes) !=
        "203902359e0d9f741356f48f5ea451f3ec5e95750542861012483a6d80483cb6")
        throw std::invalid_argument("effect51 requires pinned enemy.anm");
    const auto decoded = resources::parse_anm(bytes);
    if (decoded.scripts.size() <= 73 || decoded.sprites.size() <= 121)
        throw std::invalid_argument("effect51 resource indices missing");
    const auto &script = decoded.scripts[73];
    const animation::control::Program program(bytes, decoded, 73);
    if (script.raw_id != 73 || script.entry != 0 || program.code.size() != 4 ||
        program.code[0].opcode != 13 || program.code[0].time != 0 || program.code[0].mask ||
        program.code[1].opcode != 3 || program.code[1].time != 0 || program.code[1].mask ||
        program.code[1].words[0] != 121 || program.code[2].opcode != 2 ||
        program.code[2].time != 30000 || program.code[3].opcode != -1)
        throw std::invalid_argument("effect51 script shape mismatch");
    Effect51Animation result{};
    // SpawnEffect first zeroes the entire Effect, then Initialize zeroes its
    // ANM base and installs white color/flags7. Sprite sets visible; opcode13
    // sets updateRotation. Neither changes the already-set flags7.
    result.fields.primary = {255, 255, 255, 255};
    result.fields.flags = 7;
    const auto payload = decoded.instructions[script.first].offset + 8;
    result.angular_velocity = {resources::f32(bytes, payload), resources::f32(bytes, payload + 4),
                               resources::f32(bytes, payload + 8)};
    // ExecuteScript applies angular velocity in its time-zero tail as well.
    auto rotate = [](float velocity) {
        return velocity != 0.0f ? kinematics::normalize_angle(0.0f, velocity) : 0.0f;
    };
    result.rotation = {rotate(result.angular_velocity.x), rotate(result.angular_velocity.y),
                       rotate(result.angular_velocity.z)};
    const auto step = animation::control::advance(program, result.control);
    if (step.status != animation::control::Status::advanced || result.control.pc != 2 ||
        result.control.sprite != 121 || result.control.time.current != 1)
        throw std::runtime_error("effect51 time-zero control mismatch");
    // The pinned loader uses directory indices; do not substitute raw sprite ID.
    const auto &sprite = decoded.sprites[121];
    result.sprite_width = sprite.width;
    result.sprite_height = sprite.height;
    result.unit_rate_script73 = true;
    return result;
}
} // namespace th08::effect
