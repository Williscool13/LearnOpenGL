#version 330 core

layout (location = 0) out vec4 color;
in vec2 uv;
uniform sampler2D renderTexture;
uniform sampler2D shadowTexture;

void main()
{
    //color = texture(renderTexture, uv);
	
    float depthValue = texture(shadowTexture, uv).r;
    color = vec4(vec3(depthValue), 1.0);
}