#pragma once
#include <filesystem>
#include <string>

namespace th08::source_probe {
// Verify identity before a generator inspects source text. The source checkout
// is read-only; generated reference bodies belong exclusively in build outputs.
std::string read_pinned_source(const std::filesystem::path &path, const char *expected_sha256);

// Extract an unchanged brace-balanced body from an already hash-checked file.
// This is deliberately not a C++ parser: each pinned signature/body is reviewed
// for the simple brace scan. A new reference needs that review before use.
std::string extract_function(const std::string &text, const std::string &signature);
} // namespace th08::source_probe
