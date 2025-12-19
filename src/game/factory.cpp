#include "../../include/game/factory.hpp"
#include "../../include/npc/knight.hpp"
#include "../../include/npc/elf.hpp"
#include "../../include/npc/dragon.hpp"
#include "../../include/game/constants.hpp"
#include <stdexcept>
#include <random>
#include <sstream>

std::unique_ptr<NPC> CreatureFactory::createCreature(NPCType type) {
  return createCreature(type, 
                       (ArenaConfig::WORLD_MAX_X - ArenaConfig::WORLD_MIN_X) / 2,
                       (ArenaConfig::WORLD_MAX_Y - ArenaConfig::WORLD_MIN_Y) / 2,
                       generateCreatureName(type));
}

std::unique_ptr<NPC> CreatureFactory::createCreature(NPCType type, double x, double y, 
                                                     const std::string& name) {
  if (!validatePosition(x, y)) {
    std::string error = "Координаты (" + std::to_string(x) + ", " + 
                       std::to_string(y) + ") вне диапазона [" +
                       std::to_string(ArenaConfig::WORLD_MIN_X) + ", " +
                       std::to_string(ArenaConfig::WORLD_MAX_X) + "]";
    throw std::invalid_argument(error);
  }

  switch (type) {
    case NPCType::KNIGHT: 
      return std::make_unique<Knight>(x, y, name.empty() ? generateCreatureName(type) : name);
    case NPCType::ELF: 
      return std::make_unique<Elf>(x, y, name.empty() ? generateCreatureName(type) : name);
    case NPCType::DRAGON: 
      return std::make_unique<Dragon>(x, y, name.empty() ? generateCreatureName(type) : name);
    default: 
      throw std::invalid_argument("Неизвестный тип существа");
  }
}

std::unique_ptr<NPC> CreatureFactory::createRandomCreature() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<int> typeDist(1, 3);
  std::uniform_real_distribution<double> xDist(ArenaConfig::WORLD_MIN_X, 
                                              ArenaConfig::WORLD_MAX_X);
  std::uniform_real_distribution<double> yDist(ArenaConfig::WORLD_MIN_Y, 
                                              ArenaConfig::WORLD_MAX_Y);
  
  NPCType type = static_cast<NPCType>(typeDist(gen));
  return createCreature(type, xDist(gen), yDist(gen), generateCreatureName(type));
}

std::unique_ptr<NPC> CreatureFactory::createCreatureAtEdge(NPCType type, 
                                                           const std::string& name) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<double> coordDist(0, 
      std::max(ArenaConfig::WORLD_MAX_X, ArenaConfig::WORLD_MAX_Y));
  
  double x, y;
  int edge = std::uniform_int_distribution<int>(0, 3)(gen);
  
  switch (edge) {
    case 0: // Верхний край
      x = coordDist(gen);
      y = ArenaConfig::WORLD_MAX_Y;
      break;
    case 1: // Правый край
      x = ArenaConfig::WORLD_MAX_X;
      y = coordDist(gen);
      break;
    case 2: // Нижний край
      x = coordDist(gen);
      y = ArenaConfig::WORLD_MIN_Y;
      break;
    case 3: // Левый край
      x = ArenaConfig::WORLD_MIN_X;
      y = coordDist(gen);
      break;
  }
  
  return createCreature(type, x, y, name.empty() ? generateCreatureName(type) : name);
}

std::unique_ptr<NPC> CreatureFactory::createCreatureAtCenter(NPCType type,
                                                             const std::string& name) {
  double centerX = (ArenaConfig::WORLD_MAX_X - ArenaConfig::WORLD_MIN_X) / 2;
  double centerY = (ArenaConfig::WORLD_MAX_Y - ArenaConfig::WORLD_MIN_Y) / 2;
  
  return createCreature(type, centerX, centerY, name.empty() ? generateCreatureName(type) : name);
}

std::unique_ptr<NPC> CreatureFactory::loadCreatureFromFile(std::ifstream& file) {
  std::string typeStr;
  std::string name;
  double x, y;
  
  if (file >> typeStr >> x >> y >> name) {
    NPCType type = convertTypeFromString(typeStr);
    return createCreature(type, x, y, name);
  }
  return nullptr;
}

std::unique_ptr<NPC> CreatureFactory::parseCreatureData(const std::string& data) {
  std::stringstream ss(data);
  std::string typeStr, name;
  double x, y;
  
  if (ss >> typeStr >> x >> y >> name) {
    NPCType type = convertTypeFromString(typeStr);
    return createCreature(type, x, y, name);
  }
  
  return nullptr;
}

std::vector<std::unique_ptr<NPC>> CreatureFactory::createCreatureSwarm(NPCType type, 
                                                                       int count) {
  std::vector<std::unique_ptr<NPC>> swarm;
  swarm.reserve(count);
  
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> xDist(ArenaConfig::WORLD_MIN_X, 
                                              ArenaConfig::WORLD_MAX_X);
  std::uniform_real_distribution<double> yDist(ArenaConfig::WORLD_MIN_Y, 
                                              ArenaConfig::WORLD_MAX_Y);
  
  for (int i = 0; i < count; ++i) {
    swarm.push_back(createCreature(type, xDist(gen), yDist(gen), 
                                  generateCreatureName(type) + "_" + std::to_string(i+1)));
  }
  
  return swarm;
}

std::vector<std::unique_ptr<NPC>> CreatureFactory::createRandomSwarm(int count) {
  std::vector<std::unique_ptr<NPC>> swarm;
  swarm.reserve(count);
  
  for (int i = 0; i < count; ++i) {
    swarm.push_back(createRandomCreature());
  }
  
  return swarm;
}

std::string CreatureFactory::generateCreatureName(NPCType type) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<int> idDist(1000, 9999);
  
  std::string base;
  switch (type) {
    case NPCType::KNIGHT: base = "Рыцарь_"; break;
    case NPCType::ELF: base = "Эльф_"; break;
    case NPCType::DRAGON: base = "Дракон_"; break;
    default: base = "Существо_";
  }
  
  return base + std::to_string(idDist(gen));
}

bool CreatureFactory::validatePosition(double x, double y) {
  return (x >= ArenaConfig::WORLD_MIN_X && x <= ArenaConfig::WORLD_MAX_X &&
          y >= ArenaConfig::WORLD_MIN_Y && y <= ArenaConfig::WORLD_MAX_Y);
}

std::mt19937& CreatureFactory::getRandomEngine() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  return gen;
}

std::string CreatureFactory::getRandomKnightName() {
  static const std::vector<std::string> names = {
    "Артур", "Ланселот", "Гавейн", "Тристан", "Персиваль", "Галахад", "Борс"
  };
  static std::uniform_int_distribution<int> dist(0, names.size() - 1);
  return names[dist(getRandomEngine())];
}

std::string CreatureFactory::getRandomElfName() {
  static const std::vector<std::string> names = {
    "Леголас", "Элронд", "Галадриэль", "Арвен", "Трандуил", "Хальдир", "Эрестор"
  };
  static std::uniform_int_distribution<int> dist(0, names.size() - 1);
  return names[dist(getRandomEngine())];
}

std::string CreatureFactory::getRandomDragonName() {
  static const std::vector<std::string> names = {
    "Смауг", "Дрого", "Визерион", "Рейгаль", "Балион", "Анкалагон", "Глаурунг"
  };
  static std::uniform_int_distribution<int> dist(0, names.size() - 1);
  return names[dist(getRandomEngine())];
}