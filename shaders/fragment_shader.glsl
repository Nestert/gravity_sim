#version 330 core
out vec4 FragColor;

// Можно добавить in переменные для освещения
// in vec3 FragPos;
// in vec3 Normal;

uniform vec4 objectColor;
uniform bool glow;
uniform bool selected;

void main()
{
    vec4 finalColor = objectColor;
    if (selected) {
        // Делаем выбранный объект ярче/желтее
        finalColor.rgb += vec3(0.3, 0.3, 0.0);
    }
    // Здесь можно добавить логику освещения, используя Normal и FragPos
    
    // Или эффект свечения на основе glow
    if (glow) {
         finalColor.rgb *= 1.5; // Просто делаем ярче
    }

    FragColor = finalColor;
}