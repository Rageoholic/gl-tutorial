#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 ourColor;
layout(location = 2) in vec2 ourTextCoord;

out vec4 vertColor;
out vec2 texCoord;

void main()
{
    gl_Position = vec4(aPos, 1);
    vertColor = vec4(ourColor, 1);
    texCoord = ourTextCoord;
}
