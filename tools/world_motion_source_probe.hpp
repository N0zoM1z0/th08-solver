#pragma once
#include <filesystem>
#include <string>

// Requires the shared timer/math/RNG and enemy_motion_reference definitions.
std::string world_motion_reference(const std::filesystem::path &repo);
