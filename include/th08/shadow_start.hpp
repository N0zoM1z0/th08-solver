#pragma once
#include "th08/shadow_protocol.hpp"
#include <istream>
#include <sstream>
#include <string>

namespace th08::shadow {
struct StartRecord {
    unsigned seed = 0, maximum_updates = 0, row_bytes = Bytes{}.size() + 24;
    int stage = 0, difficulty = 0, shot = 0, spell = -1;
    bool story = false, prior_final_a_clear = false, advance_dialogue = false;
    std::string retail_hash = "330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924";
};
inline void validate_start(const StartRecord &s) {
    if (s.seed > 65535 || s.stage < 0 || s.stage > 8 || s.difficulty < 0 || s.difficulty > 4 ||
        s.shot < 0 || s.shot > 11 || s.spell < -1 || s.spell > 221 || !s.maximum_updates ||
        s.maximum_updates > 1000000 || s.row_bytes != Bytes{}.size() + 24 ||
        (s.story && s.spell >= 0) ||
        (s.prior_final_a_clear &&
         (!s.story || s.stage >= 6 || s.spell >= 0 || s.shot > 3 || s.difficulty > 3)) ||
        (s.advance_dialogue && s.spell >= 0) ||
        s.retail_hash != "330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924")
        throw std::runtime_error("invalid_shadow_start_record");
}
inline StartRecord read_start(std::istream &input) {
    StartRecord s;
    std::string version, trailing;
    unsigned story, history = 0, dialogue = 0;
    if (!(input >> version >> s.seed >> s.stage >> s.difficulty >> s.shot >> story >> s.spell >>
          s.maximum_updates >> s.row_bytes) ||
        (version != "TH08_SHADOW_START_V1" && version != "TH08_SHADOW_START_V2") || story > 1)
        throw std::runtime_error("invalid_shadow_start_record");
    if (version == "TH08_SHADOW_START_V2" &&
        (!(input >> history >> dialogue) || history > 1 || dialogue > 1))
        throw std::runtime_error("invalid_shadow_start_record");
    if (!(input >> s.retail_hash) || (input >> trailing))
        throw std::runtime_error("invalid_shadow_start_record");
    s.story = story;
    s.prior_final_a_clear = history;
    s.advance_dialogue = dialogue;
    validate_start(s);
    return s;
}
inline std::string encode_start(const StartRecord &s) {
    validate_start(s);
    const bool extended = s.prior_final_a_clear || s.advance_dialogue;
    std::ostringstream out;
    out << (extended ? "TH08_SHADOW_START_V2 " : "TH08_SHADOW_START_V1 ") << s.seed << ' '
        << s.stage << ' ' << s.difficulty << ' ' << s.shot << ' ' << unsigned(s.story) << ' '
        << s.spell << ' ' << s.maximum_updates << ' ' << s.row_bytes;
    if (extended)
        out << ' ' << unsigned(s.prior_final_a_clear) << ' ' << unsigned(s.advance_dialogue);
    out << '\n' << s.retail_hash << '\n';
    return out.str();
}
} // namespace th08::shadow
