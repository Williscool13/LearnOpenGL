#version 410 core
layout (triangles, equal_spacing, ccw) in;

uniform sampler2D displacementTexture;

uniform mat4 m;

in TESC_OUT {
    vec3 pos;
    vec3 normal;
    vec3 tangent;
    vec2 uv;
} tese_in[];

vec3 interpolate(vec3 v0, vec3 v1, vec3 v2){
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}
vec2 interpolate(vec2 v0, vec2 v1, vec2 v2){
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

void main(){
    vec3 interp_pos = tese_in[0].pos * gl_TessCoord.x + tese_in[1].pos * gl_TessCoord.y + tese_in[2].pos * gl_TessCoord.z;
    vec3 interp_normal = tese_in[0].normal * gl_TessCoord.x + tese_in[1].normal * gl_TessCoord.y + tese_in[2].normal * gl_TessCoord.z;
    vec2 interp_texCoord = tese_in[0].uv * gl_TessCoord.x + tese_in[1].uv * gl_TessCoord.y + tese_in[2].uv * gl_TessCoord.z;

    interp_pos += interp_normal * texture(displacementTexture, interp_texCoord).r * 10.0f;
    gl_Position = m * vec4(interp_pos, 1.0);
}
