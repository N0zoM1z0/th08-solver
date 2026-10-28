#include "th08/phase_archive.hpp"
#include <charconv>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {
std::uint64_t number(const std::string &s) {
    std::uint64_t n = 0;
    const auto r = std::from_chars(s.data(), s.data() + s.size(), n);
    if (s.empty() || r.ec != std::errc() || r.ptr != s.data() + s.size())
        throw std::runtime_error("invalid_archive_integer");
    return n;
}
} // namespace
int main(int argc, char **argv) try {
    if (argc != 5)
        throw std::runtime_error("usage: phase_frontier INPUT_TSV WIDTH GENERATION OUTPUT_TSV");
    const auto width = number(argv[2]), generation = number(argv[3]);
    if (width > 64)
        throw std::runtime_error("invalid_archive_width");
    std::ifstream in(argv[1]);
    std::string line;
    if (!std::getline(in, line) ||
        line != "id\tboundary\tfamily\trequest\tcheckpoint\tscore\tcompleted\tno_miss\tbombs")
        throw std::runtime_error("invalid_archive_header");
    std::vector<th08::phase::Candidate> rows;
    while (std::getline(in, line)) {
        if (line.empty() || line.back() == '\t' || line.size() > 1024 || rows.size() >= 4096)
            throw std::runtime_error("invalid_archive_row_size");
        std::istringstream stream(line);
        std::vector<std::string> fields;
        std::string field;
        while (std::getline(stream, field, '\t'))
            fields.push_back(field);
        if (fields.size() != 9)
            throw std::runtime_error("invalid_archive_row");
        const auto complete = number(fields[6]), nm = number(fields[7]), bombs = number(fields[8]);
        if (complete > 1 || nm > 1 || bombs > 400000)
            throw std::runtime_error("invalid_archive_outcome");
        rows.push_back({fields[0], fields[1], fields[2], fields[3], fields[4], number(fields[5]),
                        bool(complete), bool(nm), unsigned(bombs)});
    }
    if (!in.eof())
        throw std::runtime_error("archive_read_failed");
    const auto selected = th08::phase::frontier(std::move(rows), unsigned(width), generation);
    // Parse and select completely before opening the output: rejected evidence
    // must not overwrite an existing frontier or look like a valid empty beam.
    std::ofstream out(argv[4]);
    if (!out)
        throw std::runtime_error("archive_output_unwritable");
    out << "id\tcheckpoint\tscore\n";
    for (const auto &c : selected)
        out << c.id << '\t' << c.checkpoint_key << '\t' << c.score << '\n';
    if (!out)
        throw std::runtime_error("archive_write_failed");
    std::cout << selected.size() << " selected\n";
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
}
