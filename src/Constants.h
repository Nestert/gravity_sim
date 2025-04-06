#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

// --- Графика и Окно ---
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const float RADIUS_DISPLAY_SCALE = 500.0f; // Дополнительный масштаб для видимости радиусов
const int GRID_DIVISIONS = 50;   // Увеличено разрешение сетки
const float GRID_SIZE = 20000.0f;

// --- Физика ---
const double GRAVITATIONAL_CONSTANT = 6.67430e-11; // м^3 кг^-1 с^-2
const double SPEED_OF_LIGHT = 299792458.0; // м/с

// --- Симуляция ---
const float SIMULATION_SCALE = 1.0e6; // 1 юнит симуляции = 1,000,000 метров (1000 км)
const float MIN_GRAVITY_DISTANCE_METERS = 100.0f;   // Минимальная дистанция для расчета гравитации в метрах
const float MIN_GRAVITY_DISTANCE_SIM = MIN_GRAVITY_DISTANCE_METERS * SIMULATION_SCALE;
// Искажение сетки
const float DEFORMATION_STRENGTH = 1.0e-18f; // Сильно уменьшено для теста

// --- Управление ---
const float CAMERA_SPEED_BASE = 2500.0f; // Базовая скорость камеры (юнитов/с)
const float CAMERA_SPEED_MULTIPLIER = 5.0f; // Множитель скорости при зажатом Shift
const float CAMERA_SENSITIVITY = 0.1f; // Чувствительность мыши
const float INITIAL_OBJECT_MOVE_SPEED = 1500.0f; // Скорость перемещения объекта при инициализации (юнитов/с)
const float INITIAL_OBJECT_MASS_SCALE_FACTOR = 1.01f; // Множитель массы при удержании ПКМ (в секунду)

#endif // CONSTANTS_H 