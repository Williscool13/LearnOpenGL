#version 330 core
uniform float time;

// material properties
uniform sampler2D renderTexture;

//in from vert
in vec3 viewPosition;
in vec3 viewNormal;
in vec2 uv;

// out from frag
out vec4 color;




void main()
{
    vec4 renderTex = texture(renderTexture, uv);
    color = vec4(renderTex.rgb + vec3(0.1,0.1,0.1), 1);
} 