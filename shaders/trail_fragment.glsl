#version 330 core
out vec4 FragColor;

uniform vec4 trailColor;

void main()
{
    FragColor = trailColor;
}