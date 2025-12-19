#ifndef DRAGON_HPP
#define DRAGON_HPP

#include "./npc.hpp"

class Dragon: public NPC {
  public:
    Dragon();
    Dragon(double x, double y, const std::string &name);
    Dragon(double x, double y);
    
    // Специфичные для дракона методы
    std::string getColor() const;
    bool canFly() const;
    double getFireRange() const;
};

#endif