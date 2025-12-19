#include "../../include/game/dungeon_master.hpp"
#include "../../include/game/constants.hpp"
#include <fstream>
#include <string>
#include <random>
#include <algorithm>
#include <iomanip>

DungeonMaster::DungeonMaster() {
  watchers_.push_back(new ConsoleDisplay());
  watchers_.push_back(new FileRecorder());
}

DungeonMaster::~DungeonMaster() {
  for (size_t i = 0; i != watchers_.size(); ++i) {
    delete watchers_[i];
  }
}

bool DungeonMaster::validateCoordinates(double x, double y) const {
  return (x >= ArenaConfig::WORLD_MIN_X && x <= ArenaConfig::WORLD_MAX_X &&
          y >= ArenaConfig::WORLD_MIN_Y && y <= ArenaConfig::WORLD_MAX_Y);
}

void DungeonMaster::broadcastEvent(const std::string& event) const {
  for (auto& watcher : watchers_) {
    watcher->recordGameEvent(event);
  }
}

void DungeonMaster::initializeCreatures(int count) {
  std::unique_lock lock(creatureMutex_);
  creatures_.reserve(count);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> x_dist(ArenaConfig::WORLD_MIN_X, 
                                               ArenaConfig::WORLD_MAX_X);
  std::uniform_real_distribution<double> y_dist(ArenaConfig::WORLD_MIN_Y, 
                                               ArenaConfig::WORLD_MAX_Y);

  for (int i = 0; i < count; ++i) {
    NPCType type = static_cast<NPCType>(1 + (i % 3));
    double x = x_dist(gen);
    double y = y_dist(gen);
    std::string name = "NPC_" + std::to_string(i + 1);
    
    spawnCreature(type, x, y, name);
  }
  
  broadcastEvent("Инициализировано " + std::to_string(count) + " существ");
}

void DungeonMaster::spawnCreature(NPCType type, double x, double y, 
                                  const std::string& name) {
  std::unique_lock lock(creatureMutex_);
  if (!validateCoordinates(x, y)) {
    throw std::invalid_argument("Координаты вне игрового мира");
  }
  
  creatures_.push_back(CreatureFactory::createCreature(type, x, y, name));
  broadcastEvent("Создано существо: " + name + " (" + 
                 convertTypeFromString(std::to_string(static_cast<int>(type))) + ")");
}

void DungeonMaster::spawnCreature(const std::string& type, double x, double y, 
                                  const std::string& name) {
  spawnCreature(convertTypeFromString(type), x, y, name);
}

void DungeonMaster::relocateCreature(size_t index, MoveDirection direction) {
  std::unique_lock lock(creatureMutex_);
  if (index < creatures_.size() && creatures_[index]->isAlive()) {
    creatures_[index]->move(direction);
  }
}

void DungeonMaster::loadScenario(const std::string& fileName) {
  std::unique_lock lock(creatureMutex_);
  std::ifstream in(fileName);
  
  if (!in.is_open()) {
    throw std::invalid_argument("Не удалось открыть файл для чтения");
  }
  
  std::unique_ptr<NPC> creature;
  while ((creature = CreatureFactory::loadCreatureFromFile(in)) != nullptr) {
    creatures_.push_back(std::move(creature));
  }
  
  in.close();
  broadcastEvent("Загружен сценарий из файла: " + fileName);
}

void DungeonMaster::saveScenario(const std::string& fileName) const {
  std::shared_lock lock(creatureMutex_);
  std::ofstream out(fileName);
  
  if (!out.is_open()) {
    throw std::invalid_argument("Не удалось открыть файл для записи");
  }
  
  for (const auto& creature : creatures_) {
    creature->save(out);
  }
  
  out.close();
  broadcastEvent("Сценарий сохранен в файл: " + fileName);
}

void DungeonMaster::displayCreature(const std::string& name) const {
  std::shared_lock lock(creatureMutex_);
  for (const auto& creature : creatures_) {
    if (creature->getName() == name) {
      creature->display();
      return;
    }
  }
  std::cout << "Существо с именем " << name << " не найдено\n";
}

void DungeonMaster::displayAllCreatures() const {
  std::shared_lock lock(creatureMutex_);
  std::cout << "\n=== ВСЕ СУЩЕСТВА ===\n";
  for (const auto& creature : creatures_) {
    creature->display();
  }
  std::cout << "===================\n";
}

void DungeonMaster::displayLivingCreatures() const {
  std::shared_lock lock(creatureMutex_);
  std::cout << "\n=== ВЫЖИВШИЕ СУЩЕСТВА ===\n";
  for (const auto& creature : creatures_) {
    if (creature->isAlive()) {
      creature->display();
    }
  }
  std::cout << "========================\n";
}

