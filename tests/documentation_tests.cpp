#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;
namespace {
void require(bool valid, const std::string &message) {
    if (!valid)
        throw std::runtime_error(message);
}
std::string read(const fs::path &path) {
    std::ifstream input(path);
    require(bool(input), "cannot read " + path.string());
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}
std::string trim(const std::string &text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    return text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
}
struct MarkdownState {
    char fence = 0;
    std::size_t fence_width = 0, code_width = 0;
};
// Ignore code for prose checks, including inline spans that wrap onto another
// line. A different fence character or a short closer cannot end a code block.
std::optional<std::string> prose(const std::string &line, MarkdownState &state) {
    const auto text = trim(line);
    if (!state.code_width && !text.empty() && (text[0] == '`' || text[0] == '~')) {
        const auto width = text.find_first_not_of(text[0]);
        const auto count = width == std::string::npos ? text.size() : width;
        if (count >= 3) {
            if (!state.fence) {
                state.fence = text[0];
                state.fence_width = count;
            } else if (state.fence == text[0] && count >= state.fence_width &&
                       trim(text.substr(count)).empty()) {
                state.fence = 0;
            }
            return std::nullopt;
        }
    }
    if (state.fence)
        return std::nullopt;
    std::string result;
    for (std::size_t i = 0; i < line.size();) {
        if (line[i] == '`') {
            std::size_t end = i;
            while (end < line.size() && line[end] == '`')
                ++end;
            const auto width = end - i;
            if (!state.code_width)
                state.code_width = width;
            else if (state.code_width == width)
                state.code_width = 0;
            result.append(width, ' ');
            i = end;
        } else {
            result += state.code_width ? ' ' : line[i];
            ++i;
        }
    }
    return result;
}
const std::regex &links() {
    static const std::regex pattern(R"(\[([^\]\n]*)\]\(([^)\n]+)\))");
    return pattern;
}
// Repository headings use the ASCII GitHub slug convention. Keep explicit
// aliases and repeated-heading suffixes so reorganizing prose cannot hide a
// stale local fragment behind a still-existing filename.
std::string slug(std::string text) {
    text = std::regex_replace(text, links(), "$1");
    text = std::regex_replace(text, std::regex("<[^>]+>"), "");
    std::string result;
    for (unsigned char c : trim(text)) {
        if (std::isalnum(c) || c == '_' || c == '-')
            result += char(std::tolower(c));
        else if (std::isspace(c))
            result += '-';
    }
    return result;
}
const std::set<std::string> &anchors(const fs::path &path) {
    static std::map<fs::path, std::set<std::string>> cache;
    const auto key = path.lexically_normal();
    auto found = cache.find(key);
    if (found != cache.end())
        return found->second;
    std::set<std::string> ids;
    std::map<std::string, unsigned> counts;
    std::istringstream input(read(key));
    MarkdownState state;
    std::string line;
    const std::regex explicit_id(R"(<a\s+(?:id|name)=["']([^"']+)["'])");
    while (std::getline(input, line)) {
        const auto visible = prose(line, state);
        if (!visible || trim(*visible).empty())
            continue;
        for (auto it = std::sregex_iterator(line.begin(), line.end(), explicit_id);
             it != std::sregex_iterator(); ++it)
            ids.insert((*it)[1].str());
        const auto text = trim(line);
        const auto hashes = text.find_first_not_of('#');
        if (hashes > 0 && hashes <= 6 && text[hashes] == ' ') {
            const auto base = slug(text.substr(hashes + 1));
            const auto occurrence = counts[base]++;
            ids.insert(base + (occurrence ? "-" + std::to_string(occurrence) : ""));
        }
    }
    return cache.emplace(key, std::move(ids)).first->second;
}
std::vector<std::string> inspect(const std::string &text, const fs::path &document) {
    static const std::regex month(
        R"(\b(January|February|March|April|May|June|July|August|September|October|November|December)[0-9])");
    static const std::regex joined(
        R"(\b(the|all|and|is|in|before|after|with|above|below|task|tasks|card|cards|phase|phases|stage|stages|seed|seeds|entry|first|only|both|through|uses|yields|captures|checks)[0-9])",
        std::regex::icase);
    static const std::regex units(R"([0-9](GiB|MiB|KiB|ms|ns|px|bytes)\b)");
    static const std::regex url(R"((https?://|mailto:)[^\s<>]+)");
    static const std::regex filename(
        R"([A-Za-z0-9_.-]+\.(rpy|md|cpp|hpp|json|jsonl|tsv|csv|cjs|exe|dll|so)\b)");
    std::vector<std::string> errors;
    std::istringstream lines(text);
    std::string line, heading;
    unsigned level = 0, line_number = 0;
    bool content = true;
    MarkdownState state;
    while (std::getline(lines, line)) {
        ++line_number;
        const auto visible = prose(line, state);
        if (!visible) {
            content = true;
            continue;
        }
        const auto body = trim(*visible);
        if (body.empty())
            continue;
        const auto hashes = body.find_first_not_of('#');
        if (hashes > 0 && hashes <= 6 && body[hashes] == ' ') {
            if (hashes <= level && !content)
                errors.push_back("empty leaf heading: " + heading);
            level = unsigned(hashes);
            heading = body;
            content = false;
        } else {
            content = true;
        }
        const auto where = " at line " + std::to_string(line_number) + ": ";
        for (auto it = std::sregex_iterator(body.begin(), body.end(), links());
             it != std::sregex_iterator(); ++it) {
            auto target = trim((*it)[2].str());
            if (!target.empty() && target.front() == '<' && target.back() == '>')
                target = target.substr(1, target.size() - 2);
            if (target.find("://") != std::string::npos || target.compare(0, 7, "mailto:") == 0 ||
                target.compare(0, 2, "//") == 0)
                continue;
            const auto hash = target.find('#');
            const auto relative = fs::path(target.substr(0, hash));
            if (relative.is_absolute()) {
                errors.push_back("non-portable absolute link" + where + target);
                continue;
            }
            const auto destination = relative.empty()
                                         ? document
                                         : (document.parent_path() / relative).lexically_normal();
            if (!fs::exists(destination)) {
                errors.push_back("missing local link" + where + target);
            } else if (hash != std::string::npos && hash + 1 < target.size() &&
                       destination.extension() == ".md" &&
                       !anchors(destination).count(target.substr(hash + 1))) {
                errors.push_back("missing Markdown anchor" + where + target);
            }
        }
        auto plain = std::regex_replace(body, links(), "$1");
        plain = std::regex_replace(plain, url, "");
        plain = std::regex_replace(plain, filename, "");
        std::smatch match;
        for (const auto *pattern : {&month, &joined, &units})
            if (std::regex_search(plain, match, *pattern))
                errors.push_back("missing prose spacing" + where + match.str());
    }
    if (!content)
        errors.push_back("empty leaf heading: " + heading);
    return errors;
}
// Read named integer fields without rounding digests or rewriting frozen data.
std::string count(const fs::path &path, const std::string &key) {
    const auto text = read(path);
    const std::regex field("\"" + key + "\"\\s*:\\s*([0-9]+)(?=[,}\\s])");
    std::smatch match;
    require(std::regex_search(text, match, field), "missing count " + key + " in " + path.string());
    return match[1];
}
void contains(const std::string &text, const std::string &expected, const char *document) {
    require(text.find(expected) != std::string::npos,
            std::string(document) + " needs current declaration: " + expected);
}
void check_self(const fs::path &root) {
    const auto document = root / "README.md";
    require(inspect("# Parent\n## Child\nText\n", document).empty(),
            "checker rejected a parent heading with a child");
    require(inspect("# Parent\n## Empty\n## Next\nText\n", document).size() == 1,
            "checker missed an empty leaf heading");
    require(inspect("# Parent\nText\n## Empty\n", document).size() == 1,
            "checker missed a trailing empty heading");
    require(inspect("```md\n~~~\n[example](missing-file) October27 is130\n```\n", document).empty(),
            "checker interpreted a fenced example or a different closer as prose");
    require(inspect("An inline `example\nOctober27 is130` is code.\n", document).empty(),
            "checker interpreted wrapped inline code as prose");
    require(inspect("[local](docs/STATUS.md#verified-results) "
                    "[remote](https://example.invalid/October27)\nTH08 SDL2 x86-64 L2 4 GiB\n",
                    document)
                .empty(),
            "checker rejected a valid link, identifier or spaced unit");
    require(inspect("[story-seed9098.rpy](README.md)\n", document).empty(),
            "checker interpreted a literal replay filename as prose");
    require(inspect("[broken](missing-documentation-test-target.md)\n", document).size() == 1,
            "checker missed a broken local link");
    require(
        inspect("[stale](docs/STATUS.md#missing-documentation-test-anchor)\n", document).size() ==
            1,
        "checker missed a stale Markdown fragment");
    require(inspect("October27\nis130,365,220\n4GiB\n", document).size() == 3,
            "checker missed the reported prose regressions");
}
std::vector<fs::path> documents(const fs::path &root) {
    std::vector<fs::path> result = {
        root / "README.md", root / "AGENTS.md", root / "reports/native/README.md",
        root / "reports/native/archive/README.md", root / "third_party/th08/README.md"};
    for (const auto &directory : {root / "docs", root / "models"})
        for (const auto &entry : fs::recursive_directory_iterator(directory))
            if (entry.is_regular_file() && entry.path().extension() == ".md")
                result.push_back(entry.path());
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}
} // namespace

