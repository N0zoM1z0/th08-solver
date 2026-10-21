// Synthetic process-protocol fixture: no native runtime or private game assets.
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char **argv) {
    if (argc != 4)
        return 2;
    std::ifstream replay(argv[2]);
    std::string mode;
    replay >> mode;
    if (mode == "timeout")
        std::this_thread::sleep_for(std::chrono::seconds(60));
    if (mode != "missing") {
        std::ofstream rows(argv[3], std::ios::binary);
        std::array<char, 100> row{};
        row[0] = 1; // Little-endian calculation index of the one known row.
        if (mode == "hash")
            row[4] = 1;
        if (mode != "empty")
            rows.write(row.data(), mode == "truncated" ? 99 : 100);
        if (mode == "fast-forward") {
            row[0] = 2;
            rows.write(row.data(), 100);
        }
    }
    if (mode == "duplicate-json")
        std::cout << "{\"rows\":1,\"rows\":1,\"finished\":true,\"natural\":true,\"steps\":1}\n";
    else if (mode == "malformed-json")
        std::cout << "{\"rows\":1,\"finished\":true";
    else
        std::cout << "{\"rows\":" << (mode == "fast-forward" ? 2 : 1) << ",\"finished\":"
                  << (mode == "unfinished" || mode == "budget" ? "false" : "true")
                  << ",\"natural\":" << (mode == "unnatural" || mode == "budget" ? "false" : "true")
                  << ",\"steps\":"
                  << (mode == "budget"       ? 400000
                      : mode == "overbudget" ? 400001
                                             : 1)
                  << ",\"runtime_error\":" << (mode == "runtime-error" ? "true" : "false")
                  << ",\"io_error\":false}\n";
    return mode == "exit-error" ? 1 : 0;
}
