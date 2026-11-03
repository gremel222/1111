#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace codex {

constexpr std::uint32_t kSimulationSchemaVersion = 1;
constexpr std::uint32_t kTicksPerSecond = 4;
constexpr std::uint32_t kKnowledgeTtlTicks = 12;
constexpr std::uint32_t kMaxBattleTicks = 200;

using UnitId = std::uint32_t;
using Tick = std::uint32_t;

enum class Team : std::uint8_t { Heroes, Enemies };
enum class Role : std::uint8_t { Guardian, Healer, Raider };
enum class ActionKind : std::uint8_t { Attack, Heal, Guard };
enum class ConditionKind : std::uint8_t {
    Always,
    SelfHpBelowPercent,
    AllyHpBelowPercent,
    EnemyKnown,
    EnemyHpBelowPercent,
    SelfResourceAtLeast,
};
enum class Truth : std::uint8_t { False, True, Unknown };
enum class Outcome : std::uint8_t { Running, HeroesWin, EnemiesWin, Draw };

struct Position {
    std::uint8_t depth = 0;
    std::uint8_t column = 0;
};

struct Ability {
    std::string id;
    ActionKind kind = ActionKind::Attack;
    std::int32_t resourceCost = 0;
    std::int32_t cooldownTicks = 0;
    std::int32_t recoveryTicks = 1;
    std::int32_t range = 1;
    std::int32_t power = 1;
    std::int32_t minimumDamage = 0;
};

struct Condition {
    ConditionKind kind = ConditionKind::Always;
    std::int32_t value = 0;
};

struct Rule {
    std::string id;
    std::int32_t priority = 0;
    Condition condition;
    std::string abilityId;
};

struct Program {
    std::string id;
    std::uint32_t schemaVersion = kSimulationSchemaVersion;
    std::vector<Rule> rules;
};

struct Knowledge {
    UnitId target = 0;
    Position position;
    std::int32_t hp = 0;
    std::int32_t maxHp = 1;
    Tick observedTick = 0;
    bool identified = false;
};

struct Unit {
    UnitId id = 0;
    std::string name;
    Team team = Team::Heroes;
    Role role = Role::Raider;
    Position position;
    std::int32_t hp = 1;
    std::int32_t maxHp = 1;
    std::int32_t resource = 0;
    std::int32_t maxResource = 0;
    std::int32_t resourceRegen = 0;
    Tick busyUntilTick = 0;
    Tick guardUntilTick = 0;
    std::int32_t guardPercent = 0;
    std::vector<Ability> abilities;
    std::map<std::string, Tick> readyAtTick;
    Program program;
};

struct BattleConfig {
    std::string scenarioId = "prototype.arena.v1";
    std::uint64_t seed = 1; // Reserved for replay; this slice uses no random decisions.
    std::uint32_t maxTicks = kMaxBattleTicks;
    std::vector<Unit> units;
};

struct Snapshot {
    Tick tick = 0;
    std::vector<Unit> units;
    // Knowledge is indexed by observer ID, then target ID. Decisions never read hidden enemy state.
    std::map<UnitId, std::map<UnitId, Knowledge>> knowledge;
};

struct DecisionRecord {
    Tick tick = 0;
    UnitId actor = 0;
    std::string ruleId;
    std::string abilityId;
    UnitId target = 0;
    std::string reason;
    bool selected = false;
    bool fallback = false;
};

struct BattleEvent {
    Tick tick = 0;
    std::uint64_t sequence = 0;
    UnitId source = 0;
    UnitId target = 0;
    std::string kind;
    std::int32_t amount = 0;
};

struct PendingAction {
    std::uint64_t sequence = 0;
    UnitId source = 0;
    UnitId target = 0;
    std::string abilityId;
    Tick commitTick = 0;
};

struct TickResult {
    Tick tick = 0;
    Outcome outcome = Outcome::Running;
    std::vector<BattleEvent> events;
    std::vector<DecisionRecord> decisions;
};

const char* ToString(Team value);
const char* ToString(Role value);
const char* ToString(ActionKind value);
const char* ToString(Outcome value);
std::int32_t LogicalDistance(const Unit& a, const Unit& b);
std::int32_t LogicalDistance(const Unit& a, const Position& b, bool sameTeam);

} // namespace codex
