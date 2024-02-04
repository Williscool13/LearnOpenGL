#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 mv;
uniform mat4 mvp;

uniform mat3 mvNormal;


out vec3 viewPosition;
out vec3 viewNormal;
out vec2 uv;


void main()
{
    gl_Position = mvp * vec4(pos, 1.0);
    viewPosition = (mv * vec4(pos, 1.0)).xyz;
    viewNormal = normalize(mvNormal * normal);
    uv = texCoord;
}