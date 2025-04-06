// main.cpp
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <string>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <limits> // Для numeric_limits в ImGui::DragFloat
#include <utility> // Для std::pair (хотя не строго нужен для structured binding)

#include "Constants.h"
#include "Graphics.h"
#include "Input.h"    // Включаем для доступа к глобальным переменным состояния и processInput
#include "Simulation.h"
#include "Object.h"   // Включаем для доступа к глобальному std::vector<Object> objs

// Глобальные переменные для main.cpp
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
    std::cout << "[INFO] Entering main function..." << std::endl;
    GLFWwindow* window = Graphics::StartGLU(); // Используем Graphics::StartGLU
    if (!window) {
        std::cerr << "[FATAL] Failed to initialize GLFW/GLEW or create window." << std::endl;
        return -1;
    }
    std::cout << "[INFO] GLFW and GLEW initialized successfully." << std::endl;

    // --- СНАЧАЛА устанавливаем НАШИ колбэки ---
    Input::SetCallbacks(window);
    std::cout << "[DEBUG] Input callbacks set." << std::endl;

    // --- ПОТОМ Настройка Dear ImGui ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    const char* glsl_version = "#version 330";
    // ImGui теперь должен "обернуть" наши колбэки
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    std::cout << "[INFO] ImGui initialized successfully." << std::endl;

    // --- Установка режима курсора ПОСЛЕ инициализации ImGui/GLFW колбэков ---
    glfwSetInputMode(window, GLFW_CURSOR, Input::mouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    std::cout << "[DEBUG] Initial cursor mode set." << std::endl;

    // Создание шейдерной программы (GLuint)
    GLuint shaderProgramID = Graphics::CreateShaderProgram("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    GLuint gridShaderProgramID = Graphics::CreateShaderProgram("shaders/grid_vertex.glsl", "shaders/grid_fragment.glsl");
    if (shaderProgramID == 0 || gridShaderProgramID == 0) {
        std::cerr << "[FATAL] Failed to create shader programs." << std::endl;
        // ... очистка и выход ...
        glfwTerminate();
        return -1;
    }
    std::cout << "[DEBUG] Shader Programs OK." << std::endl;

    // Получаем uniform locations для шейдера сетки ЗАРАНЕЕ
    GLint gridDistMinLoc = glGetUniformLocation(gridShaderProgramID, "distortionMin");
    GLint gridDistMaxLoc = glGetUniformLocation(gridShaderProgramID, "distortionMax");
    GLint gridColorMinLoc = glGetUniformLocation(gridShaderProgramID, "colorMin");
    GLint gridColorMaxLoc = glGetUniformLocation(gridShaderProgramID, "colorMax");

    // Переменные для управления цветом сетки из ImGui
    float gridDistortionMin = -1000.0f;
    float gridDistortionMax = 0.0f;
    glm::vec3 gridColorMin = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 gridColorMax = glm::vec3(1.0f, 0.0f, 0.0f);

    // Объекты симуляции (используем глобальный вектор из Input)
    Input::objs.emplace_back(glm::vec3(-5000, 650, -350), glm::vec3(0, 0, 1500), 5.972e22f, 5515, glm::vec4(0.0f, 0.5f, 1.0f, 1.0f));
    Input::objs.emplace_back(glm::vec3(5000, 650, -350), glm::vec3(0, 0, -1500), 5.972e22f, 5515, glm::vec4(0.0f, 1.0f, 0.5f, 1.0f));
    Input::objs.emplace_back(glm::vec3(0, 0, -350), glm::vec3(0, 0, 0), 1.989e24f, 5515, glm::vec4(1.0f, 0.9f, 0.2f, 1.0f), false);
    std::cout << "[DEBUG] Object Creation OK." << std::endl;

    // Создаем геометрию для всех объектов
    for (size_t i = 0; i < Input::objs.size(); ++i) {
        Graphics::SetupObjectGeometry(Input::objs[i]);
        std::cout << "[DEBUG] Setup Object Geometry OK for object " << i << std::endl;
    }
    std::cout << "[DEBUG] All Object Geometry OK." << std::endl;

    // Переменные для сетки
    std::vector<float> initialGridVertices = CreateGridVertices(GRID_SIZE, GRID_DIVISIONS);
    std::cout << "[DEBUG] Create Grid Vertices OK." << std::endl;
    // Получаем VAO и VBO из функции
    auto [gridVAO, gridVBO] = Graphics::SetupGridGeometry(initialGridVertices);
    if (gridVAO == 0 || gridVBO == 0) { // Проверяем оба ID
        std::cerr << "[FATAL] Failed to setup grid geometry (VAO or VBO is 0)." << std::endl;
        // ... cleanup and exit ...
        glDeleteProgram(shaderProgramID);
        glDeleteProgram(gridShaderProgramID);
        glfwTerminate();
        return -1;
    }
    std::cout << "[DEBUG] Setup Grid Geometry OK (VAO: " << gridVAO << ", VBO: " << gridVBO << ")." << std::endl;
    std::vector<float> currentGridVertices = initialGridVertices;
    // Сила деформации теперь глобальная в Input::gridDeformationStrength

    std::cout << "[INFO] Starting main loop..." << std::endl;
    lastFrame = static_cast<float>(glfwGetTime());

    // --- Главный цикл --- 
    while (!glfwWindowShouldClose(window) && Input::running) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Обработка ввода (GLFW + наш)
        glfwPollEvents();
        Input::processInput(window, deltaTime); // Используем processInput из Input

        // --- Начало нового кадра ImGui ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- Создание интерфейса ImGui ---
        if (!Input::mouseCaptured) {
            if (ImGui::Begin("Simulation Controls")) {
                // --- Секция Grid Deformation ---
                if (ImGui::CollapsingHeader("Grid Deformation Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                     // Ползунок для DEFORMATION_STRENGTH (используем Input::gridDeformationStrength)
                     const float min_strength = 1.0e-19f;
                     const float max_strength = 1.0e-14f; // Расширим диапазон вверх
                     ImGui::DragFloat("Deformation Strength##Grid", &Input::gridDeformationStrength, 1.0e-18f, min_strength, max_strength, "%.3e", ImGuiSliderFlags_Logarithmic);

                     ImGui::Separator();
                     ImGui::Text("Grid Color Mapping");
                     ImGui::DragFloat("Min Distortion##GridColor", &gridDistortionMin, 10.0f, -100000.0f, gridDistortionMax - 1.0f, "%.1f");
                     ImGui::DragFloat("Max Distortion##GridColor", &gridDistortionMax, 10.0f, gridDistortionMin + 1.0f, 10000.0f, "%.1f");
                     ImGui::ColorEdit3("Min Color##GridColor", glm::value_ptr(gridColorMin));
                     ImGui::ColorEdit3("Max Color##GridColor", glm::value_ptr(gridColorMax));
                }
                ImGui::Separator();

                // --- Секция Simulation State ---
                if (ImGui::CollapsingHeader("Simulation State", ImGuiTreeNodeFlags_DefaultOpen)) {
                    if (ImGui::Checkbox("Pause (P)", &Input::pause)) {
                         std::cout << "[INFO] Simulation " << (Input::pause ? "paused" : "resumed") << " via GUI" << std::endl;
                    }
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Update Grid (G)", &Input::updateGridEnabled)) {
                         std::cout << "[INFO] Grid update enabled: " << (Input::updateGridEnabled ? "true" : "false") << " via GUI" << std::endl;
                    }
                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                }
            }
            ImGui::End();
        }

        // --- Обновление физики ---
        if (!Input::pause) {
            UpdatePhysics(Input::objs, deltaTime); // Обновляем глобальный вектор Input::objs
        }

        // --- Рендеринг сцены ---
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Настройка камеры и проекции
        glm::mat4 projection = glm::perspective(glm::radians(Input::GetCameraZoom()), (float)display_w / (float)display_h, 0.1f, 100000.0f);
        glm::mat4 view = Input::GetViewMatrix();

        // Обновление сетки, если включено
        if (Input::updateGridEnabled) {
            float verticalShift = CalculateVerticalShift(Input::objs);
            // Передаем Input::gridDeformationStrength, управляемый через ImGui
            currentGridVertices = UpdateGridVertices(initialGridVertices, Input::objs, verticalShift, Input::gridDeformationStrength);
            // Обновление VBO
            if (gridVBO != 0 && !currentGridVertices.empty()) {
                 glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
                 glBufferSubData(GL_ARRAY_BUFFER, 0, currentGridVertices.size() * sizeof(float), currentGridVertices.data());
                 glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
        }

        // Рендеринг объектов
        glUseProgram(shaderProgramID);
        GLint projLocObj = glGetUniformLocation(shaderProgramID, "projection");
        GLint viewLocObj = glGetUniformLocation(shaderProgramID, "view");
        glUniformMatrix4fv(projLocObj, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLocObj, 1, GL_FALSE, glm::value_ptr(view));
        for (size_t i = 0; i < Input::objs.size(); ++i) {
             Graphics::DrawObject(shaderProgramID, Input::objs[i], (int)i == Input::selectedObjectIndex);
        }

        // Рендеринг сетки
        glUseProgram(gridShaderProgramID);
        // Установка uniforms для сетки (включая параметры цвета)
        GLint projLocGrid = glGetUniformLocation(gridShaderProgramID, "projection");
        GLint viewLocGrid = glGetUniformLocation(gridShaderProgramID, "view");
        glUniformMatrix4fv(projLocGrid, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLocGrid, 1, GL_FALSE, glm::value_ptr(view));
        // Устанавливаем uniforms для цвета
        if(gridDistMinLoc != -1) glUniform1f(gridDistMinLoc, gridDistortionMin);
        if(gridDistMaxLoc != -1) glUniform1f(gridDistMaxLoc, gridDistortionMax);
        if(gridColorMinLoc != -1) glUniform3fv(gridColorMinLoc, 1, glm::value_ptr(gridColorMin));
        if(gridColorMaxLoc != -1) glUniform3fv(gridColorMaxLoc, 1, glm::value_ptr(gridColorMax));
        Graphics::DrawGrid(gridShaderProgramID, gridVAO, initialGridVertices.size() / 4);

        // --- Рендеринг ImGui ---
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // --- Конец кадра ---
        glfwSwapBuffers(window);
    }

    std::cout << "[INFO] Main loop finished." << std::endl;

    // --- Очистка ImGui ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Очистка ресурсов
    Input::objs.clear();
    glDeleteVertexArrays(1, &gridVAO);
    glDeleteBuffers(1, &gridVBO);
    glDeleteProgram(shaderProgramID); // Удаляем программы по ID
    glDeleteProgram(gridShaderProgramID);

    glfwTerminate();
    return 0;
}
