#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;
uniform mat4 mv;
uniform mat4 mvp;

uniform mat3 mvN;

uniform mat4 i_m;
uniform mat4 i_v;
uniform mat4 i_p;

out vec3 mPos;
out vec3 vPos;
out vec3 vNormal;
out vec2 vTexCoord;
out vec3 dirTexCoord;


void main()
{
	gl_Position = mvp * vec4(pos, 1.0);
	vPos = (mv * vec4(pos, 1.0)).xyz;
	vNormal = normalize(mvN * normal);
	vTexCoord = texCoord;

	dirTexCoord = (i_v * vec4(reflect(vPos, vNormal), 0.0)).xyz;
}