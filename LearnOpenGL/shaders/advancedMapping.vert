layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 texCoord;

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
    vec3 wTangent;
    vec3 vPos;
    vec3 vNormal;
    vec3 vTangent;
    vec2 uv;
    vec4 lightSpacePos;
    mat3 m_tbn;
    mat3 mv_tbn;
} vs_out;

void main()
{


	gl_Position = mvp * vec4(pos, 1.0);
    vs_out.wPos = (m * vec4(pos, 1.0)).xyz;
    vs_out.wNormal = normalize(mN * normal);
    vs_out.wTangent = normalize(mN * tangent);
    vs_out.vPos = (mv * vec4(pos, 1.0)).xyz;
    vs_out.vNormal = normalize(mvN * normal);
    vs_out.vTangent = normalize(mvN * tangent);
    vs_out.uv = texCoord;
    vs_out.lightSpacePos = shadowvp * m * vec4(pos, 1.0);
    vec3 m_t = normalize(mN * tangent);
    vec3 m_n = normalize(mN * normal);
    mat3 m_tbn = mat3(m_t, cross(m_n, m_t), m_n);
    vs_out.m_tbn = m_tbn;

    vec3 mv_t = normalize(mvN * tangent);
    vec3 mv_n = normalize(mvN * normal);
    mat3 mv_tbn = mat3(mv_t, cross(mv_n, mv_t), mv_n);
    vs_out.mv_tbn = mv_tbn;


}