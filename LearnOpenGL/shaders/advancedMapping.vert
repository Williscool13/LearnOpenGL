layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 texCoord;

out VS_OUT {
    vec3 pos;
    vec3 normal;
    vec3 tangent;
    vec2 uv;


} vs_out;

void main()
{
    gl_Position = vec4(pos, 1.0);
    vs_out.pos = pos;
    vs_out.normal = normal;
    vs_out.tangent = tangent;
    vs_out.uv = texCoord;
}