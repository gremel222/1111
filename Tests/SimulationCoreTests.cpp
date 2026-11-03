#include "SimulationCore/Battle.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
int passed = 0;
int failed = 0;

void Check(bool condition, const char* expression, const char* file, int line) {
    if (!condition) throw std::runtime_error(std::string(file) + ":" + std::to_string(line) + ": " + expression);
}
#define CHECK(expr) Check(static_cast<bool>(expr), #expr, __FILE__, __LINE__)

void Test(const char* name, const std::function<void()>& body) {
    try {
        body();
        ++passed;
        std::cout << "PASS " << name << '\n';
    } catch (const std::exception& error) {
        ++failed;
        std::cout << "FAIL " << name << ": " << error.what() << '\n';
    }
}

using namespace codex;

std::string Trace(Battle& battle, int steps = 30) {
    std::ostringstream out;
    for (int i = 0; i < steps; ++i) {
        const TickResult result = battle.Step();
        out << "T" << result.tick << ':' << ToString(result.outcome) << ';';
        for (const auto& e : result.events)
            out << 'E' << e.sequence << ':' << e.source << '>' << e.target << ':' << e.kind << ':' << e.amount << ';';
        for (const auto& d : result.decisions)
            out << 'D' << d.actor << ':' << d.ruleId << ':' << d.abilityId << ':' << d.target << ':'
                << d.reason << ':' << d.selected << ':' << d.fallback << ';';
    }
    return out.str();
}

const Unit& UnitById(const Snapshot& snapshot, UnitId id) {
    const auto it = std::find_if(snapshot.units.begin(), snapshot.units.end(), [id](const Unit& u) { return u.id == id; });
    CHECK(it != snapshot.units.end());
    return *it;
}

bool HasReason(const TickResult& result, const std::string& reason) {
    return std::any_of(result.decisions.begin(), result.decisions.end(),
        [&reason](const DecisionRecord& d) { return d.reason == reason; });
}

BattleConfig MinimalConfig() {
    BattleConfig config;
    config.scenarioId = "test.minimal";
    Unit hero;
    hero.id = 1; hero.name = "Hero"; hero.team = Team::Heroes; hero.role = Role::Guardian;
    hero.hp = hero.maxHp = 100; hero.position = {0, 1};
    hero.abilities = {{"test.strike", ActionKind::Attack, 0, 0, 1, 5, 10, 1}};
    hero.program = {"test.program", kSimulationSchemaVersion, {}};
    Unit enemy;
    enemy.id = 2; enemy.name = "Enemy"; enemy.team = Team::Enemies; enemy.role = Role::Raider;
    enemy.hp = enemy.maxHp = 30; enemy.position = {0, 1};
    enemy.abilities = {{"test.bite", ActionKind::Attack, 0, 0, 1, 5, 5, 1}};
    enemy.program = {"test.enemy", kSimulationSchemaVersion, {}};
    config.units = {hero, enemy};
    return config;
}
} // namespace

