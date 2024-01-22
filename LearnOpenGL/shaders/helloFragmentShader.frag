#version 330 core
in vec3 vColor;
out vec4 color;
uniform float time;

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

    vec3 albedo = vec3(r * vColor.r, g * vColor.g, b * vColor.b);

    vec3 correctedColor = gammaCorrect(albedo, 2.2f);

    color = vec4(correctedColor, 1.0f);
} 