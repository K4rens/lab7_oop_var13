#include "../../include/game/combat_visitor.hpp"
#include "../../include/game/constants.hpp"
#include <random>

CombatMediator::CombatMediator(const std::vector<std::unique_ptr<NPC>>& participants, 
                               const std::vector<Observer*>& watchers):
  combatants_(participants), monitors_(watchers) {}

void CombatMediator::rollDice(std::mt19937& generator, int& attackerRoll, int& defenderRoll) const {
  std::uniform_int_distribution<int> dice(1, ArenaConfig::Combat::ATTACK_DICE_SIDES);
  attackerRoll = dice(generator);
  defenderRoll = dice(generator);
}

void CombatMediator::logBattleResult(NPC& victor, NPC& defeated) const {
  std::lock_guard lock(combatLogMutex_);
  for (auto monitor : monitors_) {
    monitor->recordBattle(victor, defeated);
  }
}

void CombatMediator::logMovement(NPC& creature, MoveDirection path) const {
  std::lock_guard lock(combatLogMutex_);
  for (auto monitor : monitors_) {
    monitor->recordMovement(creature, path);
  }
}

BattleOutcome CombatMediator::engage(NPC& attacker, NPC& defender) {
  if (!attacker.isAlive() || !defender.isAlive()) {
    return NO_CONTEST;
  }
  
  std::random_device rd;
  std::mt19937 gen(rd());
  int attackerPower, defenderPower;
  rollDice(gen, attackerPower, defenderPower);
  
  if (attacker.canKill(defender) && attackerPower > defenderPower) {
    defender.setAlive(false);
    logBattleResult(attacker, defender);
    return ATTACKER_VICTORY;
  }
  
  if (defender.canKill(attacker) && defenderPower > attackerPower) {
    attacker.setAlive(false);
    logBattleResult(defender, attacker);
    return DEFENDER_VICTORY;
  }
  
  // Ничья или оба промахнулись
  return NO_CONTEST;
}

void CombatMediator::relocate(NPC& creature, MoveDirection direction) {
  if (creature.isAlive()) {
    creature.move(direction);
    logMovement(creature, direction);
  }
}

CombatMediator::CombatReport CombatMediator::simulateEngagement(NPC& attacker, NPC& defender) {
  CombatReport report;
  report.attackerName = attacker.getName();
  report.defenderName = defender.getName();
  report.combatOccurred = false;
  report.attackerSurvived = attacker.isAlive();
  report.defenderSurvived = defender.isAlive();
  
  if (!attacker.isAlive() || !defender.isAlive()) {
    return report;
  }
  
  std::random_device rd;
  std::mt19937 gen(rd());
  rollDice(gen, report.attackerRoll, report.defenderRoll);
  
  if (attacker.canKill(defender) && report.attackerRoll > report.defenderRoll) {
    report.combatOccurred = true;
    report.defenderSurvived = false;
  } else if (defender.canKill(attacker) && report.defenderRoll > report.attackerRoll) {
    report.combatOccurred = true;
    report.attackerSurvived = false;
  }
  
  return report;
}