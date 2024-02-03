#version 330 core
layout (location = 0) in vec3 pos;
//layout (location = 1) in vec3 color;
//layout (location = 2) in vec2 texCoord;
layout (location = 1) in vec3 normal;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 mv;
uniform mat4 mvp;

uniform mat3 mvNormal;


out vec3 viewPosition;
out vec3 viewNormal;

//out vec3 vertexColor;
//out vec2 TexCoord;

void main()
{
    //gl_Position = vec4(pos * 0.05f , 1.0);
    gl_Position = mvp * vec4(pos, 1.0);
    viewPosition = (mv * vec4(pos, 1.0)).xyz;
    viewNormal = normalize(mvNormal * normal);

    //vertexColor = color;
    //TexCoord = texCoord;
}