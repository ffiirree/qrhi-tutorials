#version 440

layout (location = 0) in vec3 Normal;
layout (location = 1) in vec3 FragPosition;

layout (location = 0) out vec4 fragColor;

void main()
{
    vec3 color = vec3(0.75f, 0.75f, 0.75f);
    vec3 position = vec3(0.0f, 4.0f, 1.0f);
    vec3 camera = vec3(0.0f, 0.0f, -8.0f);

    vec3 normal = normalize(Normal);

    // ambient
    vec3 ambient = vec3(0.35f);

    // diffuse
    vec3 light_direction = normalize(position - FragPosition);
    vec3 diffuse = max(dot(normal, light_direction), 0.0f) * vec3(1.0f, 1.0f, 1.0f);

    // specular
    vec3 view_direction = normalize(-camera - FragPosition);
    vec3 reflect_direction = reflect(-light_direction, normal);
    vec3 specular = pow(max(dot(view_direction, reflect_direction), 0.0), 32) * 0.35 * vec3(1.0f, 1.0f, 1.0f);

    fragColor = vec4((ambient + diffuse + specular) * color, 0.5f);
}
