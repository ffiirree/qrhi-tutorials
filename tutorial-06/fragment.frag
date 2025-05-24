#version 440

layout (std140, binding = 1) uniform buf
{
    vec3 color;
    vec3 position;
    float ambient;
    vec3 camera;
} light;

layout (location = 0) in vec3 Normal;
layout (location = 1) in vec3 FragPosition;

layout (location = 0) out vec4 fragColor;

void main()
{
    vec3 color = vec3(1.0f, 0.5f, 0.31f);
    vec3 normal = normalize(Normal);

    // ambient
    vec3 ambient = light.ambient * light.color;

    // diffuse
    vec3 light_direction = normalize(light.position - FragPosition);
    vec3 diffuse = max(dot(normal, light_direction), 0.0f) * light.color;

    // specular
    vec3 view_direction = normalize(-light.camera - FragPosition);
    vec3 reflect_direction = reflect(-light_direction, normal);
    vec3 specular = pow(max(dot(view_direction, reflect_direction), 0.0), 32) * 0.35 * light.color;

    fragColor = vec4((ambient + diffuse + specular) * color, 1.0f);
}
