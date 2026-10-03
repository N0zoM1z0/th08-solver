#pragma once

// Resume only the finite search's best fully validated failure. This is an
// artifact/checkpoint protocol, not a native memory snapshot: the caller still
// reconstructs every state and all RNG consumers in a new game process.
struct Resume {
    Record selected;
    fs::path prefix;
    std::string prior_summary;
    std::uint64_t known = 0, uncertain = 0;
    double wall_ms = 0;
};
std::uint64_t checked_add(std::uint64_t a, std::uint64_t b) {
    if (UINT64_MAX - a < b)
        throw std::runtime_error("cumulative native cost overflow");
    return a + b;
}
std::string plain_string(const std::string &document, const char *key) {
    auto value = field(document, key);
    if (value.size() < 3 || value.front() != '"' || value.back() != '"')
        throw std::runtime_error("invalid resume string");
    value = value.substr(1, value.size() - 2);
    if (value.find_first_not_of(
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_") !=
        std::string::npos)
        throw std::runtime_error("unsafe resume artifact name");
    return value;
}
Resume snapshot_resume(const fs::path &source, const fs::path &destination,
                       const fs::path &producer, const std::string &native_hash, unsigned cap) {
    const auto root = fs::canonical(source);
    fs::create_directory(destination);
    std::vector<std::pair<fs::path, std::string>> hashes;
    std::ofstream manifest(destination / "manifest.tsv");
    manifest << "file\tsha256\n";
    auto copy = [&](const std::string &name) {
        const auto input = root / name;
        if (fs::is_symlink(fs::symlink_status(input)) || !fs::is_regular_file(input) ||
            fs::canonical(input).parent_path() != root || fs::file_size(input) > 64 * 1024 * 1024)
            throw std::runtime_error("resume artifact is outside its bounded directory");
        const auto before = file_hash(input);
        fs::copy_file(input, destination / name);
        if (before != file_hash(input) || before != file_hash(destination / name))
            throw std::runtime_error("resume artifact changed during snapshot");
        hashes.emplace_back(input, before);
        manifest << name << '\t' << before << '\n';
    };
    copy("summary.json");
    Resume result;
    result.prior_summary = read(destination / "summary.json");
    const auto &summary = result.prior_summary;
    if (field(summary, "producer") != "\"th08_headless_repair\"" ||
        field(summary, "child_executable_sha256") != '"' + native_hash + '"' ||
        field(summary, "repair_executable_sha256") != '"' + file_hash(producer) + '"')
        throw std::runtime_error("resume executable identity mismatch");
    fs::copy_file(producer, destination / "repair-runner");
    if (file_hash(destination / "repair-runner") !=
        plain_string(summary, "repair_executable_sha256"))
        throw std::runtime_error("resume producer changed during snapshot");
    const auto selected = plain_string(summary, "selected_case");
    const auto execution = field(summary, "execution");
    copy("initial.json");
    copy("ledger.tsv");
    for (const auto *suffix : {".json", ".actions", ".policy"}) {
        if (selected != "initial" || std::string(suffix) != ".json")
            copy(selected + suffix);
    }
    result.selected = record(destination / selected, 2, cap, true);
    if (execution != result.selected.report && execution + '\n' != result.selected.report)
        throw std::runtime_error("selected report differs from prior summary");
    if (result.selected.outcome != "\"collision\"")
        throw std::runtime_error("only a collision frontier can resume");
    if (selected != "initial") {
        copy(selected + ".prefix");
        result.prefix = destination / (selected + ".prefix");
    }
    // Ledger order is the deterministic first-enumeration tie breaker. Include
    // the initial failure so a worse candidate cannot be advertised as progress.
    const auto initial = read(destination / "initial.json");
    unsigned best_frames = unsigned(integer(field(initial, "frames"), cap));
    std::string best = "initial";
    std::ifstream ledger(destination / "ledger.tsv");
    std::string line;
    if (!std::getline(ledger, line) ||
        line != "round\tonset\trollback_segments\tbranch_frame\taction\tframes\toutcome\twall_"
                "ms\tmaximum_rss_kib\tprefix_digest")
        throw std::runtime_error("invalid prior ledger header");
    std::uint64_t count = 0, updates = 0, prefixes = 0;
    while (std::getline(ledger, line)) {
        std::istringstream row(line);
        std::string round, onset, rollback, branch, action, frames, outcome, wall, rss, digest,
            extra;
        if (!(row >> round >> onset >> rollback >> branch >> action >> frames >> outcome >> wall >>
              rss >> digest) ||
            row >> extra)
            throw std::runtime_error("invalid prior ledger row");
        const auto n = integer(frames, cap), b = integer(branch, cap);
        if (!b || !integer(round, 2) || !integer(rollback, 8) ||
            (outcome != "\"collision\"" && outcome != "\"frame_limit\"" &&
             outcome != "\"retry_menu\""))
            throw std::runtime_error("invalid prior failure ledger");
        integer(onset, cap);
        integer(action, 65535);
        const auto name = "r" + round + "k" + rollback + "a" + action;
        if (outcome == "\"collision\"" && n > best_frames) {
            best_frames = unsigned(n);
            best = name;
        }
        ++count;
        updates = checked_add(updates, n);
        prefixes = checked_add(prefixes, std::min(b - 1, n));
    }
    const auto interrupted = integer(field(summary, "interrupted_update_upper_bound"), cap);
    if (!ledger.eof() || best != selected || best_frames != result.selected.frames ||
        updates != integer(field(summary, "candidate_native_updates"), UINT64_MAX) ||
        prefixes != integer(field(summary, "replayed_prefix_updates"), UINT64_MAX) ||
        count + (interrupted ? 1 : 0) != integer(field(summary, "candidates"), 144) ||
        integer(field(initial, "frames"), cap) !=
            integer(field(summary, "initial_native_updates"), cap))
        throw std::runtime_error("prior selected frontier or ledger costs disagree");
    // An unchanged frontier would merely repeat this same finite candidate
    // family. Starting another episode is supported only after real progress;
    // budget increases or a different proposal family need a separate decision.
    if (best_frames <= integer(field(initial, "frames"), cap))
        throw std::runtime_error("resume requires a strictly improved collision frontier");
    result.known = checked_add(integer(field(summary, "initial_native_updates"), cap), updates);
    result.known =
        checked_add(result.known, integer(field(summary, "verification_native_updates"), cap));
    result.uncertain = interrupted;
    const auto object = th08::headless_report::Document(summary).parse();
    const auto prior_fields = object.count("prior_verified_native_updates") +
                              object.count("prior_unverified_update_upper_bound") +
                              object.count("prior_wall_ms");
    const auto cumulative_fields = object.count("cumulative_verified_native_updates") +
                                   object.count("cumulative_unverified_update_upper_bound") +
                                   object.count("cumulative_wall_ms");
    if ((prior_fields != 0 && prior_fields != 3) ||
        (cumulative_fields != 0 && cumulative_fields != 3) ||
        (prior_fields == 0) != (cumulative_fields == 0))
        throw std::runtime_error("incomplete prior cumulative cost contract");
    if (object.count("prior_verified_native_updates")) {
        result.known = checked_add(
            result.known, integer(field(summary, "prior_verified_native_updates"), UINT64_MAX));
        result.uncertain =
            checked_add(result.uncertain,
                        integer(field(summary, "prior_unverified_update_upper_bound"), UINT64_MAX));
    }
    if (object.count("cumulative_verified_native_updates") &&
        (result.known !=
             integer(field(summary, "cumulative_verified_native_updates"), UINT64_MAX) ||
         result.uncertain !=
             integer(field(summary, "cumulative_unverified_update_upper_bound"), UINT64_MAX)))
        throw std::runtime_error("prior cumulative costs disagree");
    // Timings are measured host costs, not a reproducibility oracle. Parse the
    // entire scalar and reject nonfinite/negative values before accumulation.
    auto milliseconds = [&](const char *key) {
        const auto scalar = field(summary, key);
        std::size_t end = 0;
        const double value = std::stod(scalar, &end);
        if (end != scalar.size() || !std::isfinite(value) || value < 0)
            throw std::runtime_error("invalid prior timing cost");
        return value;
    };
    result.wall_ms = milliseconds("total_wall_ms");
    if (object.count("prior_wall_ms"))
        result.wall_ms += milliseconds("prior_wall_ms");
    if (!std::isfinite(result.wall_ms))
        throw std::runtime_error("cumulative wall cost overflow");
    if (cumulative_fields && std::fabs(milliseconds("cumulative_wall_ms") - result.wall_ms) >
                                 0.01 + result.wall_ms * 0.00001)
        throw std::runtime_error("prior cumulative wall cost disagrees");
    for (const auto &entry : hashes)
        if (file_hash(entry.first) != entry.second)
            throw std::runtime_error("prior artifacts changed during snapshot");
    manifest.flush();
    if (!manifest)
        throw std::runtime_error("cannot preserve resume artifact manifest");
    return result;
}
