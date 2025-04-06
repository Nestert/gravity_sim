# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -fdiagnostics-color=always -g -Wall -Wextra
# Добавляем пути src и vendor/imgui в пути для include
INCFLAGS = -I/opt/homebrew/include -Isrc -Ivendor/imgui -Ivendor/imgui/backends
LDFLAGS = -L/opt/homebrew/lib
LIBS = -lglfw -lGLEW
FRAMEWORKS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

# Target executable
TARGET = gravity_sim

# Source directories
SRC_DIR = src
VENDOR_DIR = vendor/imgui
BACKENDS_DIR = $(VENDOR_DIR)/backends

# Source files (с указанием директорий)
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/Graphics.cpp $(SRC_DIR)/Input.cpp $(SRC_DIR)/Simulation.cpp $(SRC_DIR)/Object.cpp \
       $(VENDOR_DIR)/imgui.cpp \
       $(VENDOR_DIR)/imgui_draw.cpp \
       $(VENDOR_DIR)/imgui_tables.cpp \
       $(VENDOR_DIR)/imgui_widgets.cpp \
       $(BACKENDS_DIR)/imgui_impl_glfw.cpp \
       $(BACKENDS_DIR)/imgui_impl_opengl3.cpp

# Object files directory
OBJ_DIR = obj
# Object files (Явно перечисляем все объектные файлы)
OBJS = $(OBJ_DIR)/main.o \
       $(OBJ_DIR)/Graphics.o \
       $(OBJ_DIR)/Input.o \
       $(OBJ_DIR)/Simulation.o \
       $(OBJ_DIR)/Object.o \
       $(OBJ_DIR)/imgui.o \
       $(OBJ_DIR)/imgui_draw.o \
       $(OBJ_DIR)/imgui_tables.o \
       $(OBJ_DIR)/imgui_widgets.o \
       $(OBJ_DIR)/backends/imgui_impl_glfw.o \
       $(OBJ_DIR)/backends/imgui_impl_opengl3.o

# Default rule: build the target executable
all: $(TARGET)

# Rule to link the executable
$(TARGET): $(OBJS)
	@echo "Linking target: $@"
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS) $(LIBS) $(FRAMEWORKS)

# Rule to compile source files from src directory
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling $< -> $@"
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -c $< -o $@

# Rule to compile source files from vendor/imgui directory
# (Кроме бэкендов)
$(OBJ_DIR)/%.o: $(VENDOR_DIR)/%.cpp
	@echo "Compiling $< -> $@"
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -c $< -o $@

# Rule to compile source files from vendor/imgui/backends directory
$(OBJ_DIR)/backends/%.o: $(BACKENDS_DIR)/%.cpp # ИЗМЕНЕН ПУТЬ К ЦЕЛИ
	@echo "Compiling $< -> $@"
	@mkdir -p $(dir $@) # Теперь $(dir $@) будет obj/backends/
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -c $< -o $@


# Specify dependencies (пути к .o файлам бэкендов изменены)
$(OBJ_DIR)/main.o: $(SRC_DIR)/main.cpp $(SRC_DIR)/Constants.h $(SRC_DIR)/Graphics.h $(SRC_DIR)/Input.h $(SRC_DIR)/Simulation.h $(SRC_DIR)/Object.h vendor/imgui/imgui.h vendor/imgui/backends/imgui_impl_glfw.h vendor/imgui/backends/imgui_impl_opengl3.h
$(OBJ_DIR)/Graphics.o: $(SRC_DIR)/Graphics.cpp $(SRC_DIR)/Graphics.h $(SRC_DIR)/Constants.h
$(OBJ_DIR)/Input.o: $(SRC_DIR)/Input.cpp $(SRC_DIR)/Input.h $(SRC_DIR)/Constants.h $(SRC_DIR)/Object.h
$(OBJ_DIR)/Simulation.o: $(SRC_DIR)/Simulation.cpp $(SRC_DIR)/Simulation.h $(SRC_DIR)/Constants.h $(SRC_DIR)/Object.h
$(OBJ_DIR)/Object.o: $(SRC_DIR)/Object.cpp $(SRC_DIR)/Object.h $(SRC_DIR)/Constants.h $(SRC_DIR)/Graphics.h

# Зависимости для ImGui (пути к .o файлам бэкендов изменены)
$(OBJ_DIR)/imgui.o: $(VENDOR_DIR)/imgui.cpp vendor/imgui/imgui.h vendor/imgui/imconfig.h
$(OBJ_DIR)/imgui_draw.o: $(VENDOR_DIR)/imgui_draw.cpp vendor/imgui/imgui.h
$(OBJ_DIR)/imgui_tables.o: $(VENDOR_DIR)/imgui_tables.cpp vendor/imgui/imgui.h
$(OBJ_DIR)/imgui_widgets.o: $(VENDOR_DIR)/imgui_widgets.cpp vendor/imgui/imgui.h
$(OBJ_DIR)/backends/imgui_impl_glfw.o: $(BACKENDS_DIR)/imgui_impl_glfw.cpp vendor/imgui/backends/imgui_impl_glfw.h vendor/imgui/imgui.h # ИЗМЕНЕН ПУТЬ
$(OBJ_DIR)/backends/imgui_impl_opengl3.o: $(BACKENDS_DIR)/imgui_impl_opengl3.cpp vendor/imgui/backends/imgui_impl_opengl3.h vendor/imgui/imgui.h # ИЗМЕНЕН ПУТЬ


# Rule to clean generated files
clean:
	@echo "Cleaning project..."
	rm -f $(TARGET)
	rm -rf $(OBJ_DIR) # Удаляем всю директорию obj

.PHONY: all clean 