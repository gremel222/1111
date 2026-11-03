#include "SimulationCore/Battle.hpp"

#include <algorithm>
#include <cstdlib>
#include <set>
#include <utility>

namespace codex {
namespace {

bool IsAlive(const Unit& unit) { return unit.hp > 0; }
bool IsFresh(const Knowledge& entry, Tick now) {
    return entry.identified && now >= entry.observedTick && now - entry.observedTick <= kKnowledgeTtlTicks;
}
int PercentHp(const Unit& unit) {
    return unit.maxHp > 0 ? static_cast<int>((static_cast<std::int64_t>(unit.hp) * 100) / unit.maxHp) : 0;
}

} // namespace

const char* ToString(Team value) { return value == Team::Heroes ? "heroes" : "enemies"; }
const char* ToString(Role value) {
    switch (value) {
        case Role::Guardian: return "guardian";
        case Role::Healer: return "healer";
        case Role::Raider: return "raider";
    }
    return "unknown";
}
const char* ToString(ActionKind value) {
    switch (value) {
        case ActionKind::Attack: return "attack";
        case ActionKind::Heal: return "heal";
        case ActionKind::Guard: return "guard";
    }
    return "unknown";
}
const char* ToString(Outcome value) {
    switch (value) {
        case Outcome::Running: return "running";
        case Outcome::HeroesWin: return "heroes_win";
        case Outcome::EnemiesWin: return "enemies_win";
        case Outcome::Draw: return "draw";
    }
    return "unknown";
}

std::int32_t LogicalDistance(const Unit& a, const Position& b, bool sameTeam) {
    const int columns = std::abs(static_cast<int>(a.position.column) - static_cast<int>(b.column));
    if (sameTeam) {
        return std::abs(static_cast<int>(a.position.depth) - static_cast<int>(b.depth)) + columns;
    }
    return 1 + static_cast<int>(a.position.depth) + static_cast<int>(b.depth) + columns;
}
std::int32_t LogicalDistance(const Unit& a, const Unit& b) {
    return LogicalDistance(a, b.position, a.team == b.team);
}

Battle::Battle(BattleConfig config) : config_(std::move(config)) {
    errors_ = Validate(config_);
    units_ = config_.units;
    std::sort(units_.begin(), units_.end(), [](const Unit& a, const Unit& b) { return a.id < b.id; });
    if (errors_.empty()) {
        for (const Unit& unit : units_) if (!IsAlive(unit)) downReported_.insert(unit.id);
        UpdatePerception();
        snapshot_ = BuildSnapshot(tick_, units_, knowledge_);
    }
}

std::vector<std::string> Battle::Validate(const BattleConfig& config) {
    std::vector<std::string> errors;
    if (config.scenarioId.empty()) errors.push_back("scenario_id_empty");
    if (config.maxTicks == 0 || config.maxTicks > 100000) errors.push_back("max_ticks_out_of_range");
    if (config.units.empty() || config.units.size() > 18) errors.push_back("unit_count_out_of_range");
    std::set<UnitId> ids;
    std::set<std::pair<Team, std::pair<int, int>>> slots;
    bool heroes = false;
    bool enemies = false;
    for (const Unit& unit : config.units) {
        const std::string prefix = "unit:" + std::to_string(unit.id) + ":";
        if (unit.id == 0 || !ids.insert(unit.id).second) errors.push_back(prefix + "invalid_or_duplicate_id");
        if (unit.name.empty()) errors.push_back(prefix + "name_empty");
        if (unit.maxHp <= 0 || unit.hp < 0 || unit.hp > unit.maxHp) errors.push_back(prefix + "invalid_hp");
        if (unit.maxResource < 0 || unit.resource < 0 || unit.resource > unit.maxResource || unit.resourceRegen < 0)
            errors.push_back(prefix + "invalid_resource");
        if (unit.position.depth > 2 || unit.position.column > 2) errors.push_back(prefix + "invalid_position");
        if (unit.team != Team::Heroes && unit.team != Team::Enemies) errors.push_back(prefix + "invalid_team");
        if (unit.role < Role::Guardian || unit.role > Role::Raider) errors.push_back(prefix + "invalid_role");
        const auto slot = std::make_pair(unit.team, std::make_pair(int(unit.position.depth), int(unit.position.column)));
        if (!slots.insert(slot).second) errors.push_back(prefix + "duplicate_slot");
        heroes = heroes || unit.team == Team::Heroes;
        enemies = enemies || unit.team == Team::Enemies;
        if (unit.program.schemaVersion != kSimulationSchemaVersion || unit.program.id.empty())
            errors.push_back(prefix + "invalid_program_schema");
        std::set<std::string> ruleIds;
        std::set<std::string> abilityIds;
        if (unit.program.rules.size() > 64) errors.push_back(prefix + "too_many_rules");
        for (const Ability& ability : unit.abilities) {
            if (ability.id.empty() || !abilityIds.insert(ability.id).second) errors.push_back(prefix + "invalid_ability_id");
            if (ability.kind < ActionKind::Attack || ability.kind > ActionKind::Guard) errors.push_back(prefix + "invalid_ability_kind");
            if (ability.kind == ActionKind::Guard && (ability.power < 0 || ability.power > 100))
                errors.push_back(prefix + "invalid_guard_percent");
            if (ability.resourceCost < 0 || ability.cooldownTicks < 0 || ability.recoveryTicks < 0 ||
                ability.range < 0 || ability.power < 0 || ability.minimumDamage < 0)
                errors.push_back(prefix + "invalid_ability_parameters");
        }
        for (const Rule& rule : unit.program.rules) {
            if (rule.id.empty() || !ruleIds.insert(rule.id).second) errors.push_back(prefix + "invalid_rule_id");
            if (!abilityIds.count(rule.abilityId)) errors.push_back(prefix + "unknown_ability:" + rule.abilityId);
            const auto kind = rule.condition.kind;
            if (kind < ConditionKind::Always || kind > ConditionKind::SelfResourceAtLeast)
                errors.push_back(prefix + "unknown_condition");
            if ((kind == ConditionKind::SelfHpBelowPercent || kind == ConditionKind::AllyHpBelowPercent ||
                 kind == ConditionKind::EnemyHpBelowPercent) &&
                (rule.condition.value < 0 || rule.condition.value > 100))
                errors.push_back(prefix + "invalid_percent_threshold");
            if (kind == ConditionKind::SelfResourceAtLeast && rule.condition.value < 0)
                errors.push_back(prefix + "invalid_resource_threshold");
        }
    }
    if (!heroes || !enemies) errors.push_back("both_teams_required");
    std::sort(errors.begin(), errors.end());
    return errors;
}

BattleConfig Battle::MakePrototypeScenario() {
    const Ability strike{"ability.strike", ActionKind::Attack, 0, 0, 1, 5, 12, 1};
    const Ability guard{"ability.guard", ActionKind::Guard, 2, 3, 1, 0, 50, 0};
    const Ability heal{"ability.heal", ActionKind::Heal, 4, 2, 1, 6, 18, 0};
    const Ability spark{"ability.spark", ActionKind::Attack, 0, 0, 1, 5, 7, 1};
    const Ability bite{"ability.bite", ActionKind::Attack, 0, 1, 1, 5, 8, 1};

    Unit guardian;
    guardian.id = 10; guardian.name = "Guardian"; guardian.team = Team::Heroes; guardian.role = Role::Guardian;
    guardian.position = {0, 1}; guardian.hp = guardian.maxHp = 120;
    guardian.resource = guardian.maxResource = 10; guardian.resourceRegen = 1;
    guardian.abilities = {strike, guard};
    guardian.program = {"program.guardian", kSimulationSchemaVersion, {
        {"rule.guard.low_hp", 20, {ConditionKind::SelfHpBelowPercent, 35}, "ability.guard"},
        {"rule.guard.attack", 10, {ConditionKind::EnemyKnown, 0}, "ability.strike"},
    }};

    Unit healer;
    healer.id = 20; healer.name = "Healer"; healer.team = Team::Heroes; healer.role = Role::Healer;
    healer.position = {1, 1}; healer.hp = healer.maxHp = 80;
    healer.resource = healer.maxResource = 16; healer.resourceRegen = 1;
    healer.abilities = {heal, spark};
    healer.program = {"program.healer", kSimulationSchemaVersion, {
        {"rule.healer.ally", 20, {ConditionKind::AllyHpBelowPercent, 65}, "ability.heal"},
        {"rule.healer.attack", 10, {ConditionKind::EnemyKnown, 0}, "ability.spark"},
    }};

    BattleConfig config;
    config.units = {guardian, healer};
    for (std::uint32_t i = 0; i < 3; ++i) {
        Unit enemy;
        enemy.id = 100 + i; enemy.name = "Raider " + std::to_string(i + 1);
        enemy.team = Team::Enemies; enemy.role = Role::Raider;
        enemy.position = {0, static_cast<std::uint8_t>(i)};
        enemy.hp = enemy.maxHp = 45; enemy.abilities = {bite};
        enemy.program = {"program.raider", kSimulationSchemaVersion, {
            {"rule.raider.attack", 0, {ConditionKind::EnemyKnown, 0}, "ability.bite"},
        }};
        config.units.push_back(enemy);
    }
    return config;
}

const Unit* Battle::FindUnit(UnitId id) const {
    const auto it = std::lower_bound(units_.begin(), units_.end(), id,
        [](const Unit& unit, UnitId value) { return unit.id < value; });
    return it != units_.end() && it->id == id ? &*it : nullptr;
}
Unit* Battle::FindMutableUnit(UnitId id) {
    return const_cast<Unit*>(static_cast<const Battle*>(this)->FindUnit(id));
}
const Ability* Battle::FindAbility(const Unit& unit, const std::string& id) const {
    const auto it = std::find_if(unit.abilities.begin(), unit.abilities.end(),
        [&id](const Ability& ability) { return ability.id == id; });
    return it == unit.abilities.end() ? nullptr : &*it;
}

Snapshot Battle::BuildSnapshot(Tick tick, const std::vector<Unit>& units,
    const std::map<UnitId, std::map<UnitId, Knowledge>>& knowledge) {
    Snapshot snapshot;
    snapshot.tick = tick;
    snapshot.units = units;
    std::sort(snapshot.units.begin(), snapshot.units.end(), [](const Unit& a, const Unit& b) { return a.id < b.id; });
    snapshot.knowledge = knowledge;
    return snapshot;
}

void Battle::UpdatePerception() {
    // Prototype sensor: the small arena grants fresh observations of living opponents.
    // This is an explicit scenario assumption, not a claim that final LOS/scanning is implemented.
    for (const Unit& observer : units_) {
        if (!IsAlive(observer)) continue;
        auto& entries = knowledge_[observer.id];
        for (const Unit& target : units_) {
            if (!IsAlive(target) || target.team == observer.team) continue;
            entries[target.id] = Knowledge{target.id, target.position, target.hp, target.maxHp, tick_, true};
        }
        // The prototype arena observes defeats immediately; do not keep shifting dead targets.
        for (auto it = entries.begin(); it != entries.end();) {
            const Unit* target = FindUnit(it->first);
            if (!target || !IsAlive(*target)) it = entries.erase(it); else ++it;
        }
    }
}

void Battle::Emit(TickResult& result, UnitId source, UnitId target, const char* kind, std::int32_t amount) {
    result.events.push_back({tick_, nextSequence_++, source, target, kind, amount});
}

void Battle::ResolvePendingActions(TickResult& result) {
    std::stable_sort(pending_.begin(), pending_.end(), [](const PendingAction& a, const PendingAction& b) {
        if (a.commitTick != b.commitTick) return a.commitTick < b.commitTick;
        if (a.source != b.source) return a.source < b.source;
        return a.sequence < b.sequence;
    });
    std::vector<PendingAction> remaining;
    for (const PendingAction& action : pending_) {
        if (action.commitTick > tick_) { remaining.push_back(action); continue; }
        Unit* source = FindMutableUnit(action.source);
        const Ability* ability = source ? FindAbility(*source, action.abilityId) : nullptr;
        if (!source || !IsAlive(*source) || !ability) {
            if (source) Emit(result, source->id, action.target, "cancelled", 0);
            continue;
        }
        Unit* target = FindMutableUnit(action.target);
        if (ability->kind != ActionKind::Guard && (!target || !IsAlive(*target))) {
            Emit(result, source->id, action.target, "cancelled_before_commit", 0);
            continue;
        }
        const auto ready = source->readyAtTick.find(ability->id);
        if (source->resource < ability->resourceCost ||
            (ready != source->readyAtTick.end() && tick_ < ready->second)) {
            Emit(result, source->id, action.target, "cancelled_before_commit", 0);
            continue;
        }
        // COMMIT: pay once, then resolve. No cost is paid before this tick.
        source->resource -= ability->resourceCost;
        source->readyAtTick[ability->id] = tick_ + static_cast<Tick>(ability->cooldownTicks);
        Emit(result, source->id, action.target, "commit", ability->resourceCost);
        if (ability->kind == ActionKind::Attack) {
            int damage = std::max(ability->minimumDamage, ability->power);
            if (target->guardUntilTick >= tick_) {
                damage = std::max(ability->minimumDamage,
                    static_cast<int>((static_cast<std::int64_t>(damage) * (100 - target->guardPercent)) / 100));
            }
            damage = std::min(damage, target->hp);
            target->hp -= damage;
            Emit(result, source->id, target->id, "damage", damage);
        } else if (ability->kind == ActionKind::Heal) {
            const int amount = std::min(ability->power, target->maxHp - target->hp);
            target->hp += amount;
            Emit(result, source->id, target->id, "heal", amount);
        } else {
            source->guardUntilTick = tick_ + 1;
            source->guardPercent = ability->power;
            Emit(result, source->id, source->id, "guard", ability->power);
        }
    }
    pending_ = std::move(remaining);
    for (const Unit& unit : units_) {
        if (unit.hp == 0 && downReported_.insert(unit.id).second)
            Emit(result, unit.id, unit.id, "down", 0);
    }
}

Truth Battle::EvaluateCondition(const Unit& actor, const Condition& condition, const Snapshot& snapshot) {
    static const std::map<UnitId, Knowledge> kNoKnowledge;
    const auto entriesIt = snapshot.knowledge.find(actor.id);
    const auto& entries = entriesIt != snapshot.knowledge.end() ? entriesIt->second : kNoKnowledge;
    switch (condition.kind) {
        case ConditionKind::Always: return Truth::True;
        case ConditionKind::SelfHpBelowPercent:
            return PercentHp(actor) < condition.value ? Truth::True : Truth::False;
        case ConditionKind::SelfResourceAtLeast:
            return actor.resource >= condition.value ? Truth::True : Truth::False;
        case ConditionKind::AllyHpBelowPercent:
            for (const Unit& unit : snapshot.units)
                if (IsAlive(unit) && unit.team == actor.team && PercentHp(unit) < condition.value) return Truth::True;
            return Truth::False;
        case ConditionKind::EnemyKnown:
            for (const auto& entry : entries)
                if (IsFresh(entry.second, snapshot.tick)) return Truth::True;
            return Truth::Unknown;
        case ConditionKind::EnemyHpBelowPercent: {
            bool unknown = false;
            for (const auto& entry : entries) {
                if (!IsFresh(entry.second, snapshot.tick)) { unknown = true; continue; }
                if (entry.second.hp > 0 &&
                    static_cast<std::int64_t>(entry.second.hp) * 100 <
                    static_cast<std::int64_t>(condition.value) * entry.second.maxHp)
                    return Truth::True;
            }
            return unknown ? Truth::Unknown : Truth::False;
        }
    }
    return Truth::Unknown;
}

UnitId Battle::SelectTarget(const Unit& actor, const Ability& ability, const Snapshot& snapshot) {
    if (ability.kind == ActionKind::Guard) return actor.id;
    UnitId best = 0;
    const Unit* bestUnit = nullptr;
    int bestMetric = 0;
    int bestDistance = 0;

    const auto consider = [&](const Unit& candidate, const Knowledge& known, bool useKnownHp) {
        if (useKnownHp ? known.hp <= 0 : !IsAlive(candidate)) return;
        if (ability.kind == ActionKind::Heal &&
            (candidate.team != actor.team || candidate.hp >= candidate.maxHp)) return;
        const bool sameTeam = actor.team == candidate.team;
        const int distance = LogicalDistance(actor, known.position, sameTeam);
        if (distance > ability.range) return;
        const int metric = ability.kind == ActionKind::Heal ? PercentHp(candidate) : known.hp;
        if (!bestUnit || metric < bestMetric ||
            (metric == bestMetric && (distance < bestDistance ||
             (distance == bestDistance && candidate.id < best)))) {
            best = candidate.id;
            bestUnit = &candidate;
            bestMetric = metric;
            bestDistance = distance;
        }
    };

    if (ability.kind == ActionKind::Attack) {
        // Attack candidates come only from the observer's fresh knowledge, never from hidden world HP.
        const auto observer = snapshot.knowledge.find(actor.id);
        if (observer == snapshot.knowledge.end()) return 0;
        for (const auto& entry : observer->second) {
            if (!IsFresh(entry.second, snapshot.tick) || entry.second.hp <= 0) continue;
            const auto unitIt = std::find_if(snapshot.units.begin(), snapshot.units.end(),
                [id=entry.first](const Unit& unit) { return unit.id == id; });
            if (unitIt == snapshot.units.end() || unitIt->team == actor.team) continue;
            consider(*unitIt, entry.second, true);
        }
        return best;
    }

    for (const Unit& candidate : snapshot.units) {
        if (candidate.team != actor.team) continue;
        const Knowledge known{candidate.id, candidate.position, candidate.hp, candidate.maxHp, snapshot.tick, true};
        consider(candidate, known, false);
    }
    return best;
}

std::string Battle::AvailabilityReason(const Unit& actor, const Ability& ability, UnitId target) const {
    if (!IsAlive(actor)) return "actor_down";
    if (actor.busyUntilTick > snapshot_.tick) return "recovering";
    const auto ready = actor.readyAtTick.find(ability.id);
    if (ready != actor.readyAtTick.end() && snapshot_.tick < ready->second) return "cooldown";
    if (actor.resource < ability.resourceCost) return "resource";
    if (ability.kind == ActionKind::Guard) return "";
    if (target == 0) return "no_known_valid_target";
    const Unit* unit = FindUnit(target);
    if (!unit || !IsAlive(*unit)) return "target_down";
    if (ability.kind == ActionKind::Attack && unit->team == actor.team) return "invalid_target_team";
    if (ability.kind == ActionKind::Heal && (unit->team != actor.team || unit->hp >= unit->maxHp)) return "invalid_heal_target";
    if (ability.kind == ActionKind::Attack) {
        const auto observer = snapshot_.knowledge.find(actor.id);
        if (observer == snapshot_.knowledge.end()) return "target_unknown";
        const auto entry = observer->second.find(target);
        if (entry == observer->second.end() || !IsFresh(entry->second, snapshot_.tick)) return "target_unknown_or_stale";
        if (LogicalDistance(actor, entry->second.position, false) > ability.range) return "out_of_range";
    } else if (LogicalDistance(actor, *unit) > ability.range) return "out_of_range";
    return "";
}

void Battle::Declare(Unit& actor, const Ability& ability, UnitId target,
    const std::string& ruleId, bool fallback, TickResult& result) {
    const auto sequence = nextSequence_++;
    const Tick commitTick = tick_ + 1;
    pending_.push_back({sequence, actor.id, target, ability.id, commitTick});
    actor.busyUntilTick = commitTick + static_cast<Tick>(ability.recoveryTicks);
    result.decisions.push_back({tick_, actor.id, ruleId, ability.id, target, "selected", true, fallback});
    Emit(result, actor.id, target, "declare", 0);
}

bool Battle::TrySelect(Unit& actor, const Ability& ability, const std::string& ruleId,
    bool fallback, TickResult& result) {
    const UnitId target = SelectTarget(actor, ability, snapshot_);
    const std::string reason = AvailabilityReason(actor, ability, target);
    if (!reason.empty()) {
        result.decisions.push_back({tick_, actor.id, ruleId, ability.id, target, reason, false, fallback});
        return false;
    }
    Declare(actor, ability, target, ruleId, fallback, result);
    return true;
}

void Battle::RunFallback(Unit& actor, TickResult& result) {
    // R-014/R-015 are PROPOSED role algorithms; U-022 confirms role-dependence only.
    std::vector<ActionKind> preference;
    if (actor.role == Role::Guardian) preference = {ActionKind::Guard, ActionKind::Attack};
    else if (actor.role == Role::Healer) preference = {ActionKind::Heal, ActionKind::Attack};
    else preference = {ActionKind::Attack};
    for (const ActionKind kind : preference) {
        for (const Ability& ability : actor.abilities) {
            if (ability.kind == kind &&
                TrySelect(actor, ability, "fallback." + std::string(ToString(actor.role)), true, result)) return;
        }
    }
    result.decisions.push_back({tick_, actor.id, "fallback." + std::string(ToString(actor.role)), "", 0,
        "no_available_fallback", false, true});
}

void Battle::Decide(TickResult& result) {
    // Stable ID is authoritative; rule priority is local to each program.
    for (const Unit& immutableActor : snapshot_.units) {
        if (!IsAlive(immutableActor) || immutableActor.busyUntilTick > snapshot_.tick) continue;
        Unit* actor = FindMutableUnit(immutableActor.id);
        std::vector<std::pair<int, std::size_t>> rules;
        for (std::size_t i = 0; i < immutableActor.program.rules.size(); ++i)
            rules.emplace_back(-immutableActor.program.rules[i].priority, i);
        std::stable_sort(rules.begin(), rules.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
        bool selected = false;
        for (const auto& entry : rules) {
            const Rule& rule = immutableActor.program.rules[entry.second];
            const Truth truth = EvaluateCondition(immutableActor, rule.condition, snapshot_);
            if (truth != Truth::True) {
                result.decisions.push_back({tick_, actor->id, rule.id, rule.abilityId, 0,
                    truth == Truth::Unknown ? "condition_unknown_as_false" : "condition_false", false, false});
                continue;
            }
            const Ability* ability = FindAbility(immutableActor, rule.abilityId);
            if (!ability) {
                result.decisions.push_back({tick_, actor->id, rule.id, rule.abilityId, 0, "unknown_ability", false, false});
                continue;
            }
            if (TrySelect(*actor, *ability, rule.id, false, result)) { selected = true; break; }
        }
        if (!selected) RunFallback(*actor, result);
    }
}

void Battle::UpdateOutcome() {
    bool heroes = false;
    bool enemies = false;
    for (const Unit& unit : units_) {
        heroes = heroes || (unit.team == Team::Heroes && IsAlive(unit));
        enemies = enemies || (unit.team == Team::Enemies && IsAlive(unit));
    }
    if (!heroes && !enemies) outcome_ = Outcome::Draw;
    else if (!enemies) outcome_ = Outcome::HeroesWin;
    else if (!heroes) outcome_ = Outcome::EnemiesWin;
    else if (tick_ >= config_.maxTicks) outcome_ = Outcome::Draw;
}

TickResult Battle::Step() {
    TickResult result;
    result.tick = tick_;
    if (!errors_.empty()) {
        result.decisions.push_back({tick_, 0, "", "", 0, "invalid_scenario", false, false});
        return result;
    }
    if (outcome_ != Outcome::Running) return result;
    ++tick_;
    result.tick = tick_;
    // Confirmed U-021: resolve current actions/effects first, then snapshot, then DecisionPoint.
    ResolvePendingActions(result);
    for (Unit& unit : units_) {
        if (IsAlive(unit)) unit.resource = std::min(unit.maxResource, unit.resource + unit.resourceRegen);
    }
    UpdateOutcome();
    UpdatePerception();
    snapshot_ = BuildSnapshot(tick_, units_, knowledge_);
    if (outcome_ == Outcome::Running) Decide(result);
    UpdateOutcome();
    result.outcome = outcome_;
    return result;
}

} // namespace codex
