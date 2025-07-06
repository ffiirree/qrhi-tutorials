#version 440

layout (std140, binding = 0) uniform buf
{
    mat4 mvp;// Model View Projection Matrix
    mat4 bone_matrices[500];
} ubuf;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex;
layout (location = 3) in ivec4 bone_ids;
layout (location = 4) in vec4 bone_weights;

layout (location = 0) out vec2 texCoord;

void main()
{
    vec4 total_position = vec4(0.0f);
    for (int i = 0; i < 4; ++i) {
        if (bone_ids[i] == -1) continue;

        if (bone_ids[i] >= 500) {
            total_position = vec4(position, 1.0f);
            break;
        }

        vec4 local_position = ubuf.bone_matrices[bone_ids[i]] * vec4(position, 1.0f);
        total_position += local_position * bone_weights[i];
    }

    texCoord = tex;
    gl_Position = ubuf.mvp * total_position;
}
