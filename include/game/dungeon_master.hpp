#ifndef DUNGEON_MASTER_HPP
#define DUNGEON_MASTER_HPP

#include <vector>
#include <memory>
#include <queue>
#include <random>
#include <shared_mutex>
#include "../npc/npc.hpp"
#include "./factory.hpp"
#include "./observer.hpp"
#include "./combat_visitor.hpp"

class DungeonMaster {
  private:
    std::vector<std::unique_ptr<NPC>> creatures_;
    std::vector<Observer*> watchers_;
    std::queue<std::pair<size_t, size_t>> combatQueue_;
    mutable std::shared_mutex creatureMutex_;
    std::mutex queueMutex_;
    
    // Вспомогательные методы
    bool validateCoordinates(double x, double y) const;
    void broadcastEvent(const std::string& event) const;
    
  public:
    DungeonMaster();
    ~DungeonMaster();
    
    // Инициализация
    void initializeCreatures(int count = 50);
    
    // Управление существами
    void spawnCreature(NPCType type, double x, double y, const std::string& name);
    void spawnCreature(const std::string& type, double x, double y, const std::string& name);
    void relocateCreature(size_t index, MoveDirection direction);
    
    // Сохранение/загрузка
    void loadScenario(const std::string& fileName);
    void saveScenario(const std::string& fileName) const;
    
    // Отображение
    void displayCreature(const std::string& name) const;
    void displayAllCreatures() const;
    void displayLivingCreatures() const;
    void renderMap() const;
    
    // Игровая механика
    void processMovementPhase();
    void detectPotentialCombats();
    void resolveCombatQueue();
    void executeCombat(size_t attackerIdx, size_t defenderIdx);
    
    // Геттеры для многопоточности
    size_t getCreatureCount() const;
    bool isCreatureAlive(size_t index) const;
    std::string getCreatureInfo(size_t index) const;
    std::shared_mutex* getMutex() { return &creatureMutex_; }
    
    // Статистика
    struct GameStats {
        int totalCreatures;
        int aliveCreatures;
        int knights;
        int elves;
        int dragons;
    };
    
    GameStats getCurrentStats() const;
};

#endif