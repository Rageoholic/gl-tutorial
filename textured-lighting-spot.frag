#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform vec3 viewPos;

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct SpotLight
{
    vec3 pos;
    vec3 dir;
    float cutOff;
    float outerCutOff;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

uniform Material material;

uniform SpotLight light;

void main()
{

    vec3 lightDir = normalize(light.pos - FragPos);
    float theta = dot(lightDir, normalize(-light.dir));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // Only relevant for Point Lights. Multiply everything by attenuation
    float distance = length(light.pos - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                               light.quadratic * (distance * distance));

    // ambient

    vec3 ambient = vec3(texture(material.diffuse, TexCoord)) * light.ambient;
    // ambient *= attenuation;

    // diffuse
    vec3 norm = normalize(Normal);

    // If directional lightdir = normalize(-light.dir);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, TexCoord));
    diffuse *= attenuation * intensity;

    //specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0), material.shininess);
    vec3 specular = vec3(texture(material.specular, TexCoord)) * spec * light.specular;
    specular *= attenuation * intensity;

    vec3 result = (ambient + diffuse + specular);
    FragColor = vec4(result, 1.0);
}
