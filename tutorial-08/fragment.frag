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
    float shininess;
} material;

layout (std140, binding = 3) uniform Camera {
    vec3 position;
} camera;

layout (binding = 4) uniform sampler2D diffuse;
layout (binding = 5) uniform sampler2D specular;

layout (location = 0) in vec3 Normal;
layout (location = 1) in vec3 FragPosition;
layout (location = 2) in vec2 TexCoord;

layout (location = 0) out vec4 fragColor;

void main()
{
    vec3 normal = normalize(Normal);

    // ambient
    vec3 ambient = light.ambient * texture(diffuse, TexCoord).bgr;

    // diffuse
    vec3 light_direction = normalize(light.position - FragPosition);
    vec3 diffuse = light.diffuse * max(dot(normal, light_direction), 0.0f) * texture(diffuse, TexCoord).bgr;

    // specular
    vec3 view_direction = normalize(-camera.position - FragPosition);
    vec3 reflect_direction = reflect(-light_direction, normal);
    vec3 specular = light.specular *  pow(max(dot(view_direction, reflect_direction), 0.0), material.shininess) * texture(specular, TexCoord).bgr;

    fragColor = vec4(ambient + diffuse + specular, 1.0f);
}
