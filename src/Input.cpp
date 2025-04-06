// Input.cpp
#include "Input.h"
#include "Constants.h"
#include "Graphics.h"
#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <random> // Для генерации случайных цветов
#include "imgui.h" // Используем прямой include, т.к. vendor/imgui есть в путях

namespace Input {

// Определения глобальных переменных (из Input.h)
bool running = true;
bool pause = false;
bool updateGridEnabled = true;
bool mouseCaptured = false;
bool firstMouse = true;

glm::vec3 cameraPos   = glm::vec3(0.0f, 2000.0f, 15000.0f); // Начальная позиция камеры
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw   = -90.0f; // Рыскание инициализировано так, чтобы смотреть в -Z
float pitch = 0.0f;  // Тангаж
float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;
float cameraSpeed = CAMERA_SPEED_BASE; // Скорость камеры
float cameraFov = 45.0f; // Поле зрения (Zoom)

std::vector<Object> objs; // Глобальный вектор объектов
int selectedObjectIndex = -1; // Ничего не выбрано
float gridDeformationStrength = DEFORMATION_STRENGTH; // Инициализация константой

// Функция для установки всех колбэков
void SetCallbacks(GLFWwindow* window) {
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetFramebufferSizeCallback(window, Graphics::framebuffer_size_callback);
}

// Коллбэк клавиатуры
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)window; (void)scancode; (void)mods; // Неиспользуемые

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_Q: running = false; break;
            case GLFW_KEY_ESCAPE: running = false; break;
            case GLFW_KEY_P: pause = !pause; break;
            case GLFW_KEY_G: updateGridEnabled = !updateGridEnabled; break;
            case GLFW_KEY_M: 
                mouseCaptured = !mouseCaptured;
                glfwSetInputMode(window, GLFW_CURSOR, mouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
                if (mouseCaptured) firstMouse = true; // Сброс firstMouse при захвате
                break;
            case GLFW_KEY_LEFT_SHIFT: cameraSpeed = CAMERA_SPEED_BASE * 5.0f; break;
        }
    }
     if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_LEFT_SHIFT: cameraSpeed = CAMERA_SPEED_BASE; break;
        }
    }
}

// Коллбэк кнопок мыши
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) { // Если ImGui использует мышь, ничего не делаем
        return;
    }

    (void)window; (void)mods; // Неиспользуемые в остальной части функции

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !mouseCaptured) {
        std::cout << "[INFO] Left mouse button pressed (GUI interaction or future selection)" << std::endl;
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        if (mouseCaptured) {
            // Параметры нового объекта
            float offset = 1500.0f; // Расстояние от камеры
            float speed = 1000.0f;  // Начальная скорость
            float mass = 5.972e20f; // Масса (поменьше, чем у планет)
            float density = 5515.0f;
            // Случайный цвет
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.3, 1.0);
            glm::vec4 color = glm::vec4(dis(gen), dis(gen), dis(gen), 1.0f);

            glm::vec3 startPos = cameraPos + cameraFront * offset;
            glm::vec3 startVel = cameraFront * speed;

            std::cout << "[INFO] Adding new object at (" << startPos.x << "," << startPos.y << "," << startPos.z << ")" << std::endl;

            // Добавляем объект в вектор
            objs.emplace_back(startPos, startVel, mass, density, color, false);

            // Сразу создаем для него геометрию
            Object& newObj = objs.back(); // Получаем ссылку на добавленный объект
            Graphics::SetupObjectGeometry(newObj);

            std::cout << "[INFO] New object added. Total objects: " << objs.size()
                      << ", VAO: " << newObj.VAO << ", VBO: " << newObj.VBO
                      << ", vertexCount: " << newObj.vertexCount << std::endl;
        } else {
             std::cout << "[INFO] Right mouse button pressed (mouse not captured, no object added)." << std::endl;
        }
    }
}

// Коллбэк скролла мыши (для Zoom/FOV)
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) { // Если ImGui использует мышь, ничего не делаем
        return;
    }

    (void)window; (void)xoffset; // Неиспользуемые

    cameraFov -= (float)yoffset;
    if (cameraFov < 1.0f) cameraFov = 1.0f;
    if (cameraFov > 90.0f) cameraFov = 90.0f;
}

// Коллбэк движения мыши (для вращения камеры)
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    // Этот колбэк активен только когда mouseCaptured == true,
    // а UI виден когда mouseCaptured == false, поэтому здесь проверка io.WantCaptureMouse не нужна.

    if (!mouseCaptured) return;

    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos; // Обратный порядок, т.к. Y-координаты идут снизу вверх
    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = CAMERA_SENSITIVITY;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Ограничение тангажа, чтобы избежать переворота
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // Обновление вектора cameraFront
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// Обработка непрерывного ввода (удержание клавиш)
void processInput(GLFWwindow* window, float dt) {
    float currentCameraSpeed = cameraSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += currentCameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= currentCameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * currentCameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * currentCameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) cameraPos += currentCameraSpeed * cameraUp; // Вверх
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) cameraPos -= currentCameraSpeed * cameraUp; // Вниз
}

// Функция получения матрицы вида
glm::mat4 GetViewMatrix() {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

// Функция получения FOV (Zoom)
float GetCameraZoom() {
    return cameraFov;
}

} // namespace Input 