int main() {
    Test("prototype_scenario_validates", [] {
        const auto config = Battle::MakePrototypeScenario();
        const auto errors = Battle::Validate(config);
        for (const auto& error : errors) std::cout << "  validation: " << error << '\n';
        CHECK(errors.empty());
        CHECK(config.units.size() == 5);
    });

    Test("duplicate_id_and_slot_rejected", [] {
        auto config = MinimalConfig();
        config.units[1].id = config.units[0].id;
        CHECK(!Battle::Validate(config).empty());
        config = MinimalConfig();
        config.units[1].team = Team::Heroes;
        config.units[1].position = config.units[0].position;
        CHECK(!Battle::Validate(config).empty());
    });

    Test("unknown_ability_and_schema_rejected", [] {
        auto config = MinimalConfig();
        config.units[0].program.rules.push_back({"r", 0, {}, "missing"});
        CHECK(!Battle::Validate(config).empty());
        config = MinimalConfig();
        config.units[0].program.schemaVersion = 999;
        CHECK(!Battle::Validate(config).empty());
    });

    Test("invalid_battle_does_not_step", [] {
        auto config = MinimalConfig();
        config.units.clear();
        Battle battle(config);
        CHECK(!battle.GetErrors().empty());
        const auto result = battle.Step();
        CHECK(battle.GetTick() == 0);
        CHECK(HasReason(result, "invalid_scenario"));
    });

    Test("snapshot_units_sorted_by_stable_id", [] {
        auto config = Battle::MakePrototypeScenario();
        std::reverse(config.units.begin(), config.units.end());
        Battle battle(config);
        const auto& units = battle.GetSnapshot().units;
        CHECK(std::is_sorted(units.begin(), units.end(), [](const Unit& a, const Unit& b) { return a.id < b.id; }));
    });

    Test("declare_then_commit_next_tick", [] {
        Battle battle(MinimalConfig());
        const auto first = battle.Step();
        CHECK(first.events.size() == 2); // two declares, no damage yet
        CHECK(first.decisions.size() == 2);
        const auto second = battle.Step();
        bool damage = false;
        for (const auto& e : second.events) damage = damage || e.kind == "damage";
        CHECK(damage);
        CHECK(second.tick == 2);
    });

    Test("same_input_same_complete_trace", [] {
        Battle a(Battle::MakePrototypeScenario());
        Battle b(Battle::MakePrototypeScenario());
        CHECK(Trace(a, 80) == Trace(b, 80));
    });

    Test("input_order_does_not_change_trace", [] {
        auto config = Battle::MakePrototypeScenario();
        Battle a(config);
        std::reverse(config.units.begin(), config.units.end());
        Battle b(config);
        CHECK(Trace(a, 80) == Trace(b, 80));
    });

    Test("one_selected_action_per_actor_per_tick", [] {
        Battle battle(Battle::MakePrototypeScenario());
        for (int i = 0; i < 40; ++i) {
            const auto result = battle.Step();
            std::set<UnitId> selected;
            for (const auto& d : result.decisions)
                if (d.selected) CHECK(selected.insert(d.actor).second);
        }
    });

    Test("equal_priority_uses_upper_rule", [] {
        auto config = MinimalConfig();
        auto& hero = config.units[0];
        hero.abilities.push_back({"test.guard", ActionKind::Guard, 0, 0, 1, 0, 50, 0});
        hero.program.rules = {
            {"upper", 5, {ConditionKind::Always, 0}, "test.guard"},
            {"lower", 5, {ConditionKind::Always, 0}, "test.strike"},
        };
        Battle battle(config);
        const auto result = battle.Step();
        const auto it = std::find_if(result.decisions.begin(), result.decisions.end(),
            [](const DecisionRecord& d) { return d.actor == 1 && d.selected; });
        CHECK(it != result.decisions.end());
        CHECK(it->ruleId == "upper");
    });

    Test("cooldown_rule_skipped_with_reason", [] {
        auto config = MinimalConfig();
        auto& hero = config.units[0];
        hero.abilities[0].cooldownTicks = 10;
        hero.program.rules = {{"cooldown_rule", 10, {ConditionKind::Always, 0}, "test.strike"}};
        Battle battle(config);
        battle.Step();
        battle.Step();
        const auto third = battle.Step();
        CHECK(HasReason(third, "cooldown"));
    });

    Test("unknown_condition_is_false", [] {
        auto config = MinimalConfig();
        Battle battle(config);
        auto snapshot = battle.GetSnapshot();
        snapshot.knowledge.clear();
        CHECK(Battle::EvaluateCondition(snapshot.units[0], {ConditionKind::EnemyKnown, 0}, snapshot) == Truth::Unknown);
        const UnitId target = Battle::SelectTarget(snapshot.units[0], snapshot.units[0].abilities[0], snapshot);
        CHECK(target == 0);
    });

    Test("stale_knowledge_cannot_target", [] {
        auto config = MinimalConfig();
        Battle battle(config);
        auto snapshot = battle.GetSnapshot();
        snapshot.tick = kKnowledgeTtlTicks + 1;
        CHECK(Battle::SelectTarget(snapshot.units[0], snapshot.units[0].abilities[0], snapshot) == 0);
    });

    Test("attack_target_low_hp_then_distance_then_id", [] {
        auto config = MinimalConfig();
        Unit enemy = config.units[1];
        enemy.id = 3; enemy.position = {0, 0}; enemy.hp = 10;
        config.units.push_back(enemy);
        Battle battle(config);
        const auto& snapshot = battle.GetSnapshot();
        CHECK(Battle::SelectTarget(snapshot.units[0], snapshot.units[0].abilities[0], snapshot) == 3);
    });

    Test("guardian_fallback_guard_then_attack", [] {
        auto config = MinimalConfig();
        auto& hero = config.units[0];
        hero.resource = hero.maxResource = 10;
        hero.abilities.push_back({"test.guard", ActionKind::Guard, 2, 0, 0, 0, 50, 0});
        Battle battle(config);
        const auto result = battle.Step();
        const auto it = std::find_if(result.decisions.begin(), result.decisions.end(),
            [](const DecisionRecord& d) { return d.actor == 1 && d.selected; });
        CHECK(it != result.decisions.end());
        CHECK(it->fallback);
        CHECK(it->abilityId == "test.guard");
    });

    Test("healer_fallback_heals_wounded_ally", [] {
        auto config = MinimalConfig();
        auto& hero = config.units[0];
        hero.role = Role::Healer; hero.hp = 50; hero.resource = hero.maxResource = 10;
        hero.abilities.push_back({"test.heal", ActionKind::Heal, 2, 0, 0, 6, 10, 0});
        Battle battle(config);
        const auto result = battle.Step();
        const auto it = std::find_if(result.decisions.begin(), result.decisions.end(),
            [](const DecisionRecord& d) { return d.actor == 1 && d.selected; });
        CHECK(it != result.decisions.end());
        CHECK(it->abilityId == "test.heal");
        CHECK(it->target == 1);
    });


    Test("guardian_fallback_attacks_when_guard_unavailable", [] {
        auto config = MinimalConfig();
        auto& hero = config.units[0];
        hero.resource = hero.maxResource = 0;
        hero.abilities.push_back({"test.guard", ActionKind::Guard, 2, 0, 0, 0, 50, 0});
        Battle battle(config);
        const auto result = battle.Step();
        const auto it = std::find_if(result.decisions.begin(), result.decisions.end(),
            [](const DecisionRecord& d) { return d.actor == 1 && d.selected; });
        CHECK(it != result.decisions.end());
        CHECK(it->abilityId == "test.strike");
    });

    Test("enemy_fallback_uses_attack", [] {
        auto config = MinimalConfig();
        config.units[1].program.rules.clear();
        Battle battle(config);
        const auto result = battle.Step();
        const auto it = std::find_if(result.decisions.begin(), result.decisions.end(),
            [](const DecisionRecord& d) { return d.actor == 2 && d.selected; });
        CHECK(it != result.decisions.end());
        CHECK(it->fallback);
        CHECK(it->abilityId == "test.bite");
    });

    Test("no_fallback_is_explicit_skip", [] {
        auto config = MinimalConfig();
        config.units[0].abilities.clear();
        Battle battle(config);
        const auto result = battle.Step();
        CHECK(HasReason(result, "no_available_fallback"));
    });

    Test("resource_paid_once_at_commit", [] {
        auto config = MinimalConfig();
        auto& hero = config.units[0];
        hero.resource = hero.maxResource = 5;
        hero.abilities[0].resourceCost = 3;
        hero.abilities[0].cooldownTicks = 100;
        Battle battle(config);
        battle.Step();
        CHECK(UnitById(battle.GetSnapshot(), 1).resource == 5);
        battle.Step();
        CHECK(UnitById(battle.GetSnapshot(), 1).resource == 2);
        battle.Step();
        CHECK(UnitById(battle.GetSnapshot(), 1).resource == 2);
    });


    Test("guard_reduces_damage_by_ability_percent", [] {
        auto config = MinimalConfig();
        auto& hero = config.units[0];
        hero.resource = hero.maxResource = 10;
        hero.abilities.push_back({"test.guard", ActionKind::Guard, 0, 100, 0, 0, 50, 0});
        hero.program.rules = {{"guard", 0, {ConditionKind::Always, 0}, "test.guard"}};
        Battle battle(config);
        battle.Step();
        battle.Step(); // hero guard commits before enemy attack by stable source ID
        CHECK(UnitById(battle.GetSnapshot(), 1).hp == 98); // 5 damage reduced by 50% -> floor 2
    });

    Test("heal_clamped_to_max_hp", [] {
        auto config = MinimalConfig();
        auto& hero = config.units[0];
        hero.hp = 95;
        hero.abilities.push_back({"test.heal", ActionKind::Heal, 0, 0, 0, 6, 50, 0});
        hero.program.rules = {{"heal", 0, {ConditionKind::Always, 0}, "test.heal"}};
        config.units[1].abilities.clear(); // isolate healing from same-tick enemy damage
        Battle battle(config);
        battle.Step(); battle.Step();
        CHECK(UnitById(battle.GetSnapshot(), 1).hp == 100);
    });

    Test("battle_reaches_terminal_outcome", [] {
        Battle battle(Battle::MakePrototypeScenario());
        for (int i = 0; i < 200 && battle.GetOutcome() == Outcome::Running; ++i) battle.Step();
        CHECK(battle.GetOutcome() != Outcome::Running);
        const Tick tick = battle.GetTick();
        battle.Step();
        CHECK(battle.GetTick() == tick);
    });

    Test("max_ticks_prevents_infinite_battle", [] {
        auto config = MinimalConfig();
        config.maxTicks = 3;
        config.units[0].abilities.clear();
        config.units[1].abilities.clear();
        Battle battle(config);
        for (int i = 0; i < 5; ++i) battle.Step();
        CHECK(battle.GetTick() == 3);
        CHECK(battle.GetOutcome() == Outcome::Draw);
    });

    Test("hp_percent_comparison_is_integer", [] {
        auto config = MinimalConfig();
        config.units[0].maxHp = 3; config.units[0].hp = 1;
        Battle battle(config);
        const auto& snapshot = battle.GetSnapshot();
        CHECK(Battle::EvaluateCondition(snapshot.units[0], {ConditionKind::SelfHpBelowPercent, 34}, snapshot) == Truth::True);
        CHECK(Battle::EvaluateCondition(snapshot.units[0], {ConditionKind::SelfHpBelowPercent, 33}, snapshot) == Truth::False);
    });

    std::cout << "\nSimulationCore native tests: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
