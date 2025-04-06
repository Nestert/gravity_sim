// Input.h
#ifndef INPUT_H
#define INPUT_H

#include <glm/glm.hpp>
#include <vector> // Для доступа к вектору объектов

// Включим Object.h ПЕРЕД glfw3.h, т.к. Object.h включает glew.h
#include "Object.h"
#include <GLFW/glfw3.h> // Для GLFWwindow и ключей/кнопок

namespace Input {

// Глобальные переменные состояния (определения в Input.cpp)
extern bool running;
extern bool pause;
extern bool updateGridEnabled;
extern bool mouseCaptured;
extern bool firstMouse;

extern glm::vec3 cameraPos;
extern glm::vec3 cameraFront;
extern glm::vec3 cameraUp;
extern float yaw;
extern float pitch;
extern float lastX;
extern float lastY;
extern float cameraSpeed; // Базовая скорость
extern float cameraFov;   // Поле зрения (Zoom)

extern std::vector<Object> objs; // Глобальный вектор объектов
extern int selectedObjectIndex; // Индекс выбранного объекта
extern float gridDeformationStrength; // Сила искажения сетки (для ImGui)

// Новые переменные для управления физикой
extern float minGravityDistanceMeters;
extern float simulationScale;
extern float massMultiplier;

// Прототипы функций
void SetCallbacks(GLFWwindow* window);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void processInput(GLFWwindow* window, float dt);

// Функции доступа к камере
glm::mat4 GetViewMatrix();
float GetCameraZoom();

} // namespace Input

#endif // INPUT_H 