#version 330 core
// time
//uniform float time;

// transform matrices
//uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;
//uniform mat4 mvp;

//uniform mat3 mvNormal;

// light properties
//uniform float ambientIlluminance;
//uniform vec3 mainLightDirection;
//float mainLightIlluminance = 1.0f;

// material properties
//uniform sampler2D fragTexture;
//uniform sampler2D specularTexture;
//uniform sampler2D ambientTexture;
//uniform sampler2D renderTextureTest;
//uniform vec4 Ka;
//uniform vec4 Kd;
//uniform vec4 Ks;
//uniform float specularExponent;


// in from vert
//in vec3 viewPosition;
//in vec3 viewNormal;
//in vec2 uv;
// out from frag
out vec4 color;


//vec3 gammaCorrect(vec3 color, float gamma)
//{
//	return pow(color, vec3(1.0f / gamma));
//}


void main()
{
    // wasteful but this isn't production code so who cares
    //vec3 viewLightDirection = (view * vec4(mainLightDirection, 0)).xyz;

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
    color = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    //gl_FragColor = color;
    // normals
    

} 