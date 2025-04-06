// Object.h
#ifndef OBJECT_H
#define OBJECT_H

#include <GL/glew.h> // Для GLuint
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> // Для glm::pi
#include <vector>
#include <cmath> // Для std::cbrt
#include <deque> // Для std::deque

// Включим константы
#include "Constants.h"

// Объявления зависимых функций - УБРАНО, они будут в Graphics.h
// void CreateVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount);
// glm::vec3 sphericalToCartesian(float r, float theta, float phi);

class Object {
public:
    GLuint VAO = 0, VBO = 0;
    glm::vec3 position;
    glm::vec3 velocity;
    size_t vertexCount = 0; // Количество ВЕРШИН (не float'ов)
    float mass;         // кг
    float density;      // кг/м^3
    glm::vec4 color;
    bool initializing = false;
    float radius_sim; // Радиус в единицах симуляции
    float radius_meters;
    bool glow;
    glm::vec3 acceleration = glm::vec3(0.0f);

    // История траектории
    std::deque<glm::vec3> trajectory;

    Object(glm::vec3 initPosition, glm::vec3 initVelocity, float initMass, float initDensity = 5515.0f, glm::vec4 initColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), bool initGlow = false);
    ~Object();

    void UpdateRadius();
    std::vector<float> GenerateSphereVertices(); // Возвращает interleaved vertex data (pos+normal)
    void UpdateVBO(); // Обновляет существующий VBO (если нужен)
    glm::vec3 GetPos() const;
    bool CheckCollision(const Object& other) const;

    // Запрещаем копирование
    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
    // Разрешаем перемещение
    Object(Object&& other) noexcept;
    Object& operator=(Object&& other) noexcept;
};

#endif // OBJECT_H 