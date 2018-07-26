#version 330 core
layout(location = 0) in vec3 aPos;

out vec4 vertColor;

void main()
{
    gl_Position = vec4(aPos, 1);
    vertColor = vec4((aPos.x + 1) / 2, (aPos.y + 1) / 2, aPos.z, 0);
}
