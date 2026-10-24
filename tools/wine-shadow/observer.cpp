#include "identity.hpp"
#include "retail_view.hpp"
#include "th08/shadow_bridge.hpp"
#include "th08/shadow_start.hpp"
#include <cstdlib>
#include <limits>

namespace {
namespace s = th08::shadow;
namespace b = s::bridge;
using namespace wine_shadow;
HANDLE rows = INVALID_HANDLE_VALUE, mapping = nullptr;
std::uint32_t *shared = nullptr;
char directory[MAX_PATH];
bool started = false, spell_seen = false, story = true;
bool prior_final_a_clear = false, advance_dialogue = false, history_pending = false;
int configured_spell = -1;
std::uint64_t calculations = 0;
s::Packet packet;
b::Plan active;
unsigned max_frames = 1600, examined = 0;
b::CalculationCursor cursor;
LARGE_INTEGER frequency{}, first_tick{};
using Calc = int(__thiscall *)(void *);
using Init = int(__fastcall *)();
using Input = unsigned short(__fastcall *)();
Calc original_calc;
Init original_init;
Input original_input;

bool environment_flag(const char *name, bool &value) {
    char text[8];
    const auto size = GetEnvironmentVariableA(name, text, sizeof(text));
    if (!size) {
        value = false;
        return true;
    }
    if (size != 1 || (text[0] != '0' && text[0] != '1'))
        return false;
    value = text[0] == '1';
    return true;
}

[[noreturn]] void finish(b::OutcomeCode outcome, unsigned exit_code) {
    if (shared)
        b::save(shared + b::Outcome, std::uint32_t(outcome));
    if (rows != INVALID_HANDLE_VALUE) {
        FlushFileBuffers(rows);
        CloseHandle(rows);
    }
    ExitProcess(exit_code);
}
std::uint64_t ns(LONGLONG ticks) {
    return std::uint64_t(double(ticks) * 1e9 / double(frequency.QuadPart));
}
struct BulletRecord {
    unsigned slot, state, transforms;
    std::array<std::uint32_t, 14> values;
    int timer, duration, relative_timer;
};
std::array<BulletRecord, 0x601> diagnostic;
unsigned diagnostic_count = 0;
std::uint16_t diagnostic_control = 0;
void cache_diagnostic_bullets() {
    // Preserve the post-calc view before draw or the next input callback.
    // Disk I/O occurs only after a stop request.
    diagnostic_count = 0;
    __asm__ volatile("fnstcw %0" : "=m"(diagnostic_control));
    unsigned slot = 0;
    for (const auto &bullet : bullets().bullets) {
        if (bullet.state) {
            const auto &a = bullet.exStates[BULLET_TRANSFORM_STATE_VECTOR_ACCELERATION];
            const auto relative = relative_view(bullet);
            auto &record = diagnostic[diagnostic_count++];
            record.slot = slot;
            record.state = bullet.state;
            record.transforms = bullet.activeTransformFlags;
            unsigned index = 0;
            for (float value :
                 {bullet.position.x, bullet.position.y, bullet.velocity.x, bullet.velocity.y,
                  bullet.sprites.collisionSize.x, bullet.sprites.collisionSize.y, a.vector.x,
                  a.vector.y, relative.angle, relative.speed, bullet.angle, bullet.speed,
                  a.accelerationAngle, a.accelerationMagnitude})
                record.values[index++] = bits(value);
            record.timer = a.timer.current;
            record.duration = a.durationFrames;
            record.relative_timer = relative.timer;
        }
        ++slot;
    }
}
void diagnostic_bullets() {
    char path[MAX_PATH];
    std::snprintf(path, sizeof(path), "%s/retail-bullets.tsv", directory);
    FILE *file = std::fopen(path, "w");
    if (!file)
        return;
    std::fprintf(
        file,
        "sequence %llu cw %u\nslot state transforms x y vx vy width height ax ay relative_angle "
        "relative_speed bullet_angle bullet_speed acceleration_angle acceleration_magnitude "
        "accel_timer accel_duration relative_timer\n",
        static_cast<unsigned long long>(packet.sequence), unsigned(diagnostic_control));
    for (unsigned i = 0; i < diagnostic_count; ++i) {
        const auto &record = diagnostic[i];
        std::fprintf(file, "%u %u %u", record.slot, record.state, record.transforms);
        for (auto value : record.values)
            std::fprintf(file, " %u", unsigned(value));
        std::fprintf(file, " %d %d %d\n", record.timer, record.duration, record.relative_timer);
    }
    std::fclose(file);
}
int __fastcall init_hook() {
    if (!started && !game().flags.isDemoMode && !game().flags.isReplay) {
        // Optional named, reproducible startup checkpoint. This runs before the
        // original gameplay registration; it never replaces the actual RNG.
        char checkpoint[128];
        const auto checkpoint_size =
            GetEnvironmentVariableA("TH08_SHADOW_CHECKPOINT", checkpoint, sizeof(checkpoint));
        if (checkpoint_size >= sizeof(checkpoint))
            finish(b::OutcomeCode::Failure, 20);
        if (checkpoint_size) {
            int stage, difficulty, shot, spell, is_story;
            char trailing;
            if (std::sscanf(checkpoint, "%d,%d,%d,%d,%d%c", &stage, &difficulty, &shot, &spell,
                            &is_story, &trailing) != 5 ||
                stage < 0 || stage > 8 || difficulty < 0 || difficulty > 4 || shot < 0 ||
                shot > 11 || spell < -1 || spell > 221 || (is_story != 0 && is_story != 1) ||
                (is_story && spell >= 0))
                finish(b::OutcomeCode::Failure, 20);
            game().currentStage = stage;
            game().difficulty = difficulty;
            game().shotType = shot;
            game().fullShotType = 0;
            game().currentSpellCardNumber = spell;
            game().flags.isPracticeMode = !is_story && spell < 0;
            game().flags.isSpellPractice = !is_story && spell >= 0;
        }
        story = !game().flags.isPracticeMode && !game().flags.isSpellPractice;
        configured_spell = game().flags.isSpellPractice ? game().currentSpellCardNumber : -1;
        char path[MAX_PATH];
        std::snprintf(path, sizeof(path), "%s/start.tmp", directory);
        FILE *file = std::fopen(path, "w");
        if (!file)
            finish(b::OutcomeCode::Failure, 21);
        // Capture the seed at the SAME pre-RegisterChain boundary as Session.
        // A stage seed sampled after initialization is not interchangeable.
        try {
            s::StartRecord record;
            record.seed = at<unsigned short>(0x164d520);
            record.stage = game().currentStage;
            record.difficulty = game().difficulty;
            record.shot = game().shotType;
            record.story = story;
            record.spell = configured_spell;
            record.maximum_updates = max_frames;
            record.row_bytes = b::trace_bytes;
            record.prior_final_a_clear = prior_final_a_clear;
            record.advance_dialogue = advance_dialogue;
            record.retail_hash = retail_sha256;
            const auto text = s::encode_start(record);
            if (std::fwrite(text.data(), 1, text.size(), file) != text.size())
                finish(b::OutcomeCode::Failure, 21);
        } catch (...) {
            finish(b::OutcomeCode::Failure, 20);
        }
        if (std::fclose(file))
            finish(b::OutcomeCode::Failure, 21);
        char final_path[MAX_PATH];
        std::snprintf(final_path, sizeof(final_path), "%s/start.txt", directory);
        if (!MoveFileExA(path, final_path, MOVEFILE_WRITE_THROUGH))
            finish(b::OutcomeCode::Failure, 26);
        cursor.stage = game().currentStage;
        cursor.frame = 0;
        started = true;
        history_pending = prior_final_a_clear;
    }
    return original_init();
}
unsigned short __fastcall input_hook() {
    if (!started || !packet.sequence)
        return original_input();
    if (b::load(shared + b::StopRequested)) {
        diagnostic_bullets();
        finish(b::OutcomeCode::Stopped, 0);
    }
    b::Plan proposal;
    if (b::read(shared, proposal) && proposal.generation != examined) {
        examined = proposal.generation;
        const bool timely = b::timely(proposal, std::uint32_t(packet.sequence));
        if (timely && b::compatible_lead(proposal, active, std::uint16_t(packet.input),
                                         std::uint32_t(packet.sequence), configured_spell >= 0)) {
            active = proposal;
            b::save(shared + b::Accepted, proposal.generation);
        } else {
            b::save(shared + b::RejectionReason, timely ? 2 : 1);
            b::save(shared + b::Rejected, proposal.generation);
        }
    }
    const auto next = std::uint32_t(packet.sequence + 1);
    if (active.count && next > active.origin && next <= active.origin + active.count) {
        b::save(shared + b::Planned, b::load(shared + b::Planned) + 1);
        return active.inputs[next - active.origin - 1];
    }
    b::save(shared + b::Fallback, b::load(shared + b::Fallback) + 1);
    return b::fallback(std::uint16_t(packet.input), next, configured_spell >= 0);
}
int __fastcall calc_hook(void *self, void *) {
    LARGE_INTEGER begin{}, observed{}, end{};
    QueryPerformanceCounter(&begin);
    if (history_pending && game().gameplaySetupState == GAMEPLAY_SETUP_COMPLETE) {
        // Retail setup runs on a worker. Apply the declared score-history
        // checkpoint only after it has loaded, before the first game update.
        auto *recorder = at<ReplayManager *>(0x18b8a28);
        if (!recorder || !recorder->replayData || packet.sequence)
            finish(b::OutcomeCode::Failure, 27);
        auto &history = game().clrdData[game().shotType];
        for (int difficulty = EASY; difficulty <= LUNATIC; ++difficulty)
            if (history.difficultiesClearedWithoutRetries[difficulty] & ZUN_BIT(STAGE6B))
                finish(b::OutcomeCode::Failure, 27);
        history.difficultiesClearedWithoutRetries[NORMAL] |= ZUN_BIT(STAGE6A);
        history.difficultiesClearedWithRetries[NORMAL] |= ZUN_BIT(STAGE6A);
        recorder->replayData->clearState = 1;
        history_pending = false;
    }
    const int result = original_calc(self);
    QueryPerformanceCounter(&observed);
    if (started && game().globals) {
        const auto stage = unsigned(game().currentStage), frame = game().gameplayFrameCounter;
        const bool ready = supervisor().curState == SupervisorState_GameManager &&
                           game().gameplaySetupState == GAMEPLAY_SETUP_COMPLETE;
        const auto event = cursor.observe(stage, frame, ready);
        if (event == b::UpdateEvent::Unsupported)
            finish(b::OutcomeCode::Failure, 22);
        if (event != b::UpdateEvent::None) {
            calculations += event == b::UpdateEvent::Calculation;
            packet.epoch += event == b::UpdateEvent::Control;
            ++packet.sequence;
            packet.input = at<unsigned short>(0x164d528);
            try {
                packet.state = capture(packet, calculations);
                cache_diagnostic_bullets();
                if (packet.sequence == 1)
                    first_tick = begin;
                QueryPerformanceCounter(&end);
                const auto bytes = b::trace(packet, ns(end.QuadPart - first_tick.QuadPart),
                                            ns(observed.QuadPart - begin.QuadPart),
                                            ns(end.QuadPart - observed.QuadPart));
                DWORD written = 0;
                if (!WriteFile(rows, bytes.data(), bytes.size(), &written, nullptr) ||
                    written != bytes.size())
                    finish(b::OutcomeCode::Failure, 23);
            } catch (...) {
                finish(b::OutcomeCode::Failure, 24);
            }
            b::save(shared + b::Epoch, packet.epoch);
            b::save(shared + b::Sequence, packet.sequence);
            if (packet.state[s::Field::PlayerState] == 2 || packet.state[s::Field::Deaths] ||
                packet.state[s::Field::BombsUsed])
                finish(b::OutcomeCode::Death, 4);
            spell_seen = spell_seen || packet.state[s::Field::SpellActive];
            if ((configured_spell >= 0 && spell_seen && !packet.state[s::Field::SpellActive]) ||
                (packet.state[s::Field::StageComplete] && (!story || (stage >= 6 && stage <= 8))))
                finish(b::OutcomeCode::Complete, 0);
            if (packet.sequence >= max_frames)
                finish(b::OutcomeCode::Budget, 0);
        }
    }
    return result;
}

struct Hook {
    unsigned address, length;
    const unsigned char *expected;
    void *replacement;
    unsigned char *trampoline = nullptr;
    bool installed = false;
};
bool install(Hook &h) {
    auto *p = reinterpret_cast<unsigned char *>(h.address);
    DWORD protection;
    if (!VirtualProtect(p, h.length, PAGE_EXECUTE_READWRITE, &protection))
        return false;
    p[0] = 0xe9;
    const auto relative = unsigned(h.replacement) - h.address - 5;
    std::memcpy(p + 1, &relative, 4);
    std::memset(p + 5, 0x90, h.length - 5);
    h.installed = true;
    DWORD ignored;
    const bool ok = VirtualProtect(p, h.length, protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), p, h.length);
    return ok;
}
void rollback(Hook &h) {
    if (!h.installed)
        return;
    auto *p = reinterpret_cast<void *>(h.address);
    DWORD protection;
    if (!VirtualProtect(p, h.length, PAGE_EXECUTE_READWRITE, &protection))
        ExitProcess(25); // Owned suspended game cannot safely resume partial hooks.
    std::memcpy(p, h.expected, h.length);
    DWORD ignored;
    VirtualProtect(p, h.length, protection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), p, h.length);
}
bool setup() {
    char executable[MAX_PATH], path[MAX_PATH], limit[32];
    const auto executable_size = GetModuleFileNameA(nullptr, executable, sizeof(executable));
    const auto directory_size =
        GetEnvironmentVariableA("TH08_SHADOW_DIR", directory, sizeof(directory));
    if (!environment_flag("TH08_SHADOW_PRIOR_FINAL_A_CLEAR", prior_final_a_clear) ||
        !environment_flag("TH08_SHADOW_ADVANCE_DIALOGUE", advance_dialogue) || !executable_size ||
        executable_size >= sizeof(executable) || !directory_size ||
        directory_size >= sizeof(directory) - 48 || !pinned_executable(executable) ||
        !QueryPerformanceFrequency(&frequency) || frequency.QuadPart <= 0)
        return false;
    const auto limit_size = GetEnvironmentVariableA("TH08_SHADOW_UPDATES", limit, sizeof(limit));
    if (limit_size >= sizeof(limit))
        return false;
    if (limit_size) {
        char *end;
        const auto count = std::strtoul(limit, &end, 10);
        if (*end || !count || count > 1000000)
            return false;
        max_frames = unsigned(count);
    }
    std::snprintf(path, sizeof(path), "%s/frames.bin", directory);
    rows = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_NEW,
                       FILE_ATTRIBUTE_NORMAL, nullptr);
    if (rows == INVALID_HANDLE_VALUE)
        return false;
    std::snprintf(path, sizeof(path), "%s/commands.bin", directory);
    HANDLE file =
        CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                    CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;
    mapping = CreateFileMappingA(file, nullptr, PAGE_READWRITE, 0, b::map_bytes, nullptr);
    CloseHandle(file);
    if (!mapping)
        return false;
    shared = static_cast<std::uint32_t *>(
        MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, b::map_bytes));
    if (!shared)
        return false;
    b::save(shared + b::Version, b::version);
    b::save(shared + b::Magic, b::magic);
    const unsigned char calc[]{0x55, 0x8b, 0xec, 0x83, 0xec, 0x18};
    const unsigned char init[]{0x55, 0x8b, 0xec, 0x51, 0xc7, 0x45, 0xfc, 0x08, 0xf5, 0x60, 0x01};
    const unsigned char input[]{0x55, 0x8b, 0xec, 0x81, 0xec, 0x08, 0x01, 0x00, 0x00};
    Hook hooks[]{{0x43ca50, sizeof(calc), calc, reinterpret_cast<void *>(calc_hook)},
                 {0x43aa5c, sizeof(init), init, reinterpret_cast<void *>(init_hook)},
                 {0x43d970, sizeof(input), input, reinterpret_cast<void *>(input_hook)}};
    // Check every complete-instruction signature before modifying any byte.
    // These prologues contain no relative instructions. The original entry
    // thread remains suspended throughout installation by the owned launcher.
    for (auto &h : hooks) {
        if (std::memcmp(reinterpret_cast<void *>(h.address), h.expected, h.length))
            return false;
    }
    for (auto &h : hooks) {
        h.trampoline = static_cast<unsigned char *>(
            VirtualAlloc(nullptr, h.length + 5, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
        if (!h.trampoline) {
            for (auto &allocated : hooks)
                if (allocated.trampoline)
                    VirtualFree(allocated.trampoline, 0, MEM_RELEASE);
            return false;
        }
        std::memcpy(h.trampoline, h.expected, h.length);
        h.trampoline[h.length] = 0xe9;
        const auto back = h.address + h.length - unsigned(h.trampoline + h.length + 5);
        std::memcpy(h.trampoline + h.length + 1, &back, 4);
        FlushInstructionCache(GetCurrentProcess(), h.trampoline, h.length + 5);
    }
    original_calc = reinterpret_cast<Calc>(hooks[0].trampoline);
    original_init = reinterpret_cast<Init>(hooks[1].trampoline);
    original_input = reinterpret_cast<Input>(hooks[2].trampoline);
    for (auto &h : hooks)
        if (!install(h)) {
            for (auto &installed : hooks) {
                rollback(installed);
                VirtualFree(installed.trampoline, 0, MEM_RELEASE);
            }
            return false;
        }
    return true;
}
} // namespace
BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH)
        DisableThreadLibraryCalls(module);
    return TRUE;
}
// File/crypto/hook setup runs outside the loader lock, while the owned game's
// primary thread is still suspended. The launcher checks this explicit result.
extern "C" __declspec(dllexport) DWORD WINAPI Th08ShadowInitialize(LPVOID) {
    try {
        return setup() ? 1 : 0;
    } catch (...) {
        return 0;
    }
}
