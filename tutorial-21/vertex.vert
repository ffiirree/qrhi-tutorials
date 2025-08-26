#version 440

layout (std140, binding = 0) uniform buf
{
    mat4 m;
    mat4 v;
    mat4 p;
} ubuf;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 offset;

layout (location = 0) out vec3 Normal;
layout (location = 1) out vec3 FragPosition;

void main()
{
    gl_Position = ubuf.p * ubuf.v * ubuf.m * vec4(position.x + offset.x, position.y + offset.y, position.z, 1.0f);
    FragPosition = vec3(ubuf.m * vec4(position, 1.0f));
    Normal = mat3(transpose(inverse(ubuf.m))) * normal;
}
