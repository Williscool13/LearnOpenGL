#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// pass through triangle

in VS_OUT {
    vec3 wPos;
    vec3 wNormal;
    vec3 vPos;
    vec3 vNormal;
    vec2 uv;
    vec4 lightSpacePos;
} gs_in[];

out GS_OUT {
    vec3 wPos;
    vec3 wNormal;
    vec3 vPos;
    vec3 vNormal;
    vec2 uv;
    vec4 lightSpacePos;
} gs_out;

void main() {    
    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        gs_out.wPos = gs_in[i].wPos;
        gs_out.wNormal = gs_in[i].wNormal;
        gs_out.vPos = gs_in[i].vPos;
        gs_out.vNormal = gs_in[i].vNormal;
        gs_out.uv = gs_in[i].uv;
        gs_out.lightSpacePos = gs_in[i].lightSpacePos;
        EmitVertex();
    }

    EndPrimitive();
}  