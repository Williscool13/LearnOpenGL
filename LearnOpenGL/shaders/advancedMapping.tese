#version 410 core
layout (triangles, equal_spacing, ccw) in;

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;
uniform mat4 mv;
uniform mat4 mvp;

uniform mat3 mN;
uniform mat3 mvN;


uniform mat4 shadowvp;
uniform sampler2D displacementTexture;

in TESC_OUT {
    vec3 pos;
    vec3 normal;
    vec3 tangent;
    vec2 uv;
} tese_in[];

out TESE_OUT {
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
} tese_out;

vec3 interpolate(vec3 v0, vec3 v1, vec3 v2){
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}
vec2 interpolate(vec2 v0, vec2 v1, vec2 v2){
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}
vec4 interpolate(vec4 v0, vec4 v1, vec4 v2){
    return vec4(gl_TessCoord.x) * v0 + vec4(gl_TessCoord.y) * v1 + vec4(gl_TessCoord.z) * v2;
}

void main(){
    vec3 interp_pos = tese_in[0].pos * gl_TessCoord.x + tese_in[1].pos * gl_TessCoord.y + tese_in[2].pos * gl_TessCoord.z;
    vec3 interp_normal = tese_in[0].normal * gl_TessCoord.x + tese_in[1].normal * gl_TessCoord.y + tese_in[2].normal * gl_TessCoord.z;
    vec3 interp_tangent = tese_in[0].tangent * gl_TessCoord.x + tese_in[1].tangent * gl_TessCoord.y + tese_in[2].tangent * gl_TessCoord.z;
    vec2 interp_texCoord = tese_in[0].uv * gl_TessCoord.x + tese_in[1].uv * gl_TessCoord.y + tese_in[2].uv * gl_TessCoord.z;

    // displace vertex position
    interp_pos += interp_normal * texture(displacementTexture, interp_texCoord).r * 10.0f;
    gl_Position = mvp * vec4(interp_pos, 1.0);

    tese_out.wPos = (m * vec4(interp_pos, 1.0)).xyz;
    tese_out.wNormal = normalize(mN * interp_normal);
    tese_out.wTangent = normalize(mN * interp_tangent);
    tese_out.vPos = (mv * vec4(interp_pos, 1.0)).xyz;
    tese_out.vNormal = normalize(mvN * interp_normal);
    tese_out.vTangent = normalize(mvN * interp_tangent);
    tese_out.uv = interp_texCoord;
    tese_out.lightSpacePos = shadowvp * m * vec4(interp_pos, 1.0);
    vec3 m_t = normalize(mN * interp_tangent);
    vec3 m_n = normalize(mN * interp_normal);
    mat3 m_tbn = mat3(m_t, cross(m_n, m_t), m_n);
    tese_out.m_tbn = m_tbn;

    vec3 mv_t = normalize(mvN * interp_tangent);
    vec3 mv_n = normalize(mvN * interp_normal);
    mat3 mv_tbn = mat3(mv_t, cross(mv_n, mv_t), mv_n);
    tese_out.mv_tbn = mv_tbn;
}