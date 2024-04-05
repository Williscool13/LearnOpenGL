#version 330 core
layout (location = 0) in vec3 pos; // position is in clip space, dont need to be transformed

uniform mat4 i_vp;

out vec3 directionaluv;


void main()
{
    vec4 _pos = vec4(pos, 1.0);
    gl_Position = _pos;
    directionaluv = normalize((i_vp * _pos).xyz); // get the camera space direction of the camera frustum bounds
}