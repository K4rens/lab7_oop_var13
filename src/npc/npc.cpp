#include "../../include/npc/npc.hpp"
#include "../../include/game/constants.hpp"
#include <string>
#include <cmath>
#include <array>
#include <algorithm>
#include <random>
#include <sstream>

NPC::NPC(): type_(NPCType::UNKNOWN) {}

NPC::NPC(NPCType type): type_(type) {}

NPC::NPC(NPCType type, double x, double y, const std::string &name, 
         double moveDistance, double attackRange): 
  type_(type), x_(x), y_(y), name_(name), 
  moveDistance_(moveDistance), attackRange_(attackRange) {}

NPCType NPC::getType() const {
  return type_;
}

std::string NPC::getTypeString() const {
  switch (type_) {
    case NPCType::KNIGHT: return "Странствующий рыцарь";
    case NPCType::ELF: return "Эльф";
    case NPCType::DRAGON: return "Дракон";
    default: return "Неизвестный";
  }
}

double NPC::getX() const {
  std::shared_lock lock(positionMutex_);
  return x_;
}

double NPC::getY() const {
  std::shared_lock lock(positionMutex_);
  return y_;
}

std::pair<double, double> NPC::getPosition() const {
  std::shared_lock lock(positionMutex_);
  return {x_, y_};
}

std::string NPC::getName() const {
  return name_;
}

bool NPC::isAlive() const {
  std::lock_guard lock(stateMutex_);
  return alive_;
}

double NPC::getMoveDistance() const {
  return moveDistance_;
}

double NPC::getAttackRange() const {
  return attackRange_;
}

void NPC::setAlive(bool alive) {
  std::lock_guard lock(stateMutex_);
  alive_ = alive;
}

void NPC::move(MoveDirection direction) {
  if (!isAlive()) {
    return;
  }

  std::unique_lock lock(positionMutex_);
  switch (direction) {
    case MoveDirection::TOP: 
      y_ = std::clamp(y_ + moveDistance_, ArenaConfig::WORLD_MIN_Y, ArenaConfig::WORLD_MAX_Y); 
      break;
    case MoveDirection::RIGHT: 
      x_ = std::clamp(x_ + moveDistance_, ArenaConfig::WORLD_MIN_X, ArenaConfig::WORLD_MAX_X); 
      break;
    case MoveDirection::BOTTOM: 
      y_ = std::clamp(y_ - moveDistance_, ArenaConfig::WORLD_MIN_Y, ArenaConfig::WORLD_MAX_Y); 
      break;
    case MoveDirection::LEFT: 
      x_ = std::clamp(x_ - moveDistance_, ArenaConfig::WORLD_MIN_X, ArenaConfig::WORLD_MAX_X); 
      break;
  }
}

void NPC::updatePosition(double newX, double newY) {
  std::unique_lock lock(positionMutex_);
  x_ = newX;
  y_ = newY;
}

bool NPC::isValidPosition(double x, double y) const {
  return (x >= ArenaConfig::WORLD_MIN_X && x <= ArenaConfig::WORLD_MAX_X &&
          y >= ArenaConfig::WORLD_MIN_Y && y <= ArenaConfig::WORLD_MAX_Y);
}

bool NPC::canKill(const NPC &other) const {
  if (!isAlive() || !other.isAlive()) {
    return false;
  }

  if (distance(other) <= attackRange_) {
    switch (type_) {
      case NPCType::KNIGHT: return (other.type_ == NPCType::DRAGON);
      case NPCType::ELF: return (other.type_ == NPCType::KNIGHT);
      case NPCType::DRAGON: return true;
      default: return false;
    }
  }
  return false;
}

bool NPC::isWithinRange(const NPC &other) const {
  return distance(other) <= attackRange_;
}

bool NPC::hasAdvantageOver(const NPC &other) const {
  switch (type_) {
    case NPCType::KNIGHT: return other.type_ == NPCType::DRAGON;
    case NPCType::ELF: return other.type_ == NPCType::KNIGHT;
    case NPCType::DRAGON: return true;
    default: return false;
  }
}

void NPC::load(std::ifstream &in) {
  std::string type;
  in >> type >> name_ >> x_ >> y_;
  type_ = convertTypeFromString(type);
}

