#pragma once
#include <filesystem>
#include <string>

// Generate a source-backed scalar test adapter, not a replacement game VM.
std::string ecl_reference(const std::filesystem::path &repo);
