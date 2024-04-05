#version 330 core


in vec3 directionaluv;

layout (location = 0) out vec4 color;

uniform samplerCube environmentTexture;

void main()
{
	color = texture(environmentTexture, directionaluv);
}
