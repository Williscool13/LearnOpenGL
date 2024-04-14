#version 330 core
layout (location = 0) out vec4 color;

in GS_OUT {
    vec3 wPos;
    vec3 wNormal;
    vec3 wTangent;
    vec3 vPos;
    vec3 vNormal;
    vec3 vTangent;
    vec2 uv;
    vec4 lightSpacePos;
    mat3 m_tbn;
    mat3 mv_tbn;
} fs_in;


uniform vec3 mainLightDirectionView;
uniform float specularExponent;

uniform sampler2DShadow shadowTexture;
uniform samplerCube shadowCubeTexture;
uniform sampler2D normalTexture;


uniform float shadowCubeFarPlane;
uniform vec3 pointLightPos;
uniform vec3 cameraPos;

float calculateShadow(vec4 lspos, float bias){
    vec3 projCoords = lspos.xyz / lspos.w;

    projCoords.z -= bias;
    // exceeds far plane
    if (projCoords.z > 1.0f)
        return 1.0f;

    return texture(shadowTexture, projCoords);
}

float calculateCubemapShadow(vec3 wPos){
    vec3 fragToLight = wPos - pointLightPos;
    float closestZ = texture(shadowCubeTexture, fragToLight).x;
    closestZ *= shadowCubeFarPlane;

    float currentZ = length(fragToLight);

    //float cubemapBias = max(0.005f * (1.0f - dot(fs_in.wNormal, -fragToLight)), 0.005f) 
        //* shadowCubeFarPlane;
    float cubemapBias = 0.0f;
    return currentZ - cubemapBias > closestZ ? 0.0f : 1.0f;
}

float attenuationFunction(float fDistance, float lightRadius){
    float dSquared = fDistance * fDistance;
    float rSquared = lightRadius * lightRadius;
    return 2 / (dSquared + rSquared + (fDistance * sqrt(dSquared + rSquared)));
}

float stylizedAttenationFunction(float fDistance){
    float epsilon = 1.0f;
    return 1.0f / (epsilon + fDistance);
}

vec3 normalMapRemap(vec3 normal){
    return normal * 2.0f - 1.0f;
}

void main()
{
    vec3 baseColor = vec3(0.6f, 0.3f, 0.3f);
    float distFromLight = length(fs_in.wPos - pointLightPos);
    float lightAttenuation = stylizedAttenationFunction(distFromLight);
    float lightIntensity = 15.0f;
    vec3 lightColor = vec3(1.0f) * lightAttenuation * lightIntensity;

    //  directional light - everything is in world space now.
    //vec3 viewDir = normalize(-fs_in.wPos);
    //vec3 lightDir = normalize(-mainLightDirectionView);
    vec3 viewDir = normalize(cameraPos - fs_in.wPos);
    vec3 lightDir = normalize(pointLightPos - fs_in.wPos);
    vec3 halfDir = normalize(lightDir + viewDir);

    vec3 viewReflect = reflect(viewDir, fs_in.wNormal);

    // Blinn-Phong
    //  ambient
    vec3 ambient = 0.15f * lightColor;

    //  diffuse
    float nDotL = max(dot(fs_in.wNormal, lightDir), 0.0f);
    vec3 diff = nDotL * lightColor;

    //  specular
    float nDotH = max(dot(fs_in.wNormal, halfDir), 0.0f);
    float specIntensity = pow(nDotH, specularExponent) * sign(nDotL);
    vec3 spec = specIntensity * lightColor;

    
    //float shadowBias = max(0.05f * (1.0f - dot(fs_in.wNormal, lightDir)), 0.005f);
    //float shadow = calculateShadow(fs_in.lightSpacePos, shadowBias);
    //vec3 light = (ambient + shadow * (diff + spec)) * baseColor;
    //color = vec4(light, 1.0f);

    float shadow2 = calculateCubemapShadow(fs_in.wPos);
    vec3 light2 = (ambient + shadow2 * (diff + spec)) * baseColor;
    color = vec4(light2, 1.0f);

    // normal viz
    //color = vec4(fs_in.vNormal * 0.5 + 0.5, 1.0f);

    //color = vec4(fs_in.vTangent * 0.5 + 0.5, 1.0f);
    //color = vec4(fs_in.vTangent, 1.0f);
    //color = vec4(cross(fs_in.vNormal, fs_in.vTangent) * 0.5 + 0.5, 1.0f);
    //vec3 testNormal = texture(normalTexture, fs_in.uv).rgb;
    //testNormal = normalMapRemap(testNormal);
    //testNormal = normalize(fs_in.mv_tbn * testNormal);
    //color = vec4(testNormal, 1.0f);

    
    //color = vec4(texture(normalTexture, fs_in.uv).rgb, 1.0f);
    //color = vec4(cross(fs_in.vNormal, fs_in.vTangent), 1.0f);
}