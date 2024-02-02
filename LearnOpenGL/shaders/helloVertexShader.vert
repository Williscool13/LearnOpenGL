#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texCoord;

uniform mat4 mvp;


out vec3 vertexColor;
out vec2 TexCoord;

void main()
{
    gl_Position = mvp * vec4(pos, 1.0);
    vertexColor = color;
    TexCoord = texCoord;
}