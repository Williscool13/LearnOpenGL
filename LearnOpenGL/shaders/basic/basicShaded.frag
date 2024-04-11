#version 330 core
layout (location = 0) out vec4 color;

in GS_OUT {
    vec3 wPos;
    vec3 wNormal;
    vec3 vPos;
    vec3 vNormal;
    vec2 uv;
    vec4 lightSpacePos;
} fs_in;


uniform vec3 mainLightDirectionView;
uniform float specularExponent;

uniform sampler2DShadow shadowTexture;

uniform samplerCube shadowCubeTexture;
uniform float shadowCubeFarPlane;
uniform vec3 shadowCubePos;

float calculateShadow(vec4 lspos, float bias){
    vec3 projCoords = lspos.xyz / lspos.w;

    projCoords.z -= bias;
    // exceeds far plane
    if (projCoords.z > 1.0f)
        return 1.0f;

    return texture(shadowTexture, projCoords);
}

float calculateCubemapShadow(vec3 wPos){
    vec3 fragToLight = wPos - shadowCubePos;
    float closestZ = texture(shadowCubeTexture, fragToLight).x;
    closestZ *= shadowCubeFarPlane;

    float currentZ = length(fragToLight);

    float cubemapBias = max(0.005f * (1.0f - dot(fs_in.wNormal, -fragToLight)), 0.005f) * shadowCubeFarPlane;
    return currentZ - cubemapBias > closestZ ? 0.0f : 1.0f;
}

void main()
{
    vec3 baseColor = vec3(0.6f, 0.3f, 0.3f);
    vec3 lightColor = vec3(1.0f);

    vec3 viewDir = normalize(-fs_in.vPos);
    vec3 lightDir = normalize(-mainLightDirectionView);
    vec3 halfDir = normalize(lightDir + viewDir);

    vec3 viewReflect = reflect(viewDir, fs_in.vNormal);

    // Blinn-Phong
    //  ambient
    vec3 ambient = 0.15f * lightColor;

    //  diffuse
    float nDotL = max(dot(fs_in.vNormal, lightDir), 0.0f);
    vec3 diff = nDotL * lightColor;

    //  specular
    float nDotH = max(dot(fs_in.vNormal, halfDir), 0.0f);
    float specIntensity = pow(nDotH, specularExponent) * sign(nDotL);
    vec3 spec = specIntensity * lightColor;

    
    float shadowBias = max(0.05f * (1.0f - dot(fs_in.wNormal, lightDir)), 0.005f);
    float shadow = calculateShadow(fs_in.lightSpacePos, shadowBias);
    vec3 light = (ambient + shadow * (diff + spec)) * baseColor;
    color = vec4(light, 1.0f);

    float shadow2 = calculateCubemapShadow(fs_in.wPos);
    vec3 light2 = (ambient + shadow2 * (diff + spec)) * baseColor;
    color = vec4(light2, 1.0f);

    
    float bothShads = 1 - (1 - shadow + 1 - shadow2);
    bothShads = clamp(bothShads, 0.0f, 1.0f);
    vec3 light3 = (ambient + bothShads * (diff + spec)) * baseColor;
    color = vec4(light3, 1.0f);
    //color = vec4(vec3(shadow2), 1.0f);

}