#version 330 core

layout (location = 0) in vec3 pos;

layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;
uniform mat4 mv;
uniform mat4 mvp;

uniform mat3 mN;
uniform mat3 mvN;


uniform mat4 shadowvp;

out VS_OUT {
    vec3 wPos;
    vec3 wNormal;
    vec3 vPos;
    vec3 vNormal;
    vec2 uv;
    vec4 lightSpacePos;
} vs_out;

void main()
{
	gl_Position = mvp * vec4(pos, 1.0);
    vs_out.wPos = (m * vec4(pos, 1.0)).xyz;
    vs_out.wNormal = normalize(mN * normal);
    vs_out.vPos = (mv * vec4(pos, 1.0)).xyz;
    vs_out.vNormal = normalize(mvN * normal);
    vs_out.uv = texCoord;
    vs_out.lightSpacePos = shadowvp * m * vec4(pos, 1.0);
    //vPos = (mv * vec4(pos, 1.0)).xyz;
    //vNormal = normalize(mvN * normal);
    //uv = texCoord;
    //lightSpacePos = shadowvp * m * vec4(pos, 1.0);
}