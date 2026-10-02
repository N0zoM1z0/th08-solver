#include <cerrno>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;
namespace {
unsigned number(const std::string &text, unsigned maximum) {
    std::size_t end = 0;
    auto value = std::stoul(text, &end);
    if (text.empty() || text.front() == '-' || end != text.size() || value > maximum)
        throw std::runtime_error("invalid numeric option: " + text);
    return unsigned(value);
}
std::string read(const fs::path &path) {
    std::ifstream input(path);
    if (!input)
        throw std::runtime_error("cannot read " + path.string());
    std::ostringstream out;
    out << input.rdbuf();
    if (input.bad())
        throw std::runtime_error("read failed: " + path.string());
    return out.str();
}
// Only the fixed scalar fields emitted by our native CLI are needed. The full
// child report is retained verbatim, including collision geometry and timings.
std::string field(const std::string &report, const char *key) {
    std::smatch match;
    const std::regex pattern(std::string("\"") + key + "\":(\"[^\"]*\"|[0-9]+)");
    if (!std::regex_search(report, match, pattern))
        throw std::runtime_error(std::string("missing child report field: ") + key);
    return match[1];
}
struct ProcessResult {
    int exit_code;
    double wall_ms;
    long rss_kib;
};
ProcessResult run(const std::vector<std::string> &args) {
    std::vector<char *> argv;
    for (const auto &arg : args)
        argv.push_back(const_cast<char *>(arg.c_str()));
    argv.push_back(nullptr);
    const auto start = std::chrono::steady_clock::now();
    const auto pid = fork();
    if (pid < 0)
        throw std::runtime_error("cannot fork native runner");
    if (!pid) {
        execv(argv[0], argv.data());
        _exit(127);
    }
    int status = 0;
    rusage usage{};
    pid_t waited;
    do {
        waited = wait4(pid, &status, 0, &usage);
    } while (waited < 0 && errno == EINTR);
    if (waited != pid || !WIFEXITED(status))
        throw std::runtime_error("native runner did not exit normally");
    return {
        WEXITSTATUS(status),
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count(),
        usage.ru_maxrss};
}
} // namespace

// Fresh processes are the branch boundary: gameplay aiming, feedback and RNG
// belong to each candidate. This tool never copies pointer-rich native globals.
int main(int argc, char **argv) {
    try {
        fs::path executable, source, directory;
        unsigned frame = 0, through = 0;
        std::vector<std::string> scene;
        for (int i = 1; i < argc; i += 2) {
            if (i + 1 >= argc)
                throw std::runtime_error("option requires a value");
            const std::string key = argv[i], value = argv[i + 1];
            if (key == "--executable")
                executable = fs::absolute(value);
            else if (key == "--actions")
                source = fs::absolute(value);
            else if (key == "--output-dir")
                directory = fs::absolute(value);
            else if (key == "--frame")
                frame = number(value, 1000000);
            else if (key == "--through-frame")
                through = number(value, 1000000);
            else if (key == "--dat") {
                scene.push_back(key);
                scene.push_back(fs::absolute(value).string());
            } else if (key == "--stage" || key == "--spell-id" || key == "--difficulty" ||
                       key == "--seed") {
                scene.push_back(key);
                scene.push_back(value);
            } else
                throw std::runtime_error("unknown probe option: " + key);
        }
        if (executable.empty() || source.empty() || directory.empty() || !frame)
            throw std::runtime_error("--executable, --actions, --frame and --output-dir required");
        std::ifstream input(source);
        if (!input)
            throw std::runtime_error("cannot open source tape");
        std::vector<std::uint16_t> tape;
        std::string token;
        while (input >> token) {
            if (tape.size() == 1000000)
                throw std::runtime_error("action tape exceeds frame bound");
            const auto action = number(token, 0xffff);
            if (action & ~0x10f7u)
                throw std::runtime_error("unsupported input in source tape");
            tape.push_back(action);
        }
        if (!input.eof())
            throw std::runtime_error("source tape read failed");
        if (!through)
            through = frame;
        if (through < frame || through > tape.size())
            throw std::runtime_error(
                "observation boundary must be between probe frame and tape end");
        const auto original = tape[frame - 1];
        tape.resize(through);
        fs::create_directories(directory);
        // A failed rerun must not leave an older successful summary in place.
        fs::remove(directory / "summary.json");
        std::string common_prefix;
        std::ostringstream records;
        unsigned branches = 0;
        std::uint64_t total_updates = 0;
        for (int y = -1; y <= 1; ++y)
            for (int x = -1; x <= 1; ++x) {
                // Preserve shooting, focus, confirm and all other input bits.
                const auto direction = (x < 0   ? 64
                                        : x > 0 ? 128
                                                : 0) |
                                       (y < 0   ? 16
                                        : y > 0 ? 32
                                                : 0);
                const auto action = std::uint16_t((original & ~0xf0u) | direction);
                tape[frame - 1] = action;
                const auto stem = directory / ("branch" + std::to_string(branches));
                const auto tape_path = stem.string() + ".actions";
                std::ofstream output(tape_path);
                if (!output)
                    throw std::runtime_error("cannot open branch tape");
                for (auto a : tape)
                    output << a << '\n';
                output.close();
                if (!output)
                    throw std::runtime_error("cannot write branch tape");
                std::vector<std::string> args{executable.string()};
                args.insert(args.end(), scene.begin(), scene.end());
                args.insert(args.end(), {"--frames", std::to_string(through), "--replay", tape_path,
                                         "--output", stem.string() + ".json", "--trace",
                                         stem.string() + ".tsv", "--prefix-frame",
                                         std::to_string(frame - 1), "--allow-unused-actions", "1"});
                const auto process = run(args);
                if (process.exit_code != 0 && process.exit_code != 2)
                    throw std::runtime_error("branch " + std::to_string(branches) +
                                             " failed; inspect its retained outputs");
                const auto report = read(stem.string() + ".json");
                const auto frames = number(field(report, "frames"), 1000000);
                if (frames < frame)
                    throw std::runtime_error("branch ended before the probe frame");
                // Capture agreement before replacement; a fixed future suffix
                // can expose delayed effects without claiming adaptation.
                const auto prefix = field(report, "prefix_trace_digest");
                if (!branches)
                    common_prefix = prefix;
                if (prefix != common_prefix)
                    throw std::runtime_error("branch prefixes diverged before replacement");
                if (branches)
                    records << ',';
                records << "{\"dx\":" << x << ",\"dy\":" << y << ",\"action\":" << action
                        << ",\"exit_code\":" << process.exit_code
                        << ",\"process_wall_ms\":" << process.wall_ms
                        << ",\"maximum_rss_kib\":" << process.rss_kib << ",\"execution\":" << report
                        << '}';
                ++branches;
                total_updates += frames;
            }
        std::ofstream output(directory / "summary.json");
        if (!output)
            throw std::runtime_error("cannot open probe summary");
        output << "{\"producer\":\"th08_headless_probe\",\"frame\":" << frame
               << ",\"through_frame\":" << through << ",\"original_action\":" << original
               << ",\"prefix_trace_digest\":" << common_prefix << ",\"branches\":" << branches
               << ",\"total_replayed_updates\":" << total_updates << ",\"runs\":[" << records.str()
               << "]}\n";
        if (!output)
            throw std::runtime_error("cannot write probe summary");
        std::cout << "Nine native branches agree through update " << frame - 1 << "; see "
                  << (directory / "summary.json") << '\n';
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "th08_headless_probe: " << e.what() << '\n';
        return 1;
    }
}
