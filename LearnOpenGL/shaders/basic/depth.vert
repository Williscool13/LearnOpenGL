#version 330 core
layout (location = 0) in vec3 pos;

// VP is view projection of light, not camera
uniform mat4 vp;
uniform mat4 m;

void main()
{
    gl_Position = vp * m* vec4(pos, 1.0);
}  