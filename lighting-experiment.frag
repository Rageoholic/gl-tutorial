#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 objectColor;
uniform vec3 viewPos;

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light
{
    vec3 pos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;

uniform Light light;

void main()
{
    // ambient

    vec3 ambient = material.ambient * light.ambient;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.pos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * material.diffuse;

    //specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0), material.shininess);
    vec3 specular = material.specular * spec * light.specular;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
