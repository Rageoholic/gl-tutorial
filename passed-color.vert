#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 ourColor;

out vec4 vertColor;

void main()
{
    gl_Position = vec4(aPos, 1);
    vertColor = vec4(ourColor, 1);
}
