#include "../../include/game/observer.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>

std::string ConsoleDisplay::formatCoordinates(double x, double y) const {
  return "(" + std::to_string(static_cast<int>(x)) + ", " + 
                std::to_string(static_cast<int>(y)) + ")";
}

std::string ConsoleDisplay::formatCreatureType(NPCType type) const {
  switch (type) {
    case NPCType::KNIGHT: return "⚔ Рыцарь";
    case NPCType::ELF: return "🏹 Эльф";
    case NPCType::DRAGON: return "🐉 Дракон";
    default: return "❓ Неизвестный";
  }
}

void ConsoleDisplay::recordBattle(const NPC& victor, const NPC& defeated) {
  std::lock_guard lock(displayMutex_);
  std::cout << "[БОЙ] " << formatCreatureType(victor.getType()) 
            << " " << victor.getName()
            << " победил " << formatCreatureType(defeated.getType())
            << " " << defeated.getName() << "\n";
}

void ConsoleDisplay::recordMovement(const NPC& creature, MoveDirection direction) {
  std::lock_guard lock(displayMutex_);
  std::cout << "[ДВИЖ] " << creature.getName() 
            << " переместился " << convertDirectionToString(direction)
            << " в " << formatCoordinates(creature.getX(), creature.getY()) << "\n";
}

void ConsoleDisplay::recordGameEvent(const std::string& event) {
  std::lock_guard lock(displayMutex_);
  std::cout << "[СОБЫТИЕ] " << event << "\n";
}

void ConsoleDisplay::displayWorldState(const std::vector<const NPC*>& creatures) {
  std::lock_guard lock(displayMutex_);
  std::cout << "\n=== ТЕКУЩЕЕ СОСТОЯНИЕ МИРА ===\n";
  for (const auto& creature : creatures) {
    if (creature->isAlive()) {
      std::cout << formatCreatureType(creature->getType()) << " "
                << creature->getName() << " "
                << formatCoordinates(creature->getX(), creature->getY()) << "\n";
    }
  }
  std::cout << "===============================\n";
}

FileRecorder::FileRecorder() {
  battleLog_.open(ArenaConfig::Files::COMBAT_LOG_FILE, std::ios::app);
  movementLog_.open(ArenaConfig::Files::MOVEMENT_LOG_FILE, std::ios::app);
  eventLog_.open(ArenaConfig::Files::EVENT_LOG_FILE, std::ios::app);
}

void FileRecorder::writeToLog(std::ofstream& stream, const std::string& message) {
  if (stream.is_open()) {
    std::time_t now = std::time(nullptr);
    std::tm* timeinfo = std::localtime(&now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "[%Y-%m-%d %H:%M:%S] ", timeinfo);
    
    stream << buffer << message << std::endl;
  }
}

void FileRecorder::recordBattle(const NPC& victor, const NPC& defeated) {
  std::lock_guard lock(fileMutex_);
  std::string message = victor.getName() + " (" + victor.getTypeString() + 
                       ") победил " + defeated.getName() + " (" + 
                       defeated.getTypeString() + ")";
  writeToLog(battleLog_, message);
}

void FileRecorder::recordMovement(const NPC& creature, MoveDirection direction) {
  std::lock_guard lock(fileMutex_);
  std::string message = creature.getName() + " переместился " + 
                       convertDirectionToString(direction) + " в (" + 
                       std::to_string(creature.getX()) + ", " + 
                       std::to_string(creature.getY()) + ")";
  writeToLog(movementLog_, message);
}

void FileRecorder::recordGameEvent(const std::string& event) {
  std::lock_guard lock(fileMutex_);
  writeToLog(eventLog_, event);
}

void FileRecorder::displayWorldState(const std::vector<const NPC*>& creatures) {
  // Для файлового логгера не реализуем отображение состояния
}

FileRecorder::~FileRecorder() {
  if (battleLog_.is_open()) battleLog_.close();
  if (movementLog_.is_open()) movementLog_.close();
  if (eventLog_.is_open()) eventLog_.close();
}