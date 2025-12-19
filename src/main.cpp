#include "include/game/dungeon_master.hpp"
#include "include/game/constants.hpp"
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <queue>
#include <condition_variable>
#include <iomanip>

class GameSession {
private:
    DungeonMaster world;
    std::atomic<bool> sessionActive{true};
    std::chrono::seconds sessionDuration;
    
    // Потоки
    std::thread movementThread;
    std::thread combatThread;
    std::thread displayThread;
    
    // Синхронизация
    std::mutex consoleMutex;
    std::condition_variable combatReady;
    std::atomic<int> pendingCombats{0};
    
    void displayBanner() {
        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << "\n╔══════════════════════════════════════╗\n";
        std::cout << "║       BALAGUR FATE 3 - ARENA        ║\n";
        std::cout << "║     Многопоточная симуляция боя     ║\n";
        std::cout << "╚══════════════════════════════════════╝\n\n";
    }
    
    void displayStats() {
        auto stats = world.getCurrentStats();
        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << "┌───────────── ТЕКУЩАЯ СТАТИСТИКА ─────────────┐\n";
        std::cout << "│ Существ всего: " << std::setw(4) << stats.totalCreatures;
        std::cout << " │ Выжило: " << std::setw(4) << stats.aliveCreatures << " │\n";
        std::cout << "│ Рыцари: " << std::setw(4) << stats.knights;
        std::cout << " │ Эльфы: " << std::setw(4) << stats.elves;
        std::cout << " │ Драконы: " << std::setw(4) << stats.dragons << " │\n";
        std::cout << "└──────────────────────────────────────────────┘\n";
    }
    
public:
    GameSession(int durationSeconds) : sessionDuration(durationSeconds) {
        displayBanner();
        world.initializeCreatures(ArenaConfig::INITIAL_POPULATION);
        
        std::cout << "Инициализация арены...\n";
        std::cout << "Создано " << ArenaConfig::INITIAL_POPULATION << " существ в случайных позициях\n";
        std::cout << "Длительность сессии: " << durationSeconds << " секунд\n";
        std::cout << "──────────────────────────────────────────────\n";
    }
    
    void movementTask() {
        std::random_device rd;
        std::mt19937 gen(rd());
        
        while (sessionActive) {
            {
                // Используем shared_lock для чтения
                std::unique_lock lock(*world.getMutex());
                world.processMovementPhase();
                world.detectPotentialCombats();
                pendingCombats++;
            }
            
            combatReady.notify_one();
            std::this_thread::sleep_for(std::chrono::milliseconds(ArenaConfig::Timing::MOVEMENT_INTERVAL));
        }
    }
    
    void combatTask() {
        while (sessionActive) {
            std::unique_lock<std::mutex> lock(consoleMutex, std::defer_lock);
            
            // Ждем новые бои
            std::unique_lock<std::mutex> cvLock(consoleMutex);
            combatReady.wait_for(cvLock, std::chrono::milliseconds(100),
                [this]() { return pendingCombats > 0 || !sessionActive; });
            
            if (pendingCombats > 0) {
                pendingCombats--;
                cvLock.unlock();
                
                {
                    // Используем unique_lock для записи
                    std::unique_lock lock(*world.getMutex());
                    world.resolveCombatQueue();
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(ArenaConfig::Timing::COMBAT_INTERVAL));
        }
    }
    
    void displayTask() {
        int tick = 0;
        
        while (sessionActive) {
            {
                std::lock_guard<std::mutex> lock(consoleMutex);
                std::cout << "\n=== ТИК " << ++tick << " ===" << std::endl;
                
                // Используем shared_lock для чтения
                std::shared_lock lock(*world.getMutex());
                world.renderMap();
                displayStats();
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    void run() {
        auto startTime = std::chrono::steady_clock::now();
        
        // Запускаем потоки
        movementThread = std::thread(&GameSession::movementTask, this);
        combatThread = std::thread(&GameSession::combatTask, this);
        displayThread = std::thread(&GameSession::displayTask, this);
        
        // Ожидаем завершения времени сессии
        std::this_thread::sleep_for(sessionDuration);
        sessionActive = false;
        
        // Уведомляем все потоки
        combatReady.notify_all();
        
        // Ждем завершения потоков
        if (movementThread.joinable()) movementThread.join();
        if (combatThread.joinable()) combatThread.join();
        if (displayThread.joinable()) displayThread.join();
        
        // Вывод финальных результатов
        displayFinalResults();
    }
    
    void displayFinalResults() {
        auto stats = world.getCurrentStats();
        
        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << "\n\n╔══════════════════════════════════════╗\n";
        std::cout << "║         СЕССИЯ ЗАВЕРШЕНА           ║\n";
        std::cout << "╠══════════════════════════════════════╣\n";
        std::cout << "║        ФИНАЛЬНАЯ СТАТИСТИКА         ║\n";
        std::cout << "╠══════════════════════════════════════╣\n";
        std::cout << "║ Начало: " << ArenaConfig::INITIAL_POPULATION << " существ          ║\n";
        std::cout << "║ Выжило: " << std::setw(4) << stats.aliveCreatures << " существ          ║\n";
        std::cout << "║ Уничтожено: " << std::setw(3) << (ArenaConfig::INITIAL_POPULATION - stats.aliveCreatures) << " существ        ║\n";
        std::cout << "╠══════════════════════════════════════╣\n";
        std::cout << "║         ВЫЖИВШИЕ СУЩЕСТВА           ║\n";
        std::cout << "╚══════════════════════════════════════╝\n\n";
        
        world.displayLivingCreatures();
        
        // Сохранение финального состояния
        try {
            world.saveScenario("final_state.txt");
            std::cout << "\nФинальное состояние сохранено в файл 'final_state.txt'\n";
        } catch (const std::exception& e) {
            std::cout << "\nНе удалось сохранить финальное состояние: " << e.what() << "\n";
        }
    }
};

int main() {
    try {
        const int DEFAULT_SESSION_TIME = 30;
        
        std::cout << "Введите длительность сессии (секунд, по умолчанию " 
                  << DEFAULT_SESSION_TIME << "): ";
        
        int sessionTime = DEFAULT_SESSION_TIME;
        std::string input;
        std::getline(std::cin, input);
        
        if (!input.empty()) {
            try {
                sessionTime = std::stoi(input);
                if (sessionTime <= 0) sessionTime = DEFAULT_SESSION_TIME;
            } catch (...) {
                sessionTime = DEFAULT_SESSION_TIME;
            }
        }
        
        GameSession arena(sessionTime);
        arena.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}