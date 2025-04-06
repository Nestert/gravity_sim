#version 330 core
out vec4 FragColor;

in float vDistortion; // Получаем искажение от вершинного шейдера

// Цвета для интерполяции
uniform vec3 colorMin = vec3(0.0, 0.0, 1.0); // Синий (малое искажение)
uniform vec3 colorMax = vec3(1.0, 0.0, 0.0); // Красный (большое искажение)

// Диапазон искажений для нормализации (нужно подобрать экспериментально)
uniform float distortionMin = -1000.0; 
uniform float distortionMax = 0.0;    

void main()
{
    // Нормализуем искажение к диапазону [0, 1]
    float t = smoothstep(distortionMin, distortionMax, vDistortion);
    t = clamp(t, 0.0, 1.0);

    // Линейная интерполяция цвета
    vec3 finalColor = mix(colorMin, colorMax, t);

    // Добавляем прозрачность
    FragColor = vec4(finalColor, 0.6); 
}