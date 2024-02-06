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

uniform sampler2D renderTexture;

//in from vert
in vec3 viewPosition;
in vec3 viewNormal;
in vec2 uv;

// out from frag
out vec4 color;


//vec3 gammaCorrect(vec3 color, float gamma)
//{
//	return pow(color, vec3(1.0f / gamma));
//}


void main()
{
    // wasteful but this isn't production code so who cares

    //vec4 diffTex = texture(fragTexture, uv);
    //vec4 specTex = texture(specularTexture, uv);
    //vec4 ambTex = texture(ambientTexture, uv);

    //vec3 ambient = ambTex.rgb;

    //vec3 diffColor = mix(diffTex.rgb, Kd.rgb, Kd.a);
    //float diffNDotL = max(dot(normalize(viewNormal), normalize(viewLightDirection)), 0.0);
    //vec3 diffuse = diffColor * diffNDotL;
    
    //vec3 specColor = mix(specTex.rgb, Ks.rgb, Ks.a);
    //vec3 reflectVec = normalize(reflect(-viewLightDirection, normalize(viewNormal)));
    //vec3 viewDir = normalize(-viewPosition);
    //float specIntensity = pow(max(dot(reflectVec, viewDir), 0.0), specularExponent);
    //vec3 specular = specColor * specIntensity;


    // diffuse only
	//color = vec4(diffuse , 1);

    // specular only 
    //color = vec4(specular, 1);

    // ambient only
    //color = vec4(ambient, 1);

    // diffuse + specular + ambient
    //color = vec4((diffuse + specular) * mainLightIlluminance + ambient * mainLightIlluminance, 1);
    //color = vec4(uv3 + normal2, 1);
    //gl_FragColor = color;
    // normals


    // Blinn-Phong
    //vec3 surfaceColor = vec3(1.0f, 0.5f, 0.2f);
    //vec3 diffuseColor = texture(diffuseTexture, uv).rgb;
    //diffuseColor = vec3(diffuseColor.r * Kd.r, diffuseColor.g * Kd.g, diffuseColor.b * Kd.b);
    //vec3 specularColor = texture(specularTexture, uv).rgb;
    //specularColor = vec3(specularColor.r * Ks.r, specularColor.g * Ks.g, specularColor.b * Ks.b);
    //vec3 ambientColor = texture(ambientTexture, uv).rgb;
    //ambientColor = vec3(ambientColor.r * Ka.r, ambientColor.g * Ka.g, ambientColor.b * Ka.b);

    //vec3 viewDir = normalize(-viewPosition);
    //vec3 lightDir = normalize(mainLightDirectionView);
    //vec3 halfDir = normalize(lightDir + viewDir);

    //float nDotL = max(dot(viewNormal, lightDir), 0.0f);
    //float diff = nDotL;

    //float nDotH = max(dot(viewNormal, halfDir), 0.0f);
    //float spec = pow(nDotH, specularExponent);


    //vec3 ambient = ambientColor;
    //vec3 diffuse = diff * diffuseColor;
    //vec3 specular = spec * specularColor;


    //color = vec4((ambient + diffuse + specular) * mainLightIlluminance, 1.0f);



    //color = vec4(1,1,1,1);
    vec4 renderTex = texture(renderTexture, uv);
    color = vec4(renderTex.rgb + vec3(0.1,0.1,0.1), 1);


    // Flat Color Debug
    //color = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    // Normal Color Debug
    //color = vec4(normalize(viewNormal), 1.0f);


    

} 