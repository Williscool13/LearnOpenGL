#version 330 core
//in vec3 vertexColor;
//in vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 mvp;

uniform mat3 mvNormal;

in vec3 viewPosition;
in vec3 viewNormal;

out vec4 color;

uniform float time;

uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float smoothness;
uniform float ambientIlluminance;

uniform vec3 mainLightDirection;


vec3 gammaCorrect(vec3 color, float gamma)
{
	return pow(color, vec3(1.0f / gamma));
}


void main()
{

    vec3 viewLightDirection = (view * vec4(mainLightDirection, 0)).xyz;

    
    vec3 diffuse = max(dot(normalize(viewNormal), normalize(viewLightDirection)), 0.0) * diffuseColor;

    vec3 reflectVec = normalize(reflect(-viewLightDirection, normalize(viewNormal)));
    vec3 viewDir = normalize(-viewPosition);
    float spec = pow(max(dot(reflectVec, viewDir), 0.0), smoothness);
    vec3 specular = specularColor * spec;

    vec3 ambient = ambientIlluminance * diffuseColor;



    float lerpVal = sin(time) * 0.5 + 0.5;
    //color = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), lerpVal);

    //color = vec4(normalize(mvNormal * vertexNormal), 1);

    // diffuse only
    //color = vec4(diffuse, 1);

    // specular only 
    //color = vec4(specular, 1);

    // ambient only
    //color = vec4(ambient, 1);

    // diffuse + specular + ambient
    float mainLightIlluminance = 1.0f;
    color = vec4((diffuse + specular) * mainLightIlluminance + ambient, 1);

} 