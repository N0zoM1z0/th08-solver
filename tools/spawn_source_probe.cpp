#include "spawn_source_probe.hpp"
std::string source(const std::filesystem::path &, const char *);
std::string function(const std::string &, const std::string &);

std::string spawn_reference(const std::filesystem::path &repo) {
    const auto timeline =
        source(repo / "src/EnemyTimeline.cpp",
               "920ee34725aa6aad9f113d43454731acadab456abddac73256b2ba9a29e8e94b");
    const auto header = source(repo / "src/EnemyManager.hpp",
                               "e56633232cfb8e0934fb9e83f592989b577cd045e623df0c2c294eed9b2bf256");
    return R"CPP(
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <th08/practice_entry.hpp>
namespace spawn_reference {
using i32=std::int32_t; using u32=std::uint32_t;
using i16=std::int16_t; using i8=std::int8_t;
using D3DXVECTOR3=th08::enemy::Vec3;
constexpr u32 ENEMY_FLAG_ACTIVE=1, REPLAY_FRAME_EVENT_ENEMY_SPAWNED=1;
constexpr int ZUN_ERROR=-1;
)CPP" + function(header, "struct EnemyFlag1Bits") +
           R"CPP(;
struct Context { i32 intVariables[30]{}; i32 sub=0; };
struct Enemy {
    u32 flags1=0;
    i32 enemyIndex=0,life=1,score=100,maxLife=0,phaseStartingLife=0;
    i8 itemDropType=0;
    u32 displayColor=0;
    struct Vm {struct Color {u32 d3dColor=0;} color1;} vm;
    D3DXVECTOR3 position;
    Context mainEclContextStorage;
};
static_assert(sizeof(Enemy)%sizeof(u32)==0,"source whole-word copy domain");
struct Replay { u32 frameEventFlags=0; } replay;
Replay* g_ReplayManager=&replay;
Enemy before_run;
bool ran=false;
struct EclManager {
    void CallEclSub(Context* context,i16 sub) {context->sub=sub;}
    int RunEcl(Enemy* enemy) {
        // Controlled immediate-ECL boundary. These three tiny programs match
        // the independently compiled native test programs below. This compares
        // source spawn ordering, NOT the complete source ECL interpreter/tail.
        ran=true; before_run=*enemy;
        if(enemy->mainEclContextStorage.sub==1) return ZUN_ERROR;
        if(enemy->mainEclContextStorage.sub==2)
            enemy->life=enemy->maxLife=enemy->phaseStartingLife=7;
        return 0;
    }
} g_EclManager;
struct EnemyManager {
    Enemy enemies[480],spawnTemplate;
    bool lastSpawnFailed=false;
    EnemyManager(){spawnTemplate.flags1=1U|4U|8U|64U;}
    Enemy* SpawnEnemy1(i32,const D3DXVECTOR3*,i32,i32,i32,i32);
    Enemy* SpawnEnemy2(i32,const D3DXVECTOR3*,i32,i32,i32,i32*);
};
#define FLOAT3_CONST_PTR(value) (value)
)CPP" + function(timeline, "Enemy *EnemyManager::SpawnEnemy1(") +
           "\n" + function(timeline, "Enemy *EnemyManager::SpawnEnemy2(") +
           "\n#undef FLOAT3_CONST_PTR\n}\n";
}
