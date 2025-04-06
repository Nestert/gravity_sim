// Simulation.cpp
#include "Simulation.h"
#include <cmath>     // Для std::sqrt
#include <iostream> // Для отладки
#include <numeric> // для std::accumulate

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

// Обновление физики объектов
void UpdatePhysics(std::vector<Object>& objs, float dt) {
    if (dt <= 0) return; // Не обновлять, если время не идет

    // 1. Рассчитать ускорения для всех объектов
    for (auto& obj1 : objs) {
        obj1.acceleration = glm::vec3(0.0f);
        if (obj1.initializing) continue;

        for (const auto& obj2 : objs) {
            if (&obj1 == &obj2 || obj2.initializing) continue;

            glm::vec3 direction = obj2.position - obj1.position;
            float distance_sim = glm::length(direction);
            float distance_sim_safe = glm::max(distance_sim, MIN_GRAVITY_DISTANCE_SIM);

            float distance_meters = distance_sim_safe * SIMULATION_SCALE;
            if (distance_meters > 0 && obj1.mass > 0) { // Добавлена проверка массы obj1
                 double force_magnitude = (GRAVITATIONAL_CONSTANT * obj1.mass * obj2.mass) / (distance_meters * distance_meters);
                 glm::vec3 force_vector = glm::normalize(direction) * static_cast<float>(force_magnitude);
                 glm::vec3 acceleration_meters = force_vector / obj1.mass;
                 obj1.acceleration += (acceleration_meters / SIMULATION_SCALE);
            }
        }
    }

    // 2. Обновить скорости и позиции
    for (auto& obj : objs) {
         if (obj.initializing) continue;
         obj.velocity += obj.acceleration * dt;
         obj.position += obj.velocity * dt;
    }

    // TODO: Добавить обработку столкновений после обновления позиций
} 