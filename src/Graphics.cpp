// Graphics.cpp
#include "Graphics.h"
#include "Object.h"
#include "Constants.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <deque> // Для DrawTrail

namespace Graphics {

// Определения шейдеров (можно вынести в отдельные файлы .glsl и загружать их)
const char* vertexShaderSource = R"glsl(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out float lightIntensity;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vec3 worldPos = (model * vec4(aPos, 1.0)).xyz;
    vec3 normal = normalize(aPos);
    vec3 dirToCenter = normalize(-worldPos);
    lightIntensity = max(dot(normal, dirToCenter), 0.15);
})glsl";

const char* fragmentShaderSource = R"glsl(
#version 330 core
in float lightIntensity;
out vec4 FragColor;
uniform vec4 objectColor;
uniform bool isGrid;
uniform bool GLOW;
void main() {
    if (isGrid) {
        FragColor = objectColor;
    } else if(GLOW){
        FragColor = vec4(objectColor.rgb * 10.0, objectColor.a);
    } else {
        float fade = smoothstep(0.0, 1.0, lightIntensity);
        FragColor = vec4(objectColor.rgb * fade, objectColor.a);
    }
})glsl";

// Helper: Загрузка и компиляция шейдера
GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    // Проверка ошибок компиляции
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "[ERROR] Shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// Helper: Чтение файла шейдера
std::string ReadShaderFile(const char* filePath) {
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        shaderFile.open(filePath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        return shaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "[ERROR] Shader file not successfully read: " << filePath << " (" << e.what() << ")" << std::endl;
        return "";
    }
}

// Инициализация GLFW и GLEW, создание окна
GLFWwindow* StartGLU() {
    if (!glfwInit()) {
        std::cerr << "[FATAL] Failed to initialize GLFW." << std::endl;
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Gravity Simulation", NULL, NULL);
    if (!window) {
        std::cerr << "[FATAL] Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "[FATAL] Failed to initialize GLEW." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return window;
}

// Создание шейдерной программы из файлов
GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode = ReadShaderFile(vertexPath);
    std::string fragmentCode = ReadShaderFile(fragmentPath);
    if (vertexCode.empty() || fragmentCode.empty()) return 0;

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vShaderCode);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fShaderCode);
    if (vertexShader == 0 || fragmentShader == 0) return 0;

    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    glLinkProgram(programID);

    // Проверка ошибок линковки
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        std::cerr << "[ERROR] Shader program linking failed: " << infoLog << std::endl;
        glDeleteProgram(programID);
        programID = 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return programID;
}

// Колбэк изменения размера окна
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
    std::cout << "[INFO] Framebuffer resized: " << width << "x" << height << std::endl;
}

// Настройка геометрии объекта (правильная реализация)
void SetupObjectGeometry(Object& obj) {
    std::vector<float> vertices = obj.GenerateSphereVertices();
    if (vertices.empty()) {
        obj.VAO = 0;
        obj.VBO = 0;
        obj.vertexCount = 0;
        return;
    }

    if (obj.VAO != 0) glDeleteVertexArrays(1, &obj.VAO);
    if (obj.VBO != 0) glDeleteBuffers(1, &obj.VBO);

    glGenVertexArrays(1, &obj.VAO);
    glGenBuffers(1, &obj.VBO);

    glBindVertexArray(obj.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, obj.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    obj.vertexCount = vertices.size() / 6;
}

// Настройка геометрии сетки (правильная реализация)
std::pair<GLuint, GLuint> SetupGridGeometry(const std::vector<float>& vertices) {
    GLuint VAO = 0, VBO = 0;
    if (vertices.empty()) return {0, 0};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    // Атрибут позиции (X, Y, Z) - location 0
    // Stride теперь 4 * sizeof(float)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Атрибут искажения (Distortion) - location 1
    // Stride 4 * sizeof(float), offset 3 * sizeof(float)
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return {VAO, VBO};
}

// Настройка геометрии для отрисовки трейлов (один VAO/VBO на все)
TrailRenderData SetupTrailGeometry() {
    TrailRenderData data;
    glGenVertexArrays(1, &data.VAO);
    glGenBuffers(1, &data.VBO);

    glBindVertexArray(data.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, data.VBO);

    // Просто настраиваем атрибут позиции, данные загрузим позже
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return data;
}

// Отрисовка трейла для одного объекта
void DrawTrail(GLuint shaderProgramID, const TrailRenderData& trailData, const std::deque<glm::vec3>& trajectory, const glm::vec4& color) {
    if (trailData.VAO == 0 || trailData.VBO == 0 || trajectory.size() < 2) {
        return; // Нечего рисовать
    }

    glUseProgram(shaderProgramID);

    // Копируем данные из deque в vector для получения непрерывного блока памяти
    std::vector<glm::vec3> trajectory_vector(trajectory.begin(), trajectory.end());

    // Загружаем текущую траекторию в VBO из вектора
    glBindBuffer(GL_ARRAY_BUFFER, trailData.VBO);
    // Используем данные из временного вектора
    glBufferData(GL_ARRAY_BUFFER, trajectory_vector.size() * sizeof(glm::vec3), trajectory_vector.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Устанавливаем цвет трейла
    GLint colorLoc = glGetUniformLocation(shaderProgramID, "trailColor");
    if (colorLoc != -1) {
        glUniform4fv(colorLoc, 1, glm::value_ptr(color));
    }
    // Установка model матрицы (единичная, т.к. координаты мировые)
    glm::mat4 model = glm::mat4(1.0f);
    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
     if (modelLoc != -1) {
         glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    }

    // Рисуем линию
    glBindVertexArray(trailData.VAO);
    // Используем размер вектора (он совпадает с deque)
    glDrawArrays(GL_LINE_STRIP, 0, trajectory_vector.size());
    glBindVertexArray(0);
}

// Отрисовка объекта (правильная реализация)
void DrawObject(GLuint shaderProgramID, const Object& obj, bool selected) {
    if (obj.VAO == 0 || obj.vertexCount == 0) return;

    glUseProgram(shaderProgramID);

    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    GLint colorLoc = glGetUniformLocation(shaderProgramID, "objectColor");
    GLint glowLoc = glGetUniformLocation(shaderProgramID, "glow");
    GLint selectedLoc = glGetUniformLocation(shaderProgramID, "selected");

    // Матрица модели содержит ТОЛЬКО трансляцию
    glm::mat4 model = glm::translate(glm::mat4(1.0f), obj.position);
    if (modelLoc != -1) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    if (colorLoc != -1) glUniform4fv(colorLoc, 1, glm::value_ptr(obj.color));
    if (glowLoc != -1) glUniform1i(glowLoc, obj.glow ? 1 : 0);
    if (selectedLoc != -1) glUniform1i(selectedLoc, selected ? 1 : 0);

    glBindVertexArray(obj.VAO);
    glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
    glBindVertexArray(0);
}

// Отрисовка сетки (правильная реализация)
void DrawGrid(GLuint shaderProgramID, GLuint gridVAO, size_t vertexCount) {
    if (gridVAO == 0 || vertexCount == 0) return;

    glUseProgram(shaderProgramID);

    glm::mat4 model = glm::mat4(1.0f);
    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    if (modelLoc != -1) {
         glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    }

    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, vertexCount);
    glBindVertexArray(0);
}

} // namespace Graphics