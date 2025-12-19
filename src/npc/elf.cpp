#include "../../include/npc/npc.hpp"
#include "../../include/npc/elf.hpp"
#include "../../include/game/constants.hpp"

Elf::Elf(): NPC(NPCType::ELF) {}

Elf::Elf(double x, double y, const std::string &name): 
  NPC(NPCType::ELF, x, y, name,
      ArenaConfig::Mobility::ELF_STEP,
      ArenaConfig::Combat::ELF_BOW_RANGE) {}

Elf::Elf(double x, double y):
  NPC(NPCType::ELF, x, y, generateRandomName(NPCType::ELF),
      ArenaConfig::Mobility::ELF_STEP,
      ArenaConfig::Combat::ELF_BOW_RANGE) {}

std::string Elf::getClan() const {
  static const std::vector<std::string> clans = {
    "Лунный", "Лесной", "Речной", "Горный", "Звездный", "Теневой"
  };
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(0, clans.size() - 1);
  
  return clans[dist(gen)];
}

bool Elf::canUseMagic() const {
  return true; // Эльфы могут использовать магию
}