#version 330 core
layout(location = 0) in vec3 aPos;

out vec4 vertColor;
uniform vec4 uniformColor;
void main()
{
    gl_Position = vec4(aPos, 1);
    vertColor = uniformColor;
}
