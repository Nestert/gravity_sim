// Simulation.cpp
#include "Simulation.h"
#include "Input.h" // Включаем для доступа к настраиваемым параметрам
#include <cmath>     // Для std::sqrt
#include <iostream> // Для отладки
#include <numeric> // для std::accumulate
#include <vector> // Добавлено для std::vector<glm::vec3>

namespace {
// ... sphericalToCartesian ...
} // анонимное пространство имен

// Создание вершин плоской сетки (для GL_LINES)
std::vector<float> CreateGridVertices(float size, int divisions) {
    std::vector<float> vertices;
    float step = size / divisions;
    float halfSize = size / 2.0f;

    for (int i = 0; i <= divisions; ++i) {
        float coord = -halfSize + i * step;
        // Линии вдоль X (добавляем 4-й компонент = 0.0)
        vertices.push_back(-halfSize); vertices.push_back(0.0f); vertices.push_back(coord); vertices.push_back(0.0f);
        vertices.push_back(halfSize);  vertices.push_back(0.0f); vertices.push_back(coord); vertices.push_back(0.0f);
        // Линии вдоль Z (добавляем 4-й компонент = 0.0)
        vertices.push_back(coord); vertices.push_back(0.0f); vertices.push_back(-halfSize); vertices.push_back(0.0f);
        vertices.push_back(coord); vertices.push_back(0.0f); vertices.push_back(halfSize); vertices.push_back(0.0f);
    }
    return vertices;
}

// Вычисление среднего вертикального сдвига сетки (скопировано из старого кода)
float CalculateVerticalShift(const std::vector<Object>& objs) {
     float totalMass = 0.0f;
     glm::vec3 weightedPosSum(0.0f);
     for(const auto& obj : objs) {
         if (obj.initializing) continue;
         totalMass += obj.mass;
         weightedPosSum += obj.position * obj.mass;
     }
     if (totalMass <= 0) return 0.0f;
     glm::vec3 centerOfMass = weightedPosSum / totalMass;
     return centerOfMass.y;
}

// Обновление вершин сетки (для GL_LINES)
std::vector<float> UpdateGridVertices(const std::vector<float>& initialVertices, const std::vector<Object>& objs, float verticalShift, float deformationStrength) {
    std::vector<float> updatedVertices = initialVertices;

    // Итерируем по 4 float на вершину
    for (size_t i = 0; i < updatedVertices.size(); i += 4) {
        glm::vec3 vertexPos(updatedVertices[i], 0.0f, updatedVertices[i+2]);
        float totalVerticalDisplacement = 0.0f;

        for (const auto& obj : objs) {
            if (obj.mass <= 0) continue;
            glm::vec3 toObjectPlanar = glm::vec3(obj.position.x, 0.0f, obj.position.z) - vertexPos;
            float distancePlanar_sim = glm::length(toObjectPlanar);
            const float epsilon = 10.0f;
            float displacement = 0.0f;
            if (distancePlanar_sim + epsilon > 1e-6) {
                displacement = (deformationStrength * obj.mass) / (distancePlanar_sim + epsilon);
            }
            totalVerticalDisplacement -= displacement;
        }
        // Устанавливаем Y = totalVerticalDisplacement (без вычитания verticalShift)
        updatedVertices[i+1] = totalVerticalDisplacement;
        updatedVertices[i+3] = totalVerticalDisplacement; // Сохраняем искажение для цвета
    }
    return updatedVertices;
}

// Обновление физики объектов с использованием метода Верле (Velocity Verlet)
void UpdatePhysics(std::vector<Object>& objs, float dt) {
    if (dt <= 0) return;

    float dt_half = 0.5f * dt;
    float dt_sq_half = 0.5f * dt * dt;

    // 1. Рассчитать новые позиции (используя ускорения с предыдущего шага)
    for (auto& obj : objs) {
        if (obj.initializing) continue;
        obj.position += obj.velocity * dt + obj.acceleration * dt_sq_half;
    }

    // 2. Рассчитать новые ускорения a(t+dt) на основе новых позиций
    std::vector<glm::vec3> new_accelerations(objs.size(), glm::vec3(0.0f));
    // Получаем значения из Input ОДИН раз перед циклами
    const float currentSimScale = Input::simulationScale;
    const float currentMinGravityDistMeters = Input::minGravityDistanceMeters;
    const float currentMassMultiplier = Input::massMultiplier;
    // Рассчитываем минимальное расстояние в юнитах симуляции ПРАВИЛЬНО
    const float minGravityDistanceSim = currentMinGravityDistMeters / currentSimScale;

    for (size_t i = 0; i < objs.size(); ++i) {
        new_accelerations[i] = glm::vec3(0.0f);
        if (objs[i].initializing || objs[i].mass <= 0) continue;

        for (size_t j = 0; j < objs.size(); ++j) {
            if (i == j || objs[j].initializing) continue;

            glm::vec3 direction = objs[j].position - objs[i].position;
            float distance_sim = glm::length(direction);
            // Используем новое, правильно рассчитанное minGravityDistanceSim
            float distance_sim_safe = glm::max(distance_sim, minGravityDistanceSim);

            // Переводим безопасное расстояние симуляции в метры
            float distance_meters = distance_sim_safe * currentSimScale;
            if (distance_meters > 0 && currentMassMultiplier > 0) {
                 // Учитываем massMultiplier при расчете силы
                 double force_magnitude = (GRAVITATIONAL_CONSTANT * (objs[i].mass * currentMassMultiplier) * (objs[j].mass * currentMassMultiplier)) / (distance_meters * distance_meters);
                 glm::vec3 force_vector = glm::normalize(direction) * static_cast<float>(force_magnitude);
                 // Делим на массу БЕЗ множителя и на масштаб
                 new_accelerations[i] += (force_vector / objs[i].mass) / currentSimScale;
            }
        }
    }

    // 3. Рассчитать новые скорости v(t+dt) и обновить ускорения для следующего шага
    for (size_t i = 0; i < objs.size(); ++i) {
         if (objs[i].initializing) continue;
         objs[i].velocity += (objs[i].acceleration + new_accelerations[i]) * dt_half;
         objs[i].acceleration = new_accelerations[i];
    }

    // 4. Обновить историю траекторий
    for (auto& obj : objs) {
        if (obj.initializing) continue;
        obj.trajectory.push_back(obj.position);
        while (obj.trajectory.size() > MAX_TRAIL_POINTS) {
            obj.trajectory.pop_front();
        }
    }

    // TODO: Обработка столкновений
} 