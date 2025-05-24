#version 440

layout (std140, binding = 0) uniform buf
{
    mat4 m;
    mat4 v;
    mat4 p;
} mvp;

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;

layout (location = 0) out vec3 Normal;
layout (location = 1) out vec3 FragPosition;

void main()
{
    gl_Position = mvp.p * mvp.v * mvp.m * position;
    FragPosition = vec3(mvp.m * position);
    // Normal = mat3(transpose(inverse(mvp.m))) * normal;
    Normal = mat3(mvp.m) * normal;
}
