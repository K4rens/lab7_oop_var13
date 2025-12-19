#ifndef NPC_HPP
#define NPC_HPP

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <fstream>
#include <shared_mutex>
#include <mutex>

enum NPCType {
  UNKNOWN = 0,
  KNIGHT = 1,
  ELF = 2,
  DRAGON = 3
};

enum MoveDirection {
  TOP,
  RIGHT,
  BOTTOM,
  LEFT
};

class NPC {
  protected:
    NPCType type_;
    double x_ = 0;
    double y_ = 0;
    std::string name_ = "";
    bool alive_ = true;
    double moveDistance_ = 1.0;
    double attackRange_ = 1.0;
    
    // Синхронизация для многопоточности
    mutable std::shared_mutex positionMutex_;
    mutable std::mutex stateMutex_;
    
  public:
    NPC();
    NPC(NPCType type);
    NPC(NPCType type, double x, double y, const std::string &name, 
        double moveDistance, double attackRange);

    NPCType getType() const;
    std::string getTypeString() const;
    double getX() const;
    double getY() const;
    std::pair<double, double> getPosition() const;
    std::string getName() const;
    bool isAlive() const;
    double getMoveDistance() const;
    double getAttackRange() const;

    void setAlive(bool alive);
    void move(MoveDirection direction);
    void updatePosition(double newX, double newY);
    bool isValidPosition(double x, double y) const;
    
    bool canKill(const NPC &other) const;
    bool isWithinRange(const NPC &other) const;
    bool hasAdvantageOver(const NPC &other) const;
    double distance(const NPC &other) const;

    // Сериализация
    std::string serialize() const;
    static std::unique_ptr<NPC> deserialize(const std::string &data);
    
    void load(std::ifstream &in);
    void save(std::ofstream &out) const;
    void display() const;

    friend std::istream &operator>>(std::istream &in, NPC &npc);
    friend std::ostream &operator<<(std::ostream &out, const NPC &npc);
};

NPCType convertTypeFromString(const std::string &type);
MoveDirection convertDirectionFromString(const std::string &direction);
std::string convertDirectionToString(MoveDirection direction);
std::string generateRandomName(NPCType type);

#endif