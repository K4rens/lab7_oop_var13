# Makefile
CXX = g++-11
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -pthread
TARGET = balagur_arena
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = build
BIN_DIR = bin

# Автоматическое обнаружение исходных файлов
SRCS = $(shell find $(SRC_DIR) -name "*.cpp")
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Правила по умолчанию
.PHONY: all clean run debug release setup

all: setup release

# Релизная сборка
release: CXXFLAGS += -O3 -DNDEBUG
release: $(BIN_DIR)/$(TARGET)

# Отладочная сборка
debug: CXXFLAGS += -g -O0 -DDEBUG
debug: $(BIN_DIR)/$(TARGET)_debug

# Создание директорий
setup:
	@mkdir -p $(OBJ_DIR) $(OBJ_DIR)/game $(OBJ_DIR)/npc $(BIN_DIR)

# Сборка релизной версии
$(BIN_DIR)/$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "✅ Сборка завершена: $@"

# Сборка отладочной версии
$(BIN_DIR)/$(TARGET)_debug: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "✅ Отладочная сборка завершена: $@"

# Компиляция объектных файлов
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Очистка
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) *.log *.txt final_state.txt
	@echo "🧹 Очистка завершена"

# Запуск
run: release
	@./$(BIN_DIR)/$(TARGET)

# Запуск отладки
debug_run: debug
	@./$(BIN_DIR)/$(TARGET)_debug

# Создание архива
archive:
	@tar -czf balagur_arena_src_$(shell date +%Y%m%d_%H%M%S).tar.gz src/ include/ Makefile README.md
	@echo "📦 Архив создан"

# Проверка синтаксиса
check:
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) --syntax-only $(SRCS)
	@echo "✓ Синтаксис проверен"

# Подсчет строк кода
count:
	@echo "📊 Статистика проекта:"
	@find src include -name "*.cpp" -o -name "*.hpp" -o -name "*.h" | xargs wc -l