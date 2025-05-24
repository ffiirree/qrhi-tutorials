#version 440

layout (std140, binding = 1) uniform Light
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} light;

layout (std140, binding = 2) uniform Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} material;

layout (std140, binding = 3) uniform Camera {
    vec3 position;
} camera;

layout (location = 0) in vec3 Normal;
layout (location = 1) in vec3 FragPosition;

layout (location = 0) out vec4 fragColor;

void main()
{
    vec3 normal = normalize(Normal);

    // ambient
    vec3 ambient = light.ambient * material.ambient;

    // diffuse
    vec3 light_direction = normalize(light.position - FragPosition);
    vec3 diffuse = light.diffuse * max(dot(normal, light_direction), 0.0f) * material.diffuse;

    // specular
    vec3 view_direction = normalize(-camera.position - FragPosition);
    vec3 reflect_direction = reflect(-light_direction, normal);
    vec3 specular = light.specular *  pow(max(dot(view_direction, reflect_direction), 0.0), material.shininess) * material.specular;

    fragColor = vec4(ambient + diffuse + specular, 1.0f);
}
