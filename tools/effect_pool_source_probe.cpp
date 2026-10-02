// Hash-checked allocation-body comparison generator. Source checkout is read-only.
#include "camera_particle_source_probe.hpp"
#include "source_probe_support.hpp"
#include <fstream>
#include <iostream>
namespace probe = th08::source_probe;
int main(int argc, char **argv) try {
    if (argc != 3)
        throw std::runtime_error("Usage: effect_pool_source_probe source output");
    const std::filesystem::path repo(argv[1]);
    const auto effects = probe::read_pinned_source(
        repo / "src/EffectManager.cpp",
        "63d45a213956008b44874bc4707c971a7799a9c551b07e732bf1f55282c2209e");
    const auto anm = probe::read_pinned_source(
        repo / "src/AnmManager.cpp",
        "c82bb37c19af4ccaabfa4bf4606d92c72e180f5f2fdd642cf3ec2131c85cecce");
    const auto ascii = probe::read_pinned_source(
        repo / "src/AsciiManager.cpp",
        "86c0d3cca5040036f16de762e80b3126b7037c89b526044cbb74bcc4bc6abdb1");
    const auto global = probe::read_pinned_source(
        repo / "src/Global.cpp",
        "8df17616c935d684b6636619d4726889e68f7d2d7000e27c25aebc4bc460b74b");
    std::ofstream out(argv[2]);
    out.exceptions(std::ios::badbit | std::ios::failbit);
    out << "#include <th08/effect_animation.hpp>\n#include <cstring>\n#include <iostream>\n"
           "#include <random>\n#include <memory>\n#include <stdexcept>\n";
    out << camera_particle_reference(repo);
    out << R"CPP(
namespace effect_pool_reference {
using namespace camera_particle_reference;
struct Effect : camera_particle_reference::Effect {
    int active, effectId;
    void *vertices;
    void *updateCallback;
};
// Controlled ANM boundary for the allocation comparison. Angular/template
// initialization is compared separately below, not implemented as a fake engine.
struct AnmLoaded {
    Animation initial{};
    void SetAndExecuteScriptIdx(Animation *vm, int script) {
        if (script != 73 && script != 75) throw std::runtime_error("unexpected effect script");
        *vm = initial;
    }
};
int initialize(Effect *effect) { return InitializeTintedBossTrackingCameraParticle(effect); }
struct EffectTemplate {int scriptIdx; void *updateCallback; int (*initializeCallback)(Effect*);};
EffectTemplate g_EffectTemplates[66]{};
struct Memory {void Free(void*) {throw std::runtime_error("vertices outside projection");}} g_ZunMemory;
struct Replay {unsigned frameEventFlags=0;} replay;
Replay *g_ReplayManager=&replay;
constexpr unsigned REPLAY_FRAME_EVENT_EFFECT_SPAWNED=1;
struct EffectManager {
    Effect effects[654]{};
    int nextEffectIndex=0;
    AnmLoaded *effectAnm;
    Effect *SpawnEffect(i32, D3DXVECTOR3*, i32, i32);
};
#define FLOAT3_PTR(p) reinterpret_cast<Float3*>(p)
)CPP";
    out << probe::extract_function(effects, "Effect *EffectManager::SpawnEffect(")
        << "\n#undef FLOAT3_PTR\n}\n";
    out << R"CPP(
namespace effect_anm_reference {
using f32=float; using i32=std::int32_t;
constexpr float ZUN_PI=3.14159265358979323846f, ZUN_2PI=ZUN_PI*2;
struct Vec {float x,y,z;};
struct Matrix {float value[16];};
void D3DXMatrixIdentity(Matrix *m) {for(int i=0;i<16;++i)m->value[i]=(i%5==0)?1.f:0.f;}
struct Color {std::uint32_t d3dColor;};
struct Timer {int previous,current;float fraction;void Initialize(){previous=-999;current=0;fraction=0;}};
constexpr std::uint32_t COLOR_WHITE=0xffffffffU;
struct AnmVmBase {
    Vec rotation,angleVel,scale;
    Color color1;
    Matrix matrix1;
    union {std::uint32_t flags;struct {std::uint32_t visible:1,flag1:1,updateRotation:1;};};
    Timer currentTimeInScript;
    void Initialize();
};
struct Supervisor {float framerateMultiplier=1;} g_Supervisor;
)CPP";
    out << probe::extract_function(ascii, "inline void AnmVmBase::Initialize()") << '\n';
    out << probe::extract_function(global, "f32 AddNormalizeAngle(") << '\n';
    const auto body = probe::extract_function(anm, "ZunBool AnmManager::ExecuteScript(");
    const auto angular_start = body.find("            vm->angleVel.x =");
    const auto angular_end = body.find("            break;", angular_start);
    const auto tail_start = body.find("    if (vm->angleVel.x !=");
    const auto tail_end = body.find("    for (i = 0; i < AnmInterp_Last", tail_start);
    if (angular_start == std::string::npos || angular_end == std::string::npos ||
        tail_start == std::string::npos || tail_end == std::string::npos)
        throw std::runtime_error("ANM angular source block missing");
    out << "void time_zero(AnmVmBase* vm, const float* args) {\n#define GET_FLOAT_VAR(n) args[n]\n"
        << body.substr(angular_start, angular_end - angular_start)
        << "\n#undef GET_FLOAT_VAR\nvm->visible=true;\n"
        << body.substr(tail_start, tail_end - tail_start) << "\n}\n}\n"
        << "#include \"source_effect_pool_cases.hpp\"\n";
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
}
