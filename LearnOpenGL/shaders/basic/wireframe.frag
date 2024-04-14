#version 330 core

layout (location = 0) out vec4 color;

in GS_OUT {
    vec3 wPos;
    vec3 wNormal;
    vec3 wTangent;
    vec3 vPos;
    vec3 vNormal;
    vec3 vTangent;
    vec2 uv;
    vec4 lightSpacePos;
    mat3 m_tbn;
    mat3 mv_tbn;
} fs_in;

void main()
{
    color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}