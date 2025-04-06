// Graphics.h
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <utility> // Добавлено для std::pair

#include "Object.h" // Для DrawObject и SetupObjectGeometry

namespace Graphics {

// Прототипы функций
GLFWwindow* StartGLU();
GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath);
void SetupObjectGeometry(Object& obj);
std::pair<GLuint, GLuint> SetupGridGeometry(const std::vector<float>& vertices);
void DrawObject(GLuint shaderProgramID, const Object& obj, bool selected);
void DrawGrid(GLuint shaderProgramID, GLuint gridVAO, size_t vertexCount);

// Колбэк
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

} // namespace Graphics

#endif // GRAPHICS_H 