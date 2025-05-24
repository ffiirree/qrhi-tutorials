#version 440

layout (std140, binding = 0) uniform buf
{
    mat4 mvp;
    vec3 light;
} ubuf;

layout (location = 0) in vec4 position;
layout (location = 0) out vec2 texCoord;

void main()
{
    gl_Position = ubuf.mvp * position;
}
