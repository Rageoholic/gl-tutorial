#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 ourTextCoord;

out vec2 texCoord;
out vec4 vertColor;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1);
    texCoord = ourTextCoord;
    vertColor = vec4(2, 2, 2, 2);
}
