#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>

namespace ArenaConfig {
    // Размеры мира
    constexpr double WORLD_MIN_X = 0.0;
    constexpr double WORLD_MAX_X = 1000.0;
    constexpr double WORLD_MIN_Y = 0.0;
    constexpr double WORLD_MAX_Y = 1000.0;
    
    // Количество существ
    constexpr int INITIAL_POPULATION = 50;
    
    // Параметры движения
    namespace Mobility {
        constexpr double KNIGHT_STEP = 30.0;
        constexpr double ELF_STEP = 10.0;
        constexpr double DRAGON_STEP = 50.0;
    }
    
    // Боевые параметры
    namespace Combat {
        constexpr double KNIGHT_SWORD_REACH = 10.0;
        constexpr double ELF_BOW_RANGE = 50.0;
        constexpr double DRAGON_BREATH_RANGE = 30.0;
        constexpr int ATTACK_DICE_SIDES = 6;
        constexpr int DEFENSE_DICE_SIDES = 6;
    }
    
    // Тайминги (в миллисекундах)
    namespace Timing {
        constexpr int MOVEMENT_INTERVAL = 300;
        constexpr int COMBAT_INTERVAL = 50;
        constexpr int DISPLAY_INTERVAL = 1000;
        constexpr int DEFAULT_SESSION_DURATION = 30000; // 30 секунд
    }
    
    // Файлы
    namespace Files {
        const std::string DEFAULT_SAVE_FILE = "arena_state.txt";
        const std::string COMBAT_LOG_FILE = "combat_log.txt";
        const std::string MOVEMENT_LOG_FILE = "movement_log.txt";
        const std::string EVENT_LOG_FILE = "game_events.txt";
    }
}

#endif