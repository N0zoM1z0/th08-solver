// Compare fresh native replay captures with the pinned upstream retail corpus.
// The digest includes the canonical v4 schema and every raw little-endian row;
// process success and natural replay completion are separate required evidence.
#include "headless_report.hpp"
#include <openssl/evp.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <charconv>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {
namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;
using Object = std::map<std::string, std::string>;
constexpr std::uint64_t row_bytes = 25 * sizeof(std::uint32_t);
constexpr std::size_t summary_limit = 65536;
volatile std::sig_atomic_t interrupted = 0;

struct Failure : std::runtime_error {
    explicit Failure(const std::string &code) : std::runtime_error(code) {}
};

void require(bool condition, const char *code) {
    if (!condition)
        throw Failure(code);
}

void on_signal(int) {
    interrupted = 1;
}

std::string read_text(const fs::path &path, std::size_t limit) {
    std::ifstream input(path, std::ios::binary);
    require(bool(input), "input_unreadable");
    std::string text;
    std::array<char, 8192> buffer{};
    while (input.read(buffer.data(), buffer.size()) || input.gcount()) {
        require(text.size() + std::size_t(input.gcount()) <= limit, "json_size_limit");
        text.append(buffer.data(), std::size_t(input.gcount()));
    }
    require(input.eof(), "input_read_failed");
    return text;
}

Object object(const std::string &text) {
    try {
        return th08::headless_report::Document(text).parse();
    } catch (const std::exception &) {
        throw Failure("invalid_json");
    }
}

const std::string &field(const Object &value, const std::string &key) {
    const auto found = value.find(key);
    require(found != value.end(), "missing_json_field");
    return found->second;
}

std::string compact(const std::string &text) {
    bool quoted = false, escape = false;
    std::string result;
    for (const char c : text) {
        if (quoted || (c != ' ' && c != '\t' && c != '\r' && c != '\n'))
            result += c;
        if (escape)
            escape = false;
        else if (quoted && c == '\\')
            escape = true;
        else if (c == '"')
            quoted = !quoted;
    }
    return result;
}

// Document validates JSON first. This helper only separates its already-valid
// array elements; it is not a second permissive protocol parser.
std::vector<std::string> array(const std::string &text) {
    const auto value = compact(text);
    require(value.size() >= 2 && value.front() == '[' && value.back() == ']',
            "expected_json_array");
    std::vector<std::string> result;
    if (value.size() == 2)
        return result;
    std::size_t begin = 1;
    int depth = 0;
    bool quoted = false, escape = false;
    for (std::size_t i = 1; i + 1 < value.size(); ++i) {
        const char c = value[i];
        if (escape)
            escape = false;
        else if (quoted && c == '\\')
            escape = true;
        else if (c == '"')
            quoted = !quoted;
        else if (!quoted) {
            if (c == '[' || c == '{')
                ++depth;
            else if (c == ']' || c == '}')
                --depth;
            else if (c == ',' && depth == 0) {
                result.push_back(value.substr(begin, i - begin));
                begin = i + 1;
            }
        }
    }
    result.push_back(value.substr(begin, value.size() - begin - 1));
    return result;
}

std::uint64_t number(const std::string &text) {
    std::uint64_t result = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), result);
    require(!text.empty() && parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size(),
            "expected_unsigned_integer");
    return result;
}

