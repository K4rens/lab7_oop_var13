#include "../../include/npc/npc.hpp"
#include "../../include/npc/dragon.hpp"
#include "../../include/game/constants.hpp"

Dragon::Dragon(): NPC(NPCType::DRAGON) {}

Dragon::Dragon(double x, double y, const std::string &name): 
  NPC(NPCType::DRAGON, x, y, name,
      ArenaConfig::Mobility::DRAGON_STEP,
      ArenaConfig::Combat::DRAGON_BREATH_RANGE) {}

Dragon::Dragon(double x, double y):
  NPC(NPCType::DRAGON, x, y, generateRandomName(NPCType::DRAGON),
      ArenaConfig::Mobility::DRAGON_STEP,
      ArenaConfig::Combat::DRAGON_BREATH_RANGE) {}

std::string Dragon::getColor() const {
  static const std::vector<std::string> colors = {
    "Красный", "Зеленый", "Синий", "Черный", "Золотой", "Серебряный", "Бронзовый"
  };
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(0, colors.size() - 1);
  
  return colors[dist(gen)];
}

bool Dragon::canFly() const {
  return true; // Драконы умеют летать
}

double Dragon::getFireRange() const {
  return getAttackRange() * 1.5; // Дальность огненного дыхания больше обычной
}