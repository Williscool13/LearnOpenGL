#version 330 core
in vec3 vertexColor;
in vec2 TexCoord;

out vec4 color;


uniform float time;
uniform vec3 colorMod;

vec3 gammaCorrect(vec3 color, float gamma)
{
	return pow(color, vec3(1.0f / gamma));
}


void main()
{
    float r = sin(time) * 0.5 + 0.5;
    float g = cos(time) * 0.5 + 0.5;
    float b = sin(time) * cos(time) * 0.5 + 0.5;
    float col = (sin(time) / 1.5) + 0.5;
    //col = clamp(col, 0.0f, 1.0f);
    //vec3 albedo = vColor * col;

    vec3 albedo = vec3(r * vertexColor.r, g * vertexColor.g, b * vertexColor.b);

    vec3 correctedColor = gammaCorrect(albedo, 2.2f);

    //color = vec4(correctedColor, 1.0f);
    //color = texture(texture1, TexCoord) * vec4(correctedColor, 1.0);
    float lerpVal = sin(time) * 0.5 + 0.5;
    //color = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), lerpVal);

    //color = vec4(colorMod,1);
    color = vec4(vec3(1,1,1) * colorMod,1);
} 