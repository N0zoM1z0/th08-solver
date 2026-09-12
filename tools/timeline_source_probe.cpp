#include "timeline_source_probe.hpp"
// Shared source readers supplied by source_probe.cpp pin complete file hashes.
std::string source(const std::filesystem::path &, const char *);
std::string function(const std::string &, const std::string &);

std::string timeline_reference(const std::filesystem::path &repo) {
    const auto text = source(repo / "src/EnemyTimeline.cpp",
                             "920ee34725aa6aad9f113d43454731acadab456abddac73256b2ba9a29e8e94b");
    const auto header = source(repo / "src/EnemyManager.hpp",
                               "e56633232cfb8e0934fb9e83f592989b577cd045e623df0c2c294eed9b2bf256");
    const auto gui = source(repo / "src/Gui.cpp",
                            "bd053c070d1ce136910e163898e6f97f4c00683558051d1a953967c6c5c89af8");
    const auto scale = source(repo / "src/AsciiManagerScale.cpp",
                              "b439ca540148240df69319277721cad4638a5422d7f98efc7a873e2fdcda62df");
    return "\nnamespace timeline_reference {\nusing u8=std::uint8_t;\n" +
           function(header, "enum EclTimelineOpcode") + ";\n" +
           function(header, "struct EclTimelineInstruction") + R"CPP(;
// The complete source dispatcher runs unchanged. External effects are recorded
// at their invocation boundary, not represented as a complete enemy/game world.
// Random spawn handlers are outside the ungated oracle domain and fail loudly.
struct D3DXVECTOR3 { float x,y,z; };
constexpr unsigned ENEMY_FLAG_ACTIVE=1;
struct EclTimeline {
    ZunTimer timer;
    EclTimelineInstruction* instruction=nullptr;
    void Run();
};
EclTimeline* active_timeline=nullptr;
const std::uint8_t* timeline_start=nullptr;
std::array<std::uint32_t,64> effect_offsets{};
unsigned effect_count=0;
void record_effect() {
    if(!active_timeline || effect_count>=effect_offsets.size())
        throw std::runtime_error("timeline source recorder bound");
    effect_offsets[effect_count++]=std::uint32_t(
        reinterpret_cast<const std::uint8_t*>(active_timeline->instruction)-timeline_start);
}
struct EffectWrite {
    int value=0;
    void operator=(int next){value=next;record_effect();}
};
struct Enemy {
    unsigned flags1=0;
    EffectWrite pendingEclSubroutineIndex;
    int pointItemDropCount=0,powerOrPointItemDropCount=0;
};
struct EnemyManager {
    Enemy* bosses[8]{};
    Enemy spawned;
    int suppressTimelineSpawns=0;
    int timelineEventSlots[4]{};
    Enemy* SpawnEnemy1(int,const D3DXVECTOR3*,int,int,int,int){record_effect();return &spawned;}
} g_EnemyManager;
struct GameManager {
    int difficultyMask=1;
    EffectWrite showRetryMenu;
    void SetPower(int){record_effect();}
} g_GameManager;
struct Gui {
    struct Impl {struct Message {int ignoreWaitCounter=0,currentMsgIdx=-1;} message;};
    Impl* impl=nullptr;
    bool bossPresent=false;
    bool IsBossPresent();
    int MsgWait();
    void MsgRead(int){record_effect();}
} g_Gui;
struct RejectedRandom {
    float GetRandomF32InRange(float){
        throw std::runtime_error("ungated random timeline spawn outside source recorder domain");
    }
} g_Rng;
)CPP" + function(gui, "i32 Gui::MsgWait()") +
           "\n" + function(scale, "bool Gui::IsBossPresent()") + "\n" +
           function(text, "void EclTimeline::Run()") + "\n}\n";
}
