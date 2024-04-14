#version 410 core
layout(vertices = 3) out;

in VS_OUT {
    vec3 pos;
    vec3 normal;
    vec3 tangent;
    vec2 uv;
} tesc_in[];

out TESC_OUT {
    vec3 pos;
    vec3 normal;        
    vec3 tangent;
    vec2 uv;
} tesc_out[];
uniform float tessellationLevel;

void main(void)
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tesc_out[gl_InvocationID].pos = tesc_in[gl_InvocationID].pos;
    tesc_out[gl_InvocationID].normal = tesc_in[gl_InvocationID].normal;
    tesc_out[gl_InvocationID].tangent = tesc_in[gl_InvocationID].tangent;
    tesc_out[gl_InvocationID].uv = tesc_in[gl_InvocationID].uv;
    
    if (gl_InvocationID == 0){
        gl_TessLevelOuter[0] = tessellationLevel;
        gl_TessLevelOuter[1] = tessellationLevel;
        gl_TessLevelOuter[2] = tessellationLevel;
    
        gl_TessLevelInner[0] = tessellationLevel;
    }
}