#include "source_probe_support.hpp"
#include <stdexcept>
#include <th08/resources.hpp>

namespace th08::source_probe {
std::string read_pinned_source(const std::filesystem::path &path, const char *expected_sha256) {
    const auto bytes = resources::read_file(path);
    if (resources::sha256(resources::view(bytes)) != expected_sha256)
        throw std::runtime_error("reference source hash mismatch: " + path.string());
    return {bytes.begin(), bytes.end()};
}
std::string extract_function(const std::string &text, const std::string &signature) {
    const auto start = text.find(signature);
    if (start == std::string::npos)
        throw std::runtime_error("missing reference function");
    const auto opening = text.find('{', start);
    if (opening == std::string::npos)
        throw std::runtime_error("missing function body");
    unsigned depth = 1;
    auto end = opening + 1;
    for (; end < text.size() && depth; ++end) {
        if (text[end] == '{')
            ++depth;
        if (text[end] == '}')
            --depth;
    }
    if (depth)
        throw std::runtime_error("unterminated reference function");
    return text.substr(start, end - start);
}
} // namespace th08::source_probe
