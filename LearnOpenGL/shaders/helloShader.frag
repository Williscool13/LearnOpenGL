#version 330 core
uniform float time;

// light properties
uniform float ambientIlluminance;
uniform vec3 mainLightDirectionView;
uniform float mainLightIlluminance = 1.0f;

// material properties
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D ambientTexture;
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float specularExponent;

//in from vert
in vec3 viewPosition;
in vec3 viewNormal;
in vec2 uv;

// out from frag
out vec4 color;

void main()
{
    vec3 diffuseColor = texture(diffuseTexture, uv).rgb;
    diffuseColor = vec3(diffuseColor.r * Kd.r, diffuseColor.g * Kd.g, diffuseColor.b * Kd.b);
    vec3 specularColor = texture(specularTexture, uv).rgb;
    specularColor = vec3(specularColor.r * Ks.r, specularColor.g * Ks.g, specularColor.b * Ks.b);
    vec3 ambientColor = texture(ambientTexture, uv).rgb;
    ambientColor = vec3(ambientColor.r * Ka.r, ambientColor.g * Ka.g, ambientColor.b * Ka.b);

    vec3 viewDir = normalize(-viewPosition);
    vec3 lightDir = normalize(-mainLightDirectionView);
    vec3 halfDir = normalize(lightDir + viewDir);

    vec3 viewReflect = reflect(viewDir, viewNormal);

    // Blinn-Phong
    float nDotL = max(dot(viewNormal, lightDir), 0.0f);
    float diff = nDotL;


    float spec = 0.0f;
    if (nDotL > 0.0f){
        float nDotH = max(dot(viewNormal, halfDir), 0.0f);
        spec = pow(nDotH, specularExponent);
    }
    



    vec3 ambient = ambientColor;
    vec3 diffuse = diff * diffuseColor;
    vec3 specular = spec * specularColor;

    color = vec4((ambient + diffuse + specular) * mainLightIlluminance, 1.0f);

    // Flat Color Debug
    //color = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    // Normal Color Debug
    //color = vec4(normalize(viewNormal), 1.0f);
} 