void DungeonMaster::renderMap() const {
  std::shared_lock lock(creatureMutex_);
  
  const int width = 50;
  const int height = 20;
  const double cellWidth = (ArenaConfig::WORLD_MAX_X - ArenaConfig::WORLD_MIN_X) / width;
  const double cellHeight = (ArenaConfig::WORLD_MAX_Y - ArenaConfig::WORLD_MIN_Y) / height;
  
  std::vector<std::vector<char>> map(height, std::vector<char>(width, '.'));
  
  for (const auto& creature : creatures_) {
    if (creature->isAlive()) {
      int x = static_cast<int>((creature->getX() - ArenaConfig::WORLD_MIN_X) / cellWidth);
      int y = static_cast<int>((creature->getY() - ArenaConfig::WORLD_MIN_Y) / cellHeight);
      
      if (x >= 0 && x < width && y >= 0 && y < height) {
        char symbol = '.';
        switch (creature->getType()) {
          case NPCType::KNIGHT: symbol = 'K'; break;
          case NPCType::ELF: symbol = 'E'; break;
          case NPCType::DRAGON: symbol = 'D'; break;
          default: symbol = '?';
        }
        map[y][x] = symbol;
      }
    }
  }
  
  std::cout << "\nКарта арены:\n";
  std::cout << std::string(width + 2, '-') << "\n";
  for (const auto& row : map) {
    std::cout << "|";
    for (char cell : row) {
      std::cout << cell;
    }
    std::cout << "|\n";
  }
  std::cout << std::string(width + 2, '-') << "\n";
  std::cout << "K - Рыцарь, E - Эльф, D - Дракон, . - пусто\n";
}

void DungeonMaster::processMovementPhase() {
  std::unique_lock lock(creatureMutex_);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dirDist(0, 3);
  
  for (auto& creature : creatures_) {
    if (creature->isAlive()) {
      MoveDirection direction = static_cast<MoveDirection>(dirDist(gen));
      creature->move(direction);
    }
  }
}

void DungeonMaster::detectPotentialCombats() {
  std::unique_lock lock(creatureMutex_);
  std::lock_guard queueLock(queueMutex_);
  
  for (size_t i = 0; i < creatures_.size(); ++i) {
    if (!creatures_[i]->isAlive()) continue;
    
    for (size_t j = i + 1; j < creatures_.size(); ++j) {
      if (!creatures_[j]->isAlive()) continue;
      
      if (creatures_[i]->canKill(*creatures_[j])) {
        combatQueue_.push({i, j});
      } else if (creatures_[j]->canKill(*creatures_[i])) {
        combatQueue_.push({j, i});
      }
    }
  }
}

void DungeonMaster::resolveCombatQueue() {
  std::unique_lock lock(creatureMutex_);
  std::lock_guard queueLock(queueMutex_);
  
  CombatMediator mediator(creatures_, watchers_);
  while (!combatQueue_.empty()) {
    auto [attackerIdx, defenderIdx] = combatQueue_.front();
    combatQueue_.pop();
    
    if (attackerIdx < creatures_.size() && defenderIdx < creatures_.size() &&
        creatures_[attackerIdx]->isAlive() && creatures_[defenderIdx]->isAlive()) {
      executeCombat(attackerIdx, defenderIdx);
    }
  }
}

void DungeonMaster::executeCombat(size_t attackerIdx, size_t defenderIdx) {
  if (attackerIdx >= creatures_.size() || defenderIdx >= creatures_.size()) {
    return;
  }
  
  CombatMediator mediator(creatures_, watchers_);
  auto outcome = mediator.engage(*creatures_[attackerIdx], *creatures_[defenderIdx]);
  
  if (outcome == ATTACKER_VICTORY) {
    creatures_[defenderIdx]->setAlive(false);
  }
}

size_t DungeonMaster::getCreatureCount() const {
  std::shared_lock lock(creatureMutex_);
  return creatures_.size();
}

bool DungeonMaster::isCreatureAlive(size_t index) const {
  std::shared_lock lock(creatureMutex_);
  if (index < creatures_.size()) {
    return creatures_[index]->isAlive();
  }
  return false;
}

std::string DungeonMaster::getCreatureInfo(size_t index) const {
  std::shared_lock lock(creatureMutex_);
  if (index < creatures_.size()) {
    std::stringstream ss;
    ss << *creatures_[index];
    return ss.str();
  }
  return "Неверный индекс";
}

DungeonMaster::GameStats DungeonMaster::getCurrentStats() const {
  std::shared_lock lock(creatureMutex_);
  GameStats stats{0, 0, 0, 0, 0};
  stats.totalCreatures = creatures_.size();
  
  for (const auto& creature : creatures_) {
    if (creature->isAlive()) {
      stats.aliveCreatures++;
      switch (creature->getType()) {
        case NPCType::KNIGHT: stats.knights++; break;
        case NPCType::ELF: stats.elves++; break;
        case NPCType::DRAGON: stats.dragons++; break;
        default: break;
      }
    }
  }
  
  return stats;
}