// Identifiers, filenames and hashes in the pinned manifest are plain ASCII.
// Reject escaped/path-bearing labels so errors and public reports stay bounded.
std::string label(const std::string &text) {
    require(text.size() >= 3 && text.front() == '"' && text.back() == '"' && text.size() <= 130,
            "invalid_manifest_label");
    const auto result = text.substr(1, text.size() - 2);
    for (const unsigned char c : result)
        require((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
                    c == '-' || c == '_' || c == '.',
                "invalid_manifest_label");
    require(result != "." && result != "..", "invalid_manifest_label");
    return result;
}

bool valid_hash(const std::string &text) {
    return text.size() == 64 && std::all_of(text.begin(), text.end(), [](char c) {
               return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
           });
}

std::string hash_field(const Object &value, const std::string &key) {
    const auto result = label(field(value, key));
    require(valid_hash(result), "invalid_sha256");
    return result;
}

class Digest {
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> context{EVP_MD_CTX_new(),
                                                                    EVP_MD_CTX_free};

  public:
    Digest() {
        require(context && EVP_DigestInit_ex(context.get(), EVP_sha256(), nullptr) == 1,
                "sha256_initialization_failed");
    }
    void update(const char *data, std::size_t size) {
        require(EVP_DigestUpdate(context.get(), data, size) == 1, "sha256_update_failed");
    }
    std::string finish() {
        std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
        unsigned size = 0;
        require(EVP_DigestFinal_ex(context.get(), digest.data(), &size) == 1 && size == 32,
                "sha256_finalization_failed");
        std::ostringstream result;
        result << std::hex << std::setfill('0');
        for (unsigned i = 0; i < size; ++i)
            result << std::setw(2) << unsigned(digest[i]);
        return result.str();
    }
};

struct FileHash {
    std::string sha256;
    std::uint64_t bytes = 0;
};

FileHash hash_file(const fs::path &path, const std::string &prefix = {}) {
    std::ifstream input(path, std::ios::binary);
    require(bool(input), "input_unreadable");
    Digest digest;
    digest.update(prefix.data(), prefix.size());
    std::array<char, 65536> buffer{};
    FileHash result;
    while (input.read(buffer.data(), buffer.size()) || input.gcount()) {
        digest.update(buffer.data(), std::size_t(input.gcount()));
        result.bytes += std::uint64_t(input.gcount());
    }
    require(input.eof(), "input_read_failed");
    result.sha256 = digest.finish();
    return result;
}

// This is deliberately a single supported schema, not a general JSON canonical
// serializer. All of its strings are unescaped ASCII; sorting the object keys
// and removing insignificant whitespace equals upstream json.dumps(...,
// sort_keys=True, separators=(',', ':')) exactly. Unknown versions/content stop.
std::string schema_prefix(const fs::path &path) {
    const auto values = object(read_text(path, 65536));
    std::string canonical = "{";
    for (const auto &entry : values) {
        if (canonical.size() > 1)
            canonical += ',';
        canonical += '"' + entry.first + "\":" + compact(entry.second);
    }
    canonical += '}';
    const std::string expected =
        R"({"boundary":"playback-control entry, after gameplay calculation","fields":["calc","stage","frame","gameFrame","score","graze","deaths","lives","bombs","power","x","y","z","playerState","rngSeed","rngGeneration","input","pointItems","timeOrbs","gauge","clock","pointValue","character","shot","difficulty"],"floatFields":["deaths","lives","bombs","power","x","y","z"],"rngGeneration":"Generations since GameManager::OnUpdate entry for the first input frame; raw initial counter retained in completion metadata","stageIndexes":["Stage 1","Stage 2","Stage 3","Stage 4A","Stage 4B","Stage 5","Final A","Final B","Extra"],"version":4})";
    require(canonical == expected, "unsupported_replay_schema");
    return canonical + '\n';
}

struct Fixture {
    std::string id, file, replay_sha256, trace_sha256;
    std::uint64_t replay_bytes = 0, rows = 0;
};

std::vector<Fixture> fixtures(const fs::path &path, std::size_t expected_count) {
    const auto manifest = object(read_text(path, 2 * 1024 * 1024));
    require(number(field(manifest, "version")) == 1, "unsupported_manifest_version");
    const auto entries = array(field(manifest, "fixtures"));
    require(!entries.empty(), "empty_fixture_manifest");
    require(entries.size() == expected_count, "fixture_count_mismatch");
    std::set<std::string> ids, files, hashes;
    std::vector<Fixture> result;
    for (const auto &entry : entries) {
        const auto value = object(entry);
        const auto trace = object(field(value, "retailTrace"));
        Fixture fixture;
        fixture.id = label(field(value, "id"));
        fixture.file = label(field(value, "file"));
        require(fs::path(fixture.file).extension() == ".rpy", "invalid_replay_filename");
        fixture.replay_sha256 = hash_field(value, "sha256");
        fixture.trace_sha256 = hash_field(trace, "sha256");
        fixture.replay_bytes = number(field(value, "bytes"));
        fixture.rows = number(field(trace, "calculationFrames"));
        require(fixture.replay_bytes > 0, "empty_expected_replay");
        require(fixture.rows > 0 && fixture.rows <= 100000000, "invalid_expected_rows");
        require(number(field(trace, "schemaVersion")) == 4, "unsupported_fixture_schema");
        require(ids.insert(fixture.id).second, "duplicate_fixture_id");
        require(files.insert(fixture.file).second, "duplicate_fixture_file");
        require(hashes.insert(fixture.replay_sha256).second, "duplicate_replay_input");
        result.push_back(std::move(fixture));
    }
    return result;
}

void validate_corpus(const fs::path &corpus, const std::vector<Fixture> &expected) {
    require(fs::is_directory(corpus), "corpus_directory_missing");
    std::map<std::string, const Fixture *> remaining;
    for (const auto &fixture : expected)
        remaining.emplace(fixture.file, &fixture);
    for (const auto &entry : fs::directory_iterator(corpus)) {
        require(!entry.is_symlink() && entry.is_regular_file(), "invalid_corpus_entry");
        const auto found = remaining.find(entry.path().filename().string());
        require(found != remaining.end(), "unknown_corpus_input");
        const auto actual = hash_file(entry.path());
        require(actual.bytes == found->second->replay_bytes, "replay_size_mismatch");
        require(actual.sha256 == found->second->replay_sha256, "replay_hash_mismatch");
        remaining.erase(found);
    }
    require(remaining.empty(), "missing_corpus_input");
}

struct Options {
    fs::path checker, dat, manifest, schema, corpus, output, rows;
    std::string expected_sha256;
    std::uint64_t expected_rows = 0;
    std::size_t jobs = 1, expected_count = 108;
    unsigned timeout_seconds = 300;
    std::set<std::string> cases;
};

Options options(int argc, char **argv) {
    Options result;
    std::set<std::string> seen;
    for (int i = 1; i < argc; ++i) {
        const std::string key = argv[i];
        if (key == "--help") {
            std::cout << "usage: th08_replay_suite --checker CHECKER --dat DAT --manifest MANIFEST "
                         "--schema SCHEMA --corpus ACCEPTED_DIR --output NEW_DIR\n"
                         "       [--jobs 1..64] [--timeout-seconds 1..86400] "
                         "[--expected-count N (default 108)] [--case ID ...]\n"
                         "       th08_replay_suite --rows ROWS --schema SCHEMA "
                         "[--expected-rows N] [--expected-sha256 HASH]\n"
                         "Suite mode validates the entire corpus even with --case. "
                         "Fingerprint mode checks rows only, without execution evidence.\n";
            std::exit(0);
        }
        require(i + 1 < argc, "missing_option_value");
        require(key == "--case" || seen.insert(key).second, "duplicate_option");
        const std::string value = argv[++i];
        if (key == "--checker")
            result.checker = fs::absolute(value);
        else if (key == "--dat")
            result.dat = fs::absolute(value);
        else if (key == "--manifest")
            result.manifest = fs::absolute(value);
        else if (key == "--schema")
            result.schema = fs::absolute(value);
        else if (key == "--corpus")
            result.corpus = fs::absolute(value);
        else if (key == "--output")
            result.output = fs::absolute(value);
        else if (key == "--rows")
            result.rows = fs::absolute(value);
        else if (key == "--expected-sha256") {
            require(valid_hash(value), "invalid_sha256");
            result.expected_sha256 = value;
        } else if (key == "--expected-rows") {
            result.expected_rows = number(value);
            require(result.expected_rows > 0, "invalid_expected_rows");
        } else if (key == "--jobs") {
            result.jobs = number(value);
            require(result.jobs > 0 && result.jobs <= 64, "invalid_jobs");
        } else if (key == "--timeout-seconds") {
            const auto seconds = number(value);
            require(seconds > 0 && seconds <= 86400, "invalid_timeout");
            result.timeout_seconds = unsigned(seconds);
        } else if (key == "--expected-count") {
            result.expected_count = number(value);
            require(result.expected_count > 0 && result.expected_count <= 10000,
                    "invalid_expected_count");
        } else if (key == "--case") {
            require(result.cases.insert(label('"' + value + '"')).second,
                    "duplicate_case_selection");
        } else
            throw Failure("unknown_option");
    }
    require(!result.schema.empty(), "schema_required");
    if (!result.rows.empty()) {
        require(result.checker.empty() && result.dat.empty() && result.manifest.empty() &&
                    result.corpus.empty() && result.output.empty() && result.cases.empty() &&
                    !seen.count("--jobs") && !seen.count("--timeout-seconds") &&
                    !seen.count("--expected-count"),
                "mixed_fingerprint_and_suite_options");
    } else {
        require(!result.checker.empty() && !result.dat.empty() && !result.manifest.empty() &&
                    !result.corpus.empty() && !result.output.empty(),
                "suite_options_required");
        require(result.expected_sha256.empty() && !result.expected_rows,
                "fingerprint_option_in_suite");
    }
    return result;
}

FileHash fingerprint(const fs::path &rows, const std::string &prefix) {
    const auto result = hash_file(rows, prefix);
    require(result.bytes > 0, "empty_capture");
    require(result.bytes % row_bytes == 0, "truncated_capture_row");
    return result;
}

struct Result {
    std::string id, outcome = "pending", failure, sha256;
    std::uint64_t rows = 0;
    double seconds = 0;
};

struct Child {
    pid_t pid = -1;
    int output_fd = -1;
    std::size_t index = 0;
    fs::path rows;
    Clock::time_point started;
    std::string output;
    bool oversized_output = false, timed_out = false;
};

void kill_child(const Child &child) {
    if (child.pid > 0) {
        kill(-child.pid, SIGKILL);
        kill(child.pid, SIGKILL);
    }
}

struct Children {
    std::vector<Child> active;
    ~Children() {
        for (const auto &child : active)
            kill_child(child);
        for (const auto &child : active) {
            while (waitpid(child.pid, nullptr, 0) == -1 && errno == EINTR) {
            }
            if (child.output_fd >= 0)
                close(child.output_fd);
        }
    }
};

Child launch(const Options &opt, const Fixture &fixture, std::size_t index) {
    Child child;
    child.index = index;
    child.rows = opt.output / (fixture.id + ".rows");
    // The output directory was created exclusively for this run. Each checker
    // must create its own rows there; retained captures are never resumed.
    require(!fs::exists(child.rows), "stale_capture");
    const std::string checker = opt.checker.string(), dat = opt.dat.string(),
                      replay = (opt.corpus / fixture.file).string(), rows = child.rows.string();
    std::array<char *, 5> arguments{
        const_cast<char *>(checker.c_str()), const_cast<char *>(dat.c_str()),
        const_cast<char *>(replay.c_str()), const_cast<char *>(rows.c_str()), nullptr};
    int pipe_fds[2];
    require(pipe(pipe_fds) == 0, "child_pipe_failed");
    const int null_fd = open("/dev/null", O_WRONLY | O_CLOEXEC);
    if (null_fd < 0) {
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        throw Failure("child_redirect_failed");
    }
    child.started = Clock::now();
    child.pid = fork();
    if (child.pid == 0) {
        setpgid(0, 0);
        close(pipe_fds[0]);
        if (dup2(pipe_fds[1], STDOUT_FILENO) < 0 || dup2(null_fd, STDERR_FILENO) < 0)
            _exit(126);
        close(pipe_fds[1]);
        close(null_fd);
        // A broken checker cannot grow the rows file without bound. The extra
        // byte deliberately permits a partial-row failure to be diagnosed.
        const rlim_t limit = rlim_t(fixture.rows * row_bytes + 1);
        const rlimit file_limit{limit, limit};
        if (setrlimit(RLIMIT_FSIZE, &file_limit) != 0)
            _exit(126);
        execv(checker.c_str(), arguments.data());
        _exit(127);
    }
    close(pipe_fds[1]);
    close(null_fd);
    if (child.pid < 0) {
        close(pipe_fds[0]);
        throw Failure("child_fork_failed");
    }
    setpgid(child.pid, child.pid);
    child.output_fd = pipe_fds[0];
    const int flags = fcntl(child.output_fd, F_GETFL);
    if (flags < 0 || fcntl(child.output_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        kill_child(child);
        while (waitpid(child.pid, nullptr, 0) == -1 && errno == EINTR) {
        }
        close(child.output_fd);
        throw Failure("child_pipe_failed");
    }
    return child;
}

void drain(Child &child) {
    std::array<char, 4096> buffer{};
    for (;;) {
        const auto bytes = read(child.output_fd, buffer.data(), buffer.size());
        if (bytes > 0) {
            if (child.output.size() + std::size_t(bytes) <= summary_limit)
                child.output.append(buffer.data(), std::size_t(bytes));
            else {
                child.oversized_output = true;
                kill_child(child);
                return;
            }
        } else if (bytes == 0 || errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        else if (errno != EINTR)
            throw Failure("child_output_read_failed");
    }
}

Result finish(const Child &child, int status, const Fixture &fixture, const std::string &prefix) {
    Result result;
    result.id = fixture.id;
    result.outcome = "failed";
    result.seconds = std::chrono::duration<double>(Clock::now() - child.started).count();
    try {
        require(!interrupted, "interrupted");
        require(!child.timed_out, "child_timeout");
        require(!child.oversized_output, "child_summary_size_limit");
        require(WIFEXITED(status) && WEXITSTATUS(status) == 0, "child_process_failed");
        const auto summary = object(child.output);
        require(field(summary, "finished") == "true" && field(summary, "natural") == "true",
                "replay_not_naturally_finished");
        require(!summary.count("error"), "child_reported_error");
        for (const auto *key : {"runtime_error", "io_error"})
            require(!summary.count(key) || field(summary, key) == "false", "child_reported_error");
        require(number(field(summary, "rows")) == fixture.rows, "reported_row_count_mismatch");
        // One Session::step can perform several replay calculations during
        // native fast-forward. Rows and API calls therefore have no >= relation.
        const auto steps = number(field(summary, "steps"));
        require(steps > 0 && steps <= 400000, "invalid_step_count");
        require(fs::is_regular_file(child.rows) && !fs::is_symlink(child.rows), "capture_missing");
        const auto actual = fingerprint(child.rows, prefix);
        result.rows = actual.bytes / row_bytes;
        result.sha256 = actual.sha256;
        require(result.rows == fixture.rows, "capture_row_count_mismatch");
        require(actual.sha256 == fixture.trace_sha256, "trace_fingerprint_mismatch");
        result.outcome = "passed";
    } catch (const Failure &failure) {
        result.failure = failure.what();
    } catch (const std::exception &) {
        result.failure = "capture_validation_failed";
    }
    return result;
}

struct Identity {
    std::string manifest, schema, checker, dat;
};

void write_report(const Options &opt, const Identity &identity, std::size_t full_count,
                  const std::vector<Result> &results, const std::string &outcome,
                  const std::string &failure = {}) {
    std::uint64_t rows = 0;
    std::size_t passed = 0;
    for (const auto &result : results)
        if (result.outcome == "passed") {
            ++passed;
            rows += result.rows;
        }
    std::ofstream output(opt.output / "summary.tmp", std::ios::binary);
    require(bool(output), "report_write_failed");
    output << "{\"producer\":\"th08_replay_suite\",\"mode\":\"suite\",\"outcome\":\"" << outcome
           << "\",\"failure\":\"" << failure << "\",\"manifest_sha256\":\"" << identity.manifest
           << "\",\"schema_sha256\":\"" << identity.schema << "\",\"checker_sha256\":\""
           << identity.checker << "\",\"dat_sha256\":\"" << identity.dat
           << "\",\"expected_fixture_count\":" << opt.expected_count
           << ",\"manifest_fixtures\":" << full_count << ",\"selected_fixtures\":" << results.size()
           << ",\"full_suite\":"
           << (opt.expected_count == 108 && results.size() == full_count ? "true" : "false")
           << ",\"passed\":" << passed << ",\"verified_rows\":" << rows << ",\"jobs\":" << opt.jobs
           << ",\"timeout_seconds\":" << opt.timeout_seconds << ",\"cases\":[";
    bool first = true;
    for (const auto &result : results) {
        output << (first ? "" : ",") << "{\"id\":\"" << result.id << "\",\"outcome\":\""
               << result.outcome << "\",\"failure\":\"" << result.failure
               << "\",\"rows\":" << result.rows << ",\"sha256\":\"" << result.sha256
               << "\",\"elapsed_seconds\":" << std::fixed << std::setprecision(6) << result.seconds
               << '}';
        first = false;
    }
    output << "]}\n";
    output.close();
    require(bool(output), "report_write_failed");
    fs::rename(opt.output / "summary.tmp", opt.output / "summary.json");
}

int suite(const Options &opt, const std::string &prefix) {
    const auto all = fixtures(opt.manifest, opt.expected_count);
    const auto manifest_sha256 = hash_file(opt.manifest).sha256;
    // Count alone cannot establish corpus identity. A full-gate claim is bound
    // to the reviewed upstream 02ff72f manifest, including its credited inputs.
    if (opt.expected_count == 108)
        require(manifest_sha256 ==
                    "0aaef96d389bf85ad1c77734ddcddbf1a2d80d7119b86f9f1a892ebfec959eaf",
                "pinned_fixture_manifest_mismatch");
    validate_corpus(opt.corpus, all);
    std::vector<Fixture> selected;
    auto requested = opt.cases;
    for (const auto &fixture : all)
        if (opt.cases.empty() || requested.erase(fixture.id))
            selected.push_back(fixture);
    require(requested.empty(), "unknown_case_selection");
    require(!selected.empty(), "empty_case_selection");
    require(fs::is_regular_file(opt.checker) && access(opt.checker.c_str(), X_OK) == 0,
            "checker_not_executable");
    const Identity identity{manifest_sha256, hash_file(opt.schema).sha256,
                            hash_file(opt.checker).sha256, hash_file(opt.dat).sha256};
    fs::create_directories(opt.output.parent_path());
    require(fs::create_directory(opt.output), "output_directory_already_exists");
    std::vector<Result> results;
    for (const auto &fixture : selected) {
        Result result;
        result.id = fixture.id;
        results.push_back(std::move(result));
    }
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);
    write_report(opt, identity, all.size(), results, "running");
    try {
        Children children;
        children.active.reserve(opt.jobs);
        std::size_t next = 0;
        while (next < selected.size() || !children.active.empty()) {
            while (!interrupted && next < selected.size() && children.active.size() < opt.jobs) {
                children.active.push_back(launch(opt, selected[next], next));
                ++next;
            }
            for (std::size_t i = 0; i < children.active.size();) {
                auto &child = children.active[i];
                drain(child);
                if (interrupted ||
                    Clock::now() - child.started >= std::chrono::seconds(opt.timeout_seconds)) {
                    child.timed_out = !interrupted;
                    kill_child(child);
                }
                int status = 0;
                const auto waited = waitpid(child.pid, &status, WNOHANG);
                if (waited == 0 || (waited < 0 && errno == EINTR)) {
                    ++i;
                    continue;
                }
                require(waited == child.pid, "child_wait_failed");
                drain(child);
                close(child.output_fd);
                child.output_fd = -1;
                // Do not leave descendants alive after either success or error.
                kill(-child.pid, SIGKILL);
                results[child.index] = finish(child, status, selected[child.index], prefix);
                const auto &result = results[child.index];
                std::cout << result.id << ": " << result.outcome;
                if (!result.failure.empty())
                    std::cout << " (" << result.failure << ')';
                std::cout << '\n' << std::flush;
                fs::remove(child.rows);
                children.active.erase(children.active.begin() + std::ptrdiff_t(i));
                write_report(opt, identity, all.size(), results, "running");
            }
            if (interrupted && children.active.empty())
                throw Failure("interrupted");
            if (!children.active.empty())
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        require(hash_file(opt.manifest).sha256 == identity.manifest &&
                    hash_file(opt.schema).sha256 == identity.schema &&
                    hash_file(opt.checker).sha256 == identity.checker &&
                    hash_file(opt.dat).sha256 == identity.dat,
                "suite_identity_changed");
        validate_corpus(opt.corpus, all);
        const bool passed = std::all_of(results.begin(), results.end(), [](const Result &result) {
            return result.outcome == "passed";
        });
        write_report(opt, identity, all.size(), results, passed ? "passed" : "failed");
        std::cout << "verified "
                  << std::count_if(results.begin(), results.end(),
                                   [](const Result &r) { return r.outcome == "passed"; })
                  << '/' << results.size() << " selected fixtures; " << all.size()
                  << " manifest fixtures\n";
        return passed ? 0 : 1;
    } catch (const Failure &failure) {
        write_report(opt, identity, all.size(), results, "failed", failure.what());
        throw;
    } catch (const std::exception &) {
        write_report(opt, identity, all.size(), results, "failed", "suite_execution_failed");
        throw Failure("suite_execution_failed");
    }
}
} // namespace

int main(int argc, char **argv) {
    try {
        const auto opt = options(argc, argv);
        const auto prefix = schema_prefix(opt.schema);
        if (opt.rows.empty())
            return suite(opt, prefix);
        const auto result = fingerprint(opt.rows, prefix);
        const auto rows = result.bytes / row_bytes;
        require(!opt.expected_rows || rows == opt.expected_rows, "capture_row_count_mismatch");
        require(opt.expected_sha256.empty() || result.sha256 == opt.expected_sha256,
                "trace_fingerprint_mismatch");
        std::cout << "{\"producer\":\"th08_replay_suite\",\"mode\":\"fingerprint\",\"rows\":"
                  << rows << ",\"sha256\":\"" << result.sha256 << "\"}\n";
        return 0;
    } catch (const Failure &failure) {
        std::cerr << "th08_replay_suite: " << failure.what() << '\n';
    } catch (const std::exception &) {
        // Filesystem exceptions may contain local paths. Public CLI errors use
        // stable codes, never raw child output or environment-specific details.
        std::cerr << "th08_replay_suite: input_or_output_failure\n";
    }
    return 1;
}
