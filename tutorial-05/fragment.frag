#version 440

layout (std140, binding = 0) uniform buf
{
    mat4 mvp;
    vec3 light;
} ubuf;

layout (location = 0) in vec2 texCoord;
layout (location = 0) out vec4 fragColor;

void main()
{
    vec3 color = vec3(1.0f, 0.5f, 0.31f);
    fragColor = vec4(ubuf.light * color, 1.0f);
}
