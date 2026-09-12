#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
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
// This checks the repository's inline path-link convention, not arbitrary
// Markdown or remote URLs. Anchors are deliberately outside this test's scope.
std::vector<std::string> inspect(const std::string &text, const fs::path &document) {
    static const std::regex link(R"(\[[^\]\n]*\]\(([^)\n]+)\))");
    std::vector<std::string> errors;
    std::istringstream lines(text);
    std::string line, heading;
    unsigned level = 0, line_number = 0;
    bool content = true, fenced = false;
    while (std::getline(lines, line)) {
        ++line_number;
        line = trim(line);
        if (line.compare(0, 3, "```") == 0 || line.compare(0, 3, "~~~") == 0) {
            fenced = !fenced;
            content = true;
            continue;
        }
        if (fenced || line.empty())
            continue;
        const auto hashes = line.find_first_not_of('#');
        if (hashes > 0 && hashes <= 6 && line[hashes] == ' ') {
            if (hashes <= level && !content)
                errors.push_back("empty leaf heading: " + heading);
            level = unsigned(hashes);
            heading = line;
            content = false;
        } else {
            content = true;
        }
        for (auto it = std::sregex_iterator(line.begin(), line.end(), link);
             it != std::sregex_iterator(); ++it) {
            auto target = trim((*it)[1].str());
            if (!target.empty() && target.front() == '<' && target.back() == '>')
                target = target.substr(1, target.size() - 2);
            if (target.find("://") != std::string::npos || target.compare(0, 7, "mailto:") == 0 ||
                target.compare(0, 2, "//") == 0)
                continue;
            target = target.substr(0, target.find('#'));
            if (target.empty())
                continue;
            const fs::path relative(target);
            if (relative.is_absolute()) {
                errors.push_back("non-portable absolute link at line " +
                                 std::to_string(line_number) + ": " + target);
                continue;
            }
            if (!fs::exists((document.parent_path() / relative).lexically_normal()))
                errors.push_back("missing local link at line " + std::to_string(line_number) +
                                 ": " + target);
        }
    }
    if (!content)
        errors.push_back("empty leaf heading: " + heading);
    return errors;
}
// Only named nonnegative integer fields in generated summary JSON are read.
// Never round large state digests through floating point or rewrite the reports.
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
    require(inspect("```md\n[example](missing-file)\n```\n", document).empty(),
            "checker interpreted a fenced example as a live link");
    require(inspect("[local](docs/STATUS.md) [remote](https://example.invalid/)", document).empty(),
            "checker rejected a valid path or inspected a remote URL");
    require(inspect("[broken](missing-documentation-test-target.md)", document).size() == 1,
            "checker missed a broken local link");
}
} // namespace

int main(int argc, char **argv) try {
    require(argc == 3, "usage: documentation_tests source-root core-test-count");
    const fs::path root = argv[1];
    const std::string core_tests = argv[2];
    check_self(root);
    std::vector<fs::path> documents = {root / "README.md", root / "AGENTS.md",
                                       root / "reports/native/README.md"};
    for (const auto &entry : fs::recursive_directory_iterator(root / "docs"))
        if (entry.is_regular_file() && entry.path().extension() == ".md")
            documents.push_back(entry.path());
    std::sort(documents.begin(), documents.end());
    unsigned failures = 0;
    for (const auto &document : documents)
        for (const auto &error : inspect(read(document), document)) {
            std::cerr << document.lexically_relative(root).string() << ": " << error << '\n';
            ++failures;
        }
    require(failures == 0, "maintained documentation links/headings failed");
    const auto status = read(root / "docs/STATUS.md");
    const auto summary = root / "reports/native/summary.json";
    const auto solutions = count(summary, "offline_full_spell_solutions_verified");
    contains(status, "| Indexed original spell IDs | " + count(summary, "unique_spell_ids") + " |",
             "STATUS.md");
    contains(status, "| Indexed spell-start occurrences | " + count(summary, "spell_sites") + " |",
             "STATUS.md");
    contains(status, "| Complete offline spell solutions | " + solutions + " |", "STATUS.md");
    require(solutions ==
                count(root / "reports/native/motion_summary.json", "complete_spell_solutions"),
            "audit and particle reports disagree on complete spell solutions");
    require(
        count(root / "reports/native/first_spell_summary.json", "acceptance_gates_passed") == "0" &&
            count(root / "reports/native/first_spell_summary.json", "complete_spell_solutions") ==
                "0",
        "first entry-prefix evidence changed; review its completion contract and documentation");
    contains(read(root / "README.md"), "Complete offline spell solutions: " + solutions + ".",
             "README.md");
    contains(status, "There are " + core_tests + " core CTests", "STATUS.md");
    contains(read(root / "docs/VALIDATION.md"), "\nCore CTest cases: " + core_tests + "\n",
             "VALIDATION.md");
    std::cout << "maintained Markdown files=" << documents.size()
              << " local links/headings and selected status counts: passed\n";
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
