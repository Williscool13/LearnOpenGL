#version 330 core

layout (location = 0) out vec4 color;

//in from vert
in vec3 vPos;
in vec3 vNormal;
in vec2 uv;
in vec4 lightSpacePos;


uniform vec3 mainLightDirectionView;
uniform float specularExponent;

uniform sampler2DShadow shadowTexture;

float calculateShadow(vec4 lspos, float bias){
    vec3 projCoords = lspos.xyz / lspos.w;

    projCoords.z -= bias;
    // exceeds far plane
    if (projCoords.z > 1.0f)
        return 1.0f;

    return texture(shadowTexture, projCoords);
}

void main()
{
    vec3 baseColor = vec3(0.6f, 0.3f, 0.3f);
    vec3 lightColor = vec3(1.0f);

    vec3 viewDir = normalize(-vPos);
    vec3 lightDir = normalize(-mainLightDirectionView);
    vec3 halfDir = normalize(lightDir + viewDir);

    vec3 viewReflect = reflect(viewDir, vNormal);

    // Blinn-Phong
    //  ambient
    vec3 ambient = 0.15f * lightColor;

    //  diffuse
    float nDotL = max(dot(vNormal, lightDir), 0.0f);
    vec3 diff = nDotL * lightColor;

    //  specular
    float nDotH = max(dot(vNormal, halfDir), 0.0f);
    float specIntensity = pow(nDotH, specularExponent) * sign(nDotL);
    vec3 spec = specIntensity * lightColor;

    
    float shadowBias = max(0.05f * (1.0f - dot(vNormal, lightDir)), 0.005f);
    float shadow = calculateShadow(lightSpacePos, shadowBias);
    vec3 light = (ambient + shadow * (diff + spec)) * baseColor;
    color = vec4(light, 1.0f);
    //color = vec4(baseColor * diff + spec, 1.0f);
    //color = vec4(1,0,0,0);
}