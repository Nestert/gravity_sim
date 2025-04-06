// Graphics.h
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <utility> // Добавлено для std::pair
#include <deque> // Для DrawTrail

#include "Object.h" // Для DrawObject и SetupObjectGeometry

namespace Graphics {

// Структура для хранения VAO/VBO трейлов
struct TrailRenderData {
    GLuint VAO = 0;
    GLuint VBO = 0;
};

// Прототипы функций
GLFWwindow* StartGLU();
GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath);
void SetupObjectGeometry(Object& obj);
std::pair<GLuint, GLuint> SetupGridGeometry(const std::vector<float>& vertices);
// Функции для трейлов
TrailRenderData SetupTrailGeometry();
void DrawTrail(GLuint shaderProgramID, const TrailRenderData& trailData, const std::deque<glm::vec3>& trajectory, const glm::vec4& color);
void DrawObject(GLuint shaderProgramID, const Object& obj, bool selected);
void DrawGrid(GLuint shaderProgramID, GLuint gridVAO, size_t vertexCount);

// Колбэк
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

} // namespace Graphics

#endif // GRAPHICS_H 