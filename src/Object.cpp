// Object.cpp
#include "Object.h"
#include <vector>
#include <iostream> // Для отладки, если потребуется
#include <utility> // Для std::move
#include <GL/glew.h>
#include <glm/gtc/constants.hpp> // Для glm::pi

namespace {
// Вспомогательная функция для конвертации сферических координат в декартовы
// Статическая, видна только в этом файле
glm::vec3 sphericalToCartesian(float r, float theta, float phi) {
    return glm::vec3(
        r * sin(theta) * cos(phi),
        r * cos(theta), // Y - вверх
        r * sin(theta) * sin(phi)
    );
}
} // анонимное пространство имен

// Реализация конструктора
Object::Object(glm::vec3 initPosition, glm::vec3 initVelocity, float initMass, float initDensity, glm::vec4 initColor, bool initGlow)
    : position(initPosition), velocity(initVelocity), mass(initMass), density(initDensity), color(initColor), glow(initGlow)
{
    UpdateRadius();
    // Добавляем начальную позицию в траекторию
    trajectory.push_back(position);
    // Удален вызов CreateVBOVAO - геометрия создается позже в Graphics::SetupObjectGeometry
    // std::vector<float> vertices = GenerateSphereVertices();
    // vertexCount = vertices.size();
    // CreateVBOVAO(VAO, VBO, vertices.data(), vertexCount);
}

// Реализация деструктора
Object::~Object() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
}

// Реализация UpdateRadius
void Object::UpdateRadius() {
    if (density <= 0 || mass <= 0) {
         radius_meters = 0.0f;
         radius_sim = 0.0f;
         return;
    }
    radius_meters = std::cbrt((3.0f * mass / density) / (4.0f * glm::pi<float>()));
    radius_sim = (radius_meters / SIMULATION_SCALE) * RADIUS_DISPLAY_SCALE;
}

// Реализация GenerateSphereVertices (исправлено для GL_TRIANGLES)
std::vector<float> Object::GenerateSphereVertices() {
    std::vector<float> vertices;
    int stacks = 20; // Увеличим детализацию
    int sectors = 20;
    if (radius_sim <= 0) return vertices;

    float lengthInv = 1.0f / radius_sim; // Для нормализации нормалей

    for(int i = 0; i < stacks; ++i) { // Идем до stacks-1
        float theta1 = (static_cast<float>(i) / stacks) * glm::pi<float>();
        float theta2 = (static_cast<float>(i + 1) / stacks) * glm::pi<float>();

        for (int j = 0; j < sectors; ++j) { // Идем до sectors-1
            float phi1 = (static_cast<float>(j) / sectors) * 2.0f * glm::pi<float>();
            float phi2 = (static_cast<float>(j + 1) / sectors) * 2.0f * glm::pi<float>();

            // Вычисляем 4 вершины для текущего сегмента (квадрата)
            glm::vec3 p1 = ::sphericalToCartesian(radius_sim, theta1, phi1);
            glm::vec3 p2 = ::sphericalToCartesian(radius_sim, theta1, phi2);
            glm::vec3 p3 = ::sphericalToCartesian(radius_sim, theta2, phi1);
            glm::vec3 p4 = ::sphericalToCartesian(radius_sim, theta2, phi2);

            // Нормали - это нормализованные векторы позиций для сферы с центром в (0,0,0)
            glm::vec3 n1 = p1 * lengthInv;
            glm::vec3 n2 = p2 * lengthInv;
            glm::vec3 n3 = p3 * lengthInv;
            glm::vec3 n4 = p4 * lengthInv;

            // Первый треугольник (p1, p3, p2)
            vertices.insert(vertices.end(), {p1.x, p1.y, p1.z, n1.x, n1.y, n1.z});
            vertices.insert(vertices.end(), {p3.x, p3.y, p3.z, n3.x, n3.y, n3.z});
            vertices.insert(vertices.end(), {p2.x, p2.y, p2.z, n2.x, n2.y, n2.z});

            // Второй треугольник (p2, p3, p4)
            vertices.insert(vertices.end(), {p2.x, p2.y, p2.z, n2.x, n2.y, n2.z});
            vertices.insert(vertices.end(), {p3.x, p3.y, p3.z, n3.x, n3.y, n3.z});
            vertices.insert(vertices.end(), {p4.x, p4.y, p4.z, n4.x, n4.y, n4.z});
        }
    }
    return vertices;
}

// Реализация UpdateVBO
void Object::UpdateVBO() {
    if (VAO == 0 || VBO == 0) return;
    std::vector<float> vertices = GenerateSphereVertices();
    if (vertices.empty()) return;

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Реализация GetPos
glm::vec3 Object::GetPos() const {
    return position;
}

// Реализация CheckCollision
bool Object::CheckCollision(const Object& other) const {
    if (radius_sim <= 0 || other.radius_sim <= 0) return false;
    float distSq = glm::dot(other.position - position, other.position - position);
    float radiiSum = radius_sim + other.radius_sim;
    return distSq < (radiiSum * radiiSum);
}

// Реализация перемещающего конструктора (исправлен порядок)
Object::Object(Object&& other) noexcept
    : VAO(other.VAO), VBO(other.VBO), position(std::move(other.position)), velocity(std::move(other.velocity)),
      mass(other.mass), density(other.density), color(std::move(other.color)), initializing(other.initializing),
      radius_sim(other.radius_sim), radius_meters(other.radius_meters),
      glow(other.glow), acceleration(std::move(other.acceleration)),
      vertexCount(other.vertexCount),
      trajectory(std::move(other.trajectory)) // Перемещаем траекторию
{
    other.VAO = 0;
    other.VBO = 0;
    other.vertexCount = 0;
    // other.trajectory очистится при перемещении
}

// Реализация перемещающего оператора присваивания
Object& Object::operator=(Object&& other) noexcept {
    if (this != &other) {
        // Освобождаем свои ресурсы
         if (VAO != 0) glDeleteVertexArrays(1, &VAO);
         if (VBO != 0) glDeleteBuffers(1, &VBO);

        // Перемещаем данные
        VAO = other.VAO; VBO = other.VBO; position = std::move(other.position); velocity = std::move(other.velocity);
        mass = other.mass; density = other.density; color = std::move(other.color); initializing = other.initializing;
        radius_sim = other.radius_sim; radius_meters = other.radius_meters; glow = other.glow; acceleration = std::move(other.acceleration);
        vertexCount = other.vertexCount;
        trajectory = std::move(other.trajectory); // Перемещаем траекторию

        // Обнуляем ресурсы у источника
        other.VAO = 0; other.VBO = 0; other.vertexCount = 0;
    }
    return *this;
} 