void NPC::save(std::ofstream &out) const {
  if (!isAlive()) {
    return;
  }

  out << getTypeString() << " "
      << name_ << " "
      << x_ << " "
      << y_ << " "
      << (isAlive() ? "жив" : "мертв") << "\n";
}

void NPC::display() const {
  std::cout << getTypeString() << " " 
            << name_ << " ["
            << x_ << ", "
            << y_ << "] "
            << (isAlive() ? "жив" : "мертв") << "\n";
}

std::string NPC::serialize() const {
  std::stringstream ss;
  ss << static_cast<int>(type_) << " "
     << name_ << " "
     << x_ << " "
     << y_ << " "
     << alive_ << " "
     << moveDistance_ << " "
     << attackRange_;
  return ss.str();
}

std::unique_ptr<NPC> NPC::deserialize(const std::string &data) {
  std::stringstream ss(data);
  int typeInt;
  std::string name;
  double x, y;
  bool alive;
  double moveDist, attackRange;
  
  ss >> typeInt >> name >> x >> y >> alive >> moveDist >> attackRange;
  
  auto npc = std::make_unique<NPC>(static_cast<NPCType>(typeInt), x, y, name, moveDist, attackRange);
  npc->setAlive(alive);
  return npc;
}

double NPC::distance(const NPC &other) const {
  std::shared_lock lock1(positionMutex_, std::defer_lock);
  std::shared_lock lock2(other.positionMutex_, std::defer_lock);
  std::lock(lock1, lock2);
  
  const double distanceX = x_ - other.x_;
  const double distanceY = y_ - other.y_;
  return std::sqrt(distanceX * distanceX + distanceY * distanceY);
}

std::istream &operator>>(std::istream &in, NPC &npc) {
  std::string type;
  in >> type >> npc.x_ >> npc.y_ >> npc.name_;
  npc.type_ = convertTypeFromString(type);
  return in;
}

std::ostream &operator<<(std::ostream &out, const NPC &npc) {
  out << "NPC: "
      << "type=\"" << npc.getTypeString() << "\", "
      << "name=" << npc.name_ << ", "
      << "x=" << npc.getX() << ", "
      << "y=" << npc.getY() << ", "
      << "alive=" << (npc.isAlive() ? "да" : "нет") << std::endl;
  return out;
}

NPCType convertTypeFromString(const std::string &type) {
  if (type == "Странствующий рыцарь" || type == "knight" || type == "KNIGHT") 
    return NPCType::KNIGHT;
  if (type == "Эльф" || type == "elf" || type == "ELF") 
    return NPCType::ELF;
  if (type == "Дракон" || type == "dragon" || type == "DRAGON") 
    return NPCType::DRAGON;
  return NPCType::UNKNOWN;
}

MoveDirection convertDirectionFromString(const std::string &direction) {
  if (direction == "вверх" || direction == "up" || direction == "TOP") 
    return MoveDirection::TOP;
  if (direction == "вправо" || direction == "right" || direction == "RIGHT") 
    return MoveDirection::RIGHT;
  if (direction == "вниз" || direction == "down" || direction == "BOTTOM") 
    return MoveDirection::BOTTOM;
  if (direction == "влево" || direction == "left" || direction == "LEFT") 
    return MoveDirection::LEFT;
  throw std::invalid_argument("Некорректное направление движения");
}

std::string convertDirectionToString(MoveDirection direction) {
  switch (direction) {
    case MoveDirection::TOP: return "вверх";
    case MoveDirection::RIGHT: return "вправо";
    case MoveDirection::BOTTOM: return "вниз";
    case MoveDirection::LEFT: return "влево";
    default: throw std::invalid_argument("Некорректное направление движения");
  }
}

std::string generateRandomName(NPCType type) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<int> dist(1000, 9999);
  
  std::string base;
  switch (type) {
    case NPCType::KNIGHT: base = "Рыцарь_"; break;
    case NPCType::ELF: base = "Эльф_"; break;
    case NPCType::DRAGON: base = "Дракон_"; break;
    default: base = "Существо_";
  }
  
  return base + std::to_string(dist(gen));
}