#ifndef FACTORY_HPP
#define FACTORY_HPP

#include "../npc/npc.hpp"
#include <memory>
#include <string>
#include <fstream>
#include <random>
#include <vector>

class CreatureFactory {
  public:
    // Основные методы создания
    static std::unique_ptr<NPC> createCreature(NPCType type);
    static std::unique_ptr<NPC> createCreature(NPCType type, double x, double y, 
                                               const std::string& name);
    
    // Альтернативные методы создания
    static std::unique_ptr<NPC> createRandomCreature();
    static std::unique_ptr<NPC> createCreatureAtEdge(NPCType type, 
                                                     const std::string& name = "");
    static std::unique_ptr<NPC> createCreatureAtCenter(NPCType type,
                                                        const std::string& name = "");
    
    // Загрузка из файла
    static std::unique_ptr<NPC> loadCreatureFromFile(std::ifstream& file);
    static std::unique_ptr<NPC> parseCreatureData(const std::string& data);
    
    // Массовое создание
    static std::vector<std::unique_ptr<NPC>> createCreatureSwarm(NPCType type, 
                                                                 int count);
    static std::vector<std::unique_ptr<NPC>> createRandomSwarm(int count);
    
    // Утилиты
    static std::string generateCreatureName(NPCType type);
    static bool validatePosition(double x, double y);
    
  private:
    static std::mt19937& getRandomEngine();
    static std::string getRandomKnightName();
    static std::string getRandomElfName();
    static std::string getRandomDragonName();
};

// Алиас для обратной совместимости
using Factory = CreatureFactory;

#endif