#version 330 core

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexCoord;

in vec3 dirTexCoord;

uniform float specularExponent;
uniform vec3 mainLightDirectionView;
uniform samplerCube environmentTexture;

layout (location = 0) out vec4 color;

void main()
{	
	// blinn spec
    vec3 lightDir = normalize(mainLightDirectionView);
    vec3 viewDir = normalize(-vPos);
    vec3 halfDir = normalize(lightDir + viewDir);

	float nDotH = max(dot(vNormal, halfDir), 0.0f);
    float spec = pow(nDotH, specularExponent);

	vec3 finalSpecular = spec * vec3(1.0, 1.0, 1.0);
	color = texture(environmentTexture, dirTexCoord) + vec4(finalSpecular, 1.0);
	//color = vec4(1.0, 0.0, 0.0, 1.0);
}
