#version 440

layout (std140, binding = 0) uniform buf
{
    mat4 mvp;// Model View Projection Matrix
} ubuf;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex;

layout (location = 0) out vec2 texCoord;

void main()
{
    texCoord = tex;
    gl_Position = ubuf.mvp * vec4(position, 1.0f);
}
