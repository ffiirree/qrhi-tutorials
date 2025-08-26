#version 440

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 offset;

layout (location = 0) out vec3 FragColor;

void main()
{
    gl_Position = vec4(position + offset, 0.0, 1.0f);
    FragColor = color;
}
