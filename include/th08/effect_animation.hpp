#pragma once
#include "effect_pool.hpp"
#include "formats.hpp"

namespace th08::effect {
// Restricted native unit-rate projection of the hash-pinned enemy.anm script73.
// Owns control, angular motion, sprite dimensions and callback-observable fields.
// No textures/matrices/rendering, manager scheduling or resource-loading side effects.
// Throws for a different resource or script shape; never treats a visual opcode as NOP.
Effect51Animation compile_effect51_animation(resources::View enemy_anm);
} // namespace th08::effect
