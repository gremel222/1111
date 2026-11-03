#include "SimulationCore/Battle.hpp"

#include <iostream>

int main() {
    codex::Battle battle(codex::Battle::MakePrototypeScenario());
    if (!battle.GetErrors().empty()) {
        for (const auto& error : battle.GetErrors()) std::cerr << "INVALID: " << error << '\n';
        return 1;
    }
    std::cout << "The Codex of Rules — native SimulationCore demo\n"
              << "2 heroes vs 3 raiders; fixed tick = 250 ms; no Unreal required.\n\n";
    while (battle.GetOutcome() == codex::Outcome::Running) {
        const codex::TickResult result = battle.Step();
        std::cout << "Tick " << result.tick << " (" << result.tick * 0.25 << " s)\n";
        for (const auto& event : result.events) {
            if (event.kind == "damage" || event.kind == "heal" || event.kind == "down" || event.kind == "guard")
                std::cout << "  EVENT " << event.kind << " source=" << event.source
                          << " target=" << event.target << " amount=" << event.amount << '\n';
        }
        for (const auto& decision : result.decisions) {
            if (decision.selected || decision.fallback)
                std::cout << "  AI unit=" << decision.actor << " rule=" << decision.ruleId
                          << " ability=" << decision.abilityId << " target=" << decision.target
                          << " reason=" << decision.reason << (decision.fallback ? " [fallback]" : "") << '\n';
        }
    }
    std::cout << "\nOutcome: " << codex::ToString(battle.GetOutcome())
              << " at tick " << battle.GetTick() << '\n';
    for (const auto& unit : battle.GetSnapshot().units)
        std::cout << "  " << unit.name << " hp=" << unit.hp << '/' << unit.maxHp << '\n';
    return 0;
}
