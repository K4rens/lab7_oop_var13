#ifndef ELF_HPP
#define ELF_HPP

#include "./npc.hpp"

class Elf: public NPC {
  public:
    Elf();
    Elf(double x, double y, const std::string &name);
    Elf(double x, double y);
    
    // Специфичные для эльфа методы
    std::string getClan() const;
    bool canUseMagic() const;
};

#endif