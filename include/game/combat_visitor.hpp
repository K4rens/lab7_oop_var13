#ifndef COMBAT_VISITOR_HPP
#define COMBAT_VISITOR_HPP

#include "../npc/npc.hpp"
#include "./observer.hpp"
#include <vector>
#include <memory>
#include <random>
#include <mutex>

enum BattleOutcome {
  ATTACKER_VICTORY,
  DEFENDER_VICTORY,
  NO_CONTEST,
  MUTUAL_DESTRUCTION
};

class CombatMediator {
  private:
    const std::vector<std::unique_ptr<NPC>>& combatants_;
    const std::vector<Observer*>& monitors_;
    mutable std::mutex combatLogMutex_;
    
    void logBattleResult(NPC& victor, NPC& defeated) const;
    void logMovement(NPC& creature, MoveDirection path) const;
    void rollDice(std::mt19937& generator, int& attackerRoll, int& defenderRoll) const;
    
  public:
    CombatMediator(const std::vector<std::unique_ptr<NPC>>& participants, 
                   const std::vector<Observer*>& watchers);
    
    BattleOutcome engage(NPC& attacker, NPC& defender);
    void relocate(NPC& creature, MoveDirection direction);
    
    // Симуляция расширенного боя
    struct CombatReport {
        bool combatOccurred;
        std::string attackerName;
        std::string defenderName;
        int attackerRoll;
        int defenderRoll;
        bool attackerSurvived;
        bool defenderSurvived;
    };
    
    CombatReport simulateEngagement(NPC& attacker, NPC& defender);
};

#endif