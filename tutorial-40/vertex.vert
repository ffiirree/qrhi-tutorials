#version 440

layout (std140, binding = 0) uniform buf
{
    mat4 mvp;// Model View Projection Matrix
    mat4 finalBonesMatrices[500];
} ubuf;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex;
layout (location = 3) in ivec4 boneIds;
layout (location = 4) in vec4 weights;

layout (location = 0) out vec2 texCoord;

void main()
{
    vec4 total_position = vec4(0.0f);
    for(int i = 0; i < 4; ++i) {
        if(boneIds[i] == -1) continue;

        if(boneIds[i] >= 500) {
            total_position = vec4(position, 1.0f);
            break;
        }

        vec4 local_position = ubuf.finalBonesMatrices[boneIds[i]] * vec4(position, 1.0f);
        total_position += local_position * weights[i];
    }

    texCoord = tex;
    gl_Position = ubuf.mvp * total_position;
}
