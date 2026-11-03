#pragma once

#include "SimulationCore/Types.hpp"

#include <set>

namespace codex {

class Battle {
public:
    explicit Battle(BattleConfig config);

    static std::vector<std::string> Validate(const BattleConfig& config);
    static BattleConfig MakePrototypeScenario();
    static Snapshot BuildSnapshot(Tick tick, const std::vector<Unit>& units,
        const std::map<UnitId, std::map<UnitId, Knowledge>>& knowledge);
    static Truth EvaluateCondition(const Unit& actor, const Condition& condition, const Snapshot& snapshot);
    static UnitId SelectTarget(const Unit& actor, const Ability& ability, const Snapshot& snapshot);

    TickResult Step();
    const Snapshot& GetSnapshot() const { return snapshot_; }
    const std::vector<std::string>& GetErrors() const { return errors_; }
    Outcome GetOutcome() const { return outcome_; }
    Tick GetTick() const { return tick_; }

private:
    const Unit* FindUnit(UnitId id) const;
    Unit* FindMutableUnit(UnitId id);
    const Ability* FindAbility(const Unit& unit, const std::string& id) const;
    void ResolvePendingActions(TickResult& result);
    void UpdatePerception();
    void Decide(TickResult& result);
    std::string AvailabilityReason(const Unit& actor, const Ability& ability, UnitId target) const;
    bool TrySelect(Unit& actor, const Ability& ability, const std::string& ruleId,
        bool fallback, TickResult& result);
    void RunFallback(Unit& actor, TickResult& result);
    void Declare(Unit& actor, const Ability& ability, UnitId target,
        const std::string& ruleId, bool fallback, TickResult& result);
    void Emit(TickResult& result, UnitId source, UnitId target, const char* kind, std::int32_t amount);
    void UpdateOutcome();

    BattleConfig config_;
    std::vector<std::string> errors_;
    std::vector<Unit> units_;
    std::map<UnitId, std::map<UnitId, Knowledge>> knowledge_;
    std::vector<PendingAction> pending_;
    std::set<UnitId> downReported_;
    Snapshot snapshot_;
    Tick tick_ = 0;
    std::uint64_t nextSequence_ = 1;
    Outcome outcome_ = Outcome::Running;
};

} // namespace codex
