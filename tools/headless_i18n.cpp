#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

// Only the upstream three-column translation table is accepted. English log
// strings avoid a build-time Python/Shift-JIS dependency; DAT text is untouched.
int main(int argc, char **argv) {
    try {
        if (argc != 3)
            throw std::runtime_error("expected translation CSV and output header");
        std::ifstream input(argv[1]);
        std::ofstream output(argv[2]);
        if (!input || !output)
            throw std::runtime_error("cannot open translation input/output");
        output
            << "// Generated English diagnostics; game resources remain unchanged.\n#pragma once\n";
        std::string line;
        std::getline(input, line);
        if (line != "identifier,jp,en")
            throw std::runtime_error("unexpected translation table header");
        while (std::getline(input, line)) {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            std::array<std::string, 3> fields;
            std::size_t field = 0;
            bool quoted = false;
            for (std::size_t i = 0; i < line.size(); ++i) {
                const char c = line[i];
                if (c == '"') {
                    if (quoted && i + 1 < line.size() && line[i + 1] == '"') {
                        fields.at(field) += '"';
                        ++i;
                    } else
                        quoted = !quoted;
                } else if (c == ',' && !quoted)
                    ++field;
                else
                    fields.at(field) += c;
            }
            if (quoted || field != 2 || fields[0].empty())
                throw std::runtime_error("malformed translation row");
            output << "#define TH_" << fields[0] << " \"";
            for (const char c : fields[2]) {
                if (c == '"')
                    output << '\\';
                output << c;
            }
            output << "\"\n";
        }
        if (!input.eof() || !output)
            throw std::runtime_error("translation I/O failure");
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
