#include "transform_source_probe.hpp"
#include <stdexcept>

std::string source(const std::filesystem::path &path, const char *expected);
std::string function(const std::string &text, const std::string &signature);
namespace {
std::string section(const std::string &text, const char *first, const char *after) {
    const auto begin = text.find(first);
    const auto end = text.find(after, begin);
    if (begin == std::string::npos || end == std::string::npos)
        throw std::runtime_error("missing pinned transform section");
    return text.substr(begin, end - begin);
}
constexpr const char *adapter = R"CPP(
struct BulletSpawnDescriptor {
    Float3 position;
    int aimMode=0,bulletType=0,color=0,transformStartIndex=0,count1=0,count2=0;
    float speed1=0,speed2=0,angle=0,angleStep=0;
    unsigned transformFlags=0;
    BulletTransformRecord transforms[18]{};
};
struct Sprites {struct Vm {int activeSpriteIndex=0;} bulletVm;};
struct Anm {
    void SetSprite(Sprites::Vm*,int) {throw std::runtime_error("unexpected source sprite replacement");}
};
struct BulletManager {
    Sprites bulletTypeSprites[1];
    Anm* bulletAnm=nullptr;
    void SpawnBulletPattern(BulletSpawnDescriptor*) {throw std::runtime_error("unexpected source child pattern");}
} g_BulletManager;
struct SoundRecorder {
    std::array<th08::bullet::transform::SoundEvent,21> events;
    unsigned count=0;
    void emit(int id,bool positioned,float x) {
        if(count==events.size()) throw std::runtime_error("source exceeded sound trace bound");
        events[count++]={id,positioned,x};
    }
    void PlaySoundByIdx(int id,int){emit(id,false,0);}
    void PlaySoundPositionedByIdx(int id,float x){emit(id,true,x);}
} g_SoundPlayer;
constexpr int BULLET_STATE_FIRED=0,BULLET_STATE_DESPAWNING=1;
struct Bullet {
    Float3 position,velocity;
    float angle=0,speed=0;
    int transformIndex=0,transformSound=-1,offscreenCullDelayFrames=0,state=BULLET_STATE_FIRED;
    unsigned transformFlags=0,activeTransformFlags=0;
    BulletTransformRecord transforms[18]{};
    MotionState exStates[7];
    Sprites sprites;
    void AdvanceTransformProgram();
    void UpdateDeceleration();
    void UpdateVectorAcceleration();
    void UpdatePolarAcceleration();
    void UpdateRelativeDirectionChange();
    void UpdateAbsoluteDirectionChange();
    void UpdateAimedDirectionChange();
    void UpdateBoundaryBounce(){throw std::runtime_error("unexpected source bounce");}
    void UpdateHorizontalWrap(){throw std::runtime_error("unexpected source horizontal wrap");}
    void UpdateVerticalWrap(){throw std::runtime_error("unexpected source vertical wrap");}
};
)CPP";
} // namespace
std::string transform_reference(const std::filesystem::path &repo) {
    const auto header = source(repo / "src/BulletManager.hpp",
                               "583e9b9e89d49a358dafffaa9f123d817e4784afef114c33c464b906c8e7f596");
    const auto implementation =
        source(repo / "src/BulletManager.cpp",
               "77562e578c4fd2b2fd55f836f3b2e9e0ade16198208f81e94eb1d1a837207dc1");
    std::string generated =
        "namespace transform_reference {\n#define C_ASSERT(x) static_assert(x)\n" +
        section(header, "struct BulletTransformRawPayload", "enum BulletAimMode") +
        "#undef C_ASSERT\n" + function(header, "enum BulletTransformStateSlot") + ";\n" + adapter;
    for (const char *name :
         {"void Bullet::AdvanceTransformProgram()", "void Bullet::UpdateDeceleration()",
          "void Bullet::UpdateVectorAcceleration()", "void Bullet::UpdatePolarAcceleration()",
          "void Bullet::UpdateRelativeDirectionChange()",
          "void Bullet::UpdateAbsoluteDirectionChange()",
          "void Bullet::UpdateAimedDirectionChange()"})
        generated += function(implementation, name) + '\n';
    generated += "void step(Bullet* bullet) {bullet->AdvanceTransformProgram();\n" +
                 section(implementation, "            if (bullet->activeTransformFlags != 0)",
                         "            if (bullet->offscreenCullDelayFrames != 0)") +
                 "}\n}\n";
    return generated;
}
