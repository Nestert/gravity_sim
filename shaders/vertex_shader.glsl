#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // Используем нормали

uniform mat4 model; // Раскомментировано
uniform mat4 view;
uniform mat4 projection;

// Можно добавить out переменные для освещения, если нужно
// out vec3 FragPos;
// out vec3 Normal;

void main()
{
    // FragPos = vec3(model * vec4(aPos, 1.0));
    // Normal = mat3(transpose(inverse(model))) * aNormal;
    // Применяем все матрицы, включая model
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}