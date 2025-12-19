#include "../../include/npc/npc.hpp"
#include "../../include/npc/knight.hpp"
#include "../../include/game/constants.hpp"

Knight::Knight(): NPC(NPCType::KNIGHT) {}

Knight::Knight(double x, double y, const std::string &name):
  NPC(NPCType::KNIGHT, x, y, name, 
      ArenaConfig::Mobility::KNIGHT_STEP, 
      ArenaConfig::Combat::KNIGHT_SWORD_REACH) {}

Knight::Knight(double x, double y):
  NPC(NPCType::KNIGHT, x, y, generateRandomName(NPCType::KNIGHT),
      ArenaConfig::Mobility::KNIGHT_STEP,
      ArenaConfig::Combat::KNIGHT_SWORD_REACH) {}

std::string Knight::getTitle() const {
  static const std::vector<std::string> titles = {
    "Сэр", "Лорд", "Барон", "Герцог", "Граф", "Витязь", "Паладин"
  };
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(0, titles.size() - 1);
  
  return titles[dist(gen)];
}

bool Knight::canDefend() const {
  return true; // Рыцари всегда могут защищаться
}