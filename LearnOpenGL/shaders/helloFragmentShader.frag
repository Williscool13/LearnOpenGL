#version 330 core
// transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 mvp;

uniform mat3 mvNormal;

// material properties
uniform float smoothness;
uniform float ambientIlluminance;

// time
uniform float time;

// light properties
uniform vec3 mainLightDirection;

// vertex attributes
in vec3 viewPosition;
in vec3 viewNormal;
in vec2 uv;

out vec4 color;

uniform sampler2D fragTexture;
uniform sampler2D specularTexture;

vec3 gammaCorrect(vec3 color, float gamma)
{
	return pow(color, vec3(1.0f / gamma));
}


void main()
{
    // wasteful but this isn't production code so who cares
    vec3 viewLightDirection = (view * vec4(mainLightDirection, 0)).xyz;

    vec4 diffTex = texture(fragTexture, uv);
    vec4 specTex = texture(specularTexture, uv);


    vec3 ambient = ambientIlluminance * diffTex.rgb;

    vec3 diffuse = max(dot(normalize(viewNormal), normalize(viewLightDirection)), 0.0) * diffTex.rgb;

    vec3 reflectVec = normalize(reflect(-viewLightDirection, normalize(viewNormal)));
    vec3 viewDir = normalize(-viewPosition);
    float specIntensity = pow(max(dot(reflectVec, viewDir), 0.0), smoothness);
    vec3 specular = specTex.rgb * specIntensity;




    // diffuse only
    //color = vec4(diffTex.rgb, 1);

    // specular only 
    //color = vec4(specular, 1);

    // ambient only
    //color = vec4(ambient, 1);

    // diffuse + specular + ambient
    float mainLightIlluminance = 1.0f;
    color = vec4((diffuse + specular) * mainLightIlluminance + ambient, 1);

    // normals
    //color = texture(fragTexture, uv);
    //color = vec4(normalize(viewNormal), 1);

} 