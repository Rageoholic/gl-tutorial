#version 330 core
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightingColor;

void main()
{
    FragColor = vec4(objectColor * lightingColor, 1.0f);
}
