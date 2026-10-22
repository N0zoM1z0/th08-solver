#include "th08/plan_matrix.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char **argv) {
    try {
        std::string output;
        int seed = -1;
        if (argc != 5)
            throw std::invalid_argument("usage: th08_plan_matrix --seed UINT16 --output FILE");
        for (int i = 1; i < argc; i += 2) {
            const std::string key = argv[i];
            if (key == "--seed")
                seed = th08::plan::integer<int>(argv[i + 1], key);
            else if (key == "--output")
                output = argv[i + 1];
            else
                throw std::invalid_argument("unknown matrix option");
        }
        th08::plan::range(seed >= 0 && seed <= 65535 && !output.empty(),
                          "a uint16 seed and output file are required");
        std::ofstream out(output);
        out.exceptions(std::ios::failbit | std::ios::badbit);
        out << "{\"schema_version\":1,\"seed\":" << seed << ",\"cases\":[\n";
        bool first = true;
        for (const auto &entry : th08::plan::full_run_matrix()) {
            const auto *difficulty = th08::plan::difficulty_names[entry.difficulty];
            const char *route = entry.route == th08::plan::FinalRoute::A   ? "a"
                                : entry.route == th08::plan::FinalRoute::B ? "b"
                                                                           : "natural";
            out << (first ? "" : ",\n") << "{\"id\":\"" << difficulty << "-t" << entry.shot << '-'
                << route << "-s" << seed << "\",\"shot_name\":\""
                << th08::plan::shot_names[entry.shot] << "\",\"difficulty_name\":\"" << difficulty
                << "\",\"shot\":" << entry.shot << ",\"difficulty\":" << entry.difficulty
                << ",\"seed\":" << seed << ",\"stage\":\"" << (entry.stage() == 8 ? "extra" : "1")
                << "\",\"story\":1,\"final_route\":\"" << route << "\",\"unlock_profile\":\""
                << (entry.difficulty == 4 ? "original-extra-entry" : "both-finals-clearState-2")
                << "\",\"expected_stages\":[";
            if (entry.difficulty != 4)
                out << "0,1,2," << entry.fourth_stage() << ",5,";
            out << entry.final_stage() << "]}";
            first = false;
        }
        out << "\n]}\n";
        out.close();
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "matrix configuration failed: " << error.what() << '\n';
        return 2;
    }
}
