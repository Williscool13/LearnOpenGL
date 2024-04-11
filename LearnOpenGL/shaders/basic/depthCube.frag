#version 330 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float farPlane;

//layout (location = 0) out vec4 color;

void main()
{
    float lightDistance = length(FragPos.xyz - lightPos);
    lightDistance = lightDistance / farPlane;
    gl_FragDepth = lightDistance;

    //color = vec4(vec3(lightDistance), 1.0);
}  