int main(int argc, char **argv) try {
    require(argc == 3, "usage: documentation_tests source-root core-test-count");
    const fs::path root = argv[1];
    check_self(root);
    const auto maintained = documents(root);
    unsigned failures = 0;
    for (const auto &document : maintained)
        for (const auto &error : inspect(read(document), document)) {
            std::cerr << document.lexically_relative(root).string() << ": " << error << '\n';
            ++failures;
        }
    require(failures == 0, "maintained documentation links/headings/spacing failed");
    const auto archive = read(root / "reports/native/archive/README.md");
    const auto summary = root / "reports/native/archive/summary.json";
    const auto solutions = count(summary, "offline_full_spell_solutions_verified");
    contains(archive, "| Indexed original spell IDs | " + count(summary, "unique_spell_ids") + " |",
             "archive/README.md");
    contains(archive, "| Indexed spell-start occurrences | " + count(summary, "spell_sites") + " |",
             "archive/README.md");
    contains(archive, "| Complete offline spell solutions | " + solutions + " |",
             "archive/README.md");
    require(solutions == count(root / "reports/native/archive/motion_summary.json",
                               "complete_spell_solutions"),
            "audit and particle reports disagree on complete spell solutions");
    require(
        count(root / "reports/native/archive/first_spell_summary.json",
              "acceptance_gates_passed") == "0" &&
            count(root / "reports/native/archive/first_spell_summary.json",
                  "complete_spell_solutions") == "0",
        "first entry-prefix evidence changed; review its completion contract and documentation");
    contains(read(root / "docs/VALIDATION.md"),
             "\nCore CTest cases: " + std::string(argv[2]) + "\n", "VALIDATION.md");
    std::cout << "maintained Markdown files=" << maintained.size()
              << " links/anchors/headings/prose spacing and catalog counts: passed\n";
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
