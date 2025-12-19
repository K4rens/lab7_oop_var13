#ifndef OBSERVER_HPP
#define OBSERVER_HPP

#include <fstream>
#include <string>
#include <mutex>
#include <memory>
#include <vector>
#include "../npc/npc.hpp"

class GameEventLogger {
  public:
    virtual ~GameEventLogger() = default;
    
    virtual void recordBattle(const NPC& victor, const NPC& defeated) = 0;
    virtual void recordMovement(const NPC& creature, MoveDirection direction) = 0;
    virtual void recordGameEvent(const std::string& event) = 0;
    virtual void displayWorldState(const std::vector<const NPC*>& creatures) = 0;
};

class ConsoleDisplay : public GameEventLogger {
  private:
    mutable std::mutex displayMutex_;
    
    std::string formatCoordinates(double x, double y) const;
    std::string formatCreatureType(NPCType type) const;
    
  public:
    void recordBattle(const NPC& victor, const NPC& defeated) override;
    void recordMovement(const NPC& creature, MoveDirection direction) override;
    void recordGameEvent(const std::string& event) override;
    void displayWorldState(const std::vector<const NPC*>& creatures) override;
};

class FileRecorder : public GameEventLogger {
  private:
    std::ofstream battleLog_;
    std::ofstream movementLog_;
    std::ofstream eventLog_;
    mutable std::mutex fileMutex_;
    
    void writeToLog(std::ofstream& stream, const std::string& message);
    
  public:
    FileRecorder();
    
    void recordBattle(const NPC& victor, const NPC& defeated) override;
    void recordMovement(const NPC& creature, MoveDirection direction) override;
    void recordGameEvent(const std::string& event) override;
    void displayWorldState(const std::vector<const NPC*>& creatures) override;
    
    ~FileRecorder();
};

// Алиас для обратной совместимости
using Observer = GameEventLogger;
using ScreenObserver = ConsoleDisplay;
using FileObserver = FileRecorder